#include <QString>
#include <QHostAddress>
#include <QByteArray>
#include <QDataStream>
#include <QTime>
#include <QDate>
#include <QMutexLocker>

#include "vkkiller_server.h"
#include "vkkiller_client.h"
#include "vkkiller_request_reply.h"


VkKillerServer::VkKillerServer(QObject* parent):
    QTcpServer          (parent),
    m_openTopicsAmount  (0)
{}


VkKillerServer::~VkKillerServer() {
    for (auto& client: m_clients)
        client.second->close();
    m_clients.clear();

    close();
}


bool VkKillerServer::start(const QHostAddress& address, quint16 port, QString* errMsg) {
    if (!listen(address, port)) {
        if (errMsg != nullptr)
            *errMsg = errorString();
        close();
        return false;
    }

    return true;
}

void VkKillerServer::incomingConnection(qintptr socketDescriptor) {
    QMutexLocker locker(&m_globalSynchMutex);
    m_clients[socketDescriptor] = std::make_unique<VkKillerClient>(socketDescriptor);
    const VkKillerClient* client = m_clients[socketDescriptor].get();

    connect(client, SIGNAL(disconnected()), this, SLOT(disconnectClient()));
    connect(client, SIGNAL(readyRead()),    this, SLOT(processClientRequest()));

    qDebug() << "\nNew connection from" << client->peerAddress().toString();
}


void VkKillerServer::processClientRequest() {
    VkKillerClient* client = static_cast<VkKillerClient*>(sender());
    QDataStream     in(client);
    quint16         blockSize = 0;

    in.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    while (true) {
        if (!blockSize) {
            if (client->bytesAvailable() < sizeof(quint16))
                break;
            in >> blockSize;
        }

        if (client->bytesAvailable() < blockSize)
            break;

        quint8 request;
        in >> request;

        quint16 topicNum = 0;
        if (request == Request_type::GET_TOPIC_HISTORY              ||
            request == Request_type::GET_LAST_MESSAGES_FROM_TOPIC   ||
            request == Request_type::TEXT_MESSAGE)
        {
            in >> topicNum;

            if (topicNum >= m_topics.size() || m_topics[topicNum].closed()) {
                replyToClient(client, Reply_type::UNKNOWN_TOPIC);
                continue;
            }
        }

        if (request == Request_type::GET_TOPIC_HISTORY) {
            qDebug() << "\nGET_TOPIC_HISTORY request from" << client->peerAddress().toString()
                     << "\nParams: topicNum =" << topicNum;

            client->m_selectedTopicNum  = topicNum;
            client->m_lastReadMsgNum    = m_topics[topicNum].size() - 1;

            QString history = m_topics[topicNum].getPackedHistory();
            replyToClient(client, Reply_type::OK, history);
        } // GET_TOPIC_HISTORY
        else if (request == Request_type::GET_LAST_MESSAGES_FROM_TOPIC) {
            qDebug() << "\nGET_LAST_MESSAGES_FROM_TOPIC request from" << client->peerAddress().toString()
                     << "\nParams: topicNum =" << topicNum;

            QString history;

            if (client->m_selectedTopicNum != topicNum) {
                client->m_selectedTopicNum  = topicNum;
                client->m_lastReadMsgNum    = m_topics[topicNum].size() - 1;
                history = m_topics[topicNum].getPackedHistory();
            }
            else {
                size_t msgNum = client->m_lastReadMsgNum;
                history = m_topics[topicNum].getPackedHistory(msgNum);
            }
            replyToClient(client, Reply_type::OK, history);
        } // GET_LAST_MESSAGES_FROM_TOPIC
        else if (request == Request_type::GET_TOPICS_LIST) {
            qDebug() << "\nGET_TOPICS_LIST request from" << client->peerAddress().toString();

            QChar   SEPARATING_CH = '\1';
            QString outstr = "";
            size_t  last = m_topics.size() - 1;

            for (size_t i = 0; i < last; ++i) {
                if (m_topics[i].closed()) continue;

                outstr += QString::number(i)                   + SEPARATING_CH
                       + m_topics[i].name()                    + SEPARATING_CH
                       + QString::number(m_topics[i].rating()) + SEPARATING_CH;
            }

            outstr += QString::number(last) + SEPARATING_CH
                   + m_topics[last].name()  + SEPARATING_CH
                   + QString::number(m_topics[last].rating());

            replyToClient(client, Reply_type::OK, outstr);
        } // GET_TOPICS_LIST
        else if (request == Request_type::TEXT_MESSAGE) {
            QString message;
            in >> message;

            qDebug() << "\nTEXT_MESSAGE request from" << client->peerAddress().toString()
                     << "\nParams: message =" << message << ", topicNum =" << topicNum;

            if (message.length() > MAX_MESSAGE_LENGTH ||
               !VkKillerTopic::isValidMessage(message))
            {
                replyToClient(client, Reply_type::WRONG_MESSAGE);
                continue;
            }

            QTime time = QTime::currentTime();
            QDate date = QDate::currentDate();

            m_topics[topicNum].addMessage(client->name(), client->id(), time, date, message);
            replyToClient(client, Reply_type::OK);
        } // TEXT_MESSAGE
        else if (request == Request_type::CREATE_TOPIC) {
            if (m_openTopicsAmount >= MAX_TOPICS_AMOUNT) {
                replyToClient(client, Reply_type::FAILED_TOPIC_CREATE);
                continue;
            }

            QString topicName, message;
            in >> topicName >> message;

            if (topicName.length() > MAX_TOPIC_NAME_LENGTH) {
                replyToClient(client, Reply_type::WRONG_TOPIC_NAME);
                continue;
            }

            if (message.length() > MAX_MESSAGE_LENGTH ||
               !VkKillerTopic::isValidMessage(message))
            {
                replyToClient(client, Reply_type::WRONG_MESSAGE);
                continue;
            }

            qDebug() << "\nCREATE_TOPIC request from" << client->peerAddress().toString()
                     << "\nParams: topicName =" << topicName << ", message =" << message;

            QTime time = QTime::currentTime();
            QDate date = QDate::currentDate();

            QMutexLocker locker(&m_openTopicMutex);
            for (size_t i = 0; i < m_topics.size(); ++i)
                if (m_topics[i].closed()) {
                    if (!m_topics[i].open(topicName))
                        continue;

                    m_topics[i].addMessage(client->name(), client->id(), time, date, message);
                    m_openTopicsAmount++;
                    break;
                }

            replyToClient(client, Reply_type::OK);
        } // CREATE_TOPIC
        else if (request == Request_type::SET_NAME) {
            QString name;
            in >> name;

            qDebug() << "\nSET_NAME request from" << client->peerAddress().toString()
                     << "\nParams: name =" << name;

            if (name.length() > MAX_CLIENT_NAME_LENGTH ||
               !VkKillerClient::isValidName(name))
            {
                replyToClient(client, Reply_type::WRONG_NAME);
                continue;
            }

            client->m_name = std::move(name);
        } // SET_NAME
        else replyToClient(client, Reply_type::UNKNOWN_REQUEST);
    }
}


inline void VkKillerServer::replyToClient(VkKillerClient* client, quint8 reply_type, const QString& msg) noexcept {
    QByteArray  buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    out << quint16(0) << reply_type << msg;
    out.device()->seek(0);
    out << quint16(buffer.size() - sizeof(quint16));
    client->write(buffer);

    QString address = client->peerAddress().toString();
    if (reply_type == Reply_type::OK) {
        if (!msg.length())
            qDebug() << "\nReply to" << address << ": OK";
        else
            qDebug() << "\nReply to" << address << ":" << msg;
    }
    else
        qDebug() << "\nRequest error from" << address << ":" << reply_type;
}


void VkKillerServer::disconnectClient() {
    VkKillerClient* client = static_cast<VkKillerClient*>(sender());
    qDebug() << "\nClient" << client->peerAddress().toString() << "has disconnected";
    client->close();
    m_clients.erase(client->id());
}