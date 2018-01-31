#include <QString>
#include <QHostAddress>
#include <QByteArray>
#include <QDataStream>
#include <QDate>
#include <QMutexLocker>

#include "vkkiller_server.h"
#include "vkkiller_client.h"
#include "vkkiller_request_reply.h"


VkKillerServer::VkKillerServer(QObject* parent):
    QTcpServer          (parent),
    m_openTopicsAmount  (0),
    loggingEnabled      (false)
{}


VkKillerServer::~VkKillerServer() {
    stop();
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

void VkKillerServer::stop() noexcept {
    for (auto& client: m_clients)
        client.second->close();
    m_clients.clear();

    close();
}


bool VkKillerServer::isWorking() const noexcept {
    return isListening();
}


void VkKillerServer::incomingConnection(qintptr socketDescriptor) {
    QMutexLocker locker(&m_globalSynchMutex);
    m_clients[socketDescriptor] = std::make_unique<VkKillerClient>(socketDescriptor);
    VkKillerClient* client = m_clients[socketDescriptor].get();

    connect(client, SIGNAL(disconnected()), this, SLOT(disconnectClient()));
    connect(client, SIGNAL(readyRead()),    this, SLOT(processClientRequest()));

    if (loggingEnabled)
        client->m_logs << "### Client has connected ###";
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

        blockSize = 0;

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
            if (loggingEnabled) {
                client->m_logs << "\nRequest: GET_TOPIC_HISTORY"
                << "\nParams: topicNum =" << QString::number(topicNum);
            }

            client->m_selectedTopicNum  = topicNum;
            client->m_lastReadMsgNum    = m_topics[topicNum].size() - 1;

            QString history = m_topics[topicNum].getPackedHistory();
            replyToClient(client, Reply_type::OK, history);
        } // GET_TOPIC_HISTORY
        else if (request == Request_type::GET_LAST_MESSAGES_FROM_TOPIC) {
            if (loggingEnabled) {
                client->m_logs << "\nRequest: GET_LAST_MESSAGES_FROM_TOPIC"
                << "\nParams: topicNum =" << QString::number(topicNum);
            }

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
            if (loggingEnabled) {
                client->m_logs << "\nRequest: GET_TOPICS_LIST";
            }

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

            if (loggingEnabled) {
                client->m_logs << "\nRequest: TEXT_MESSAGE"
                << "\nParams: message =" << message
                << ", topicNum =" << QString::number(topicNum);
            }

            if (message.length() > MAX_MESSAGE_LENGTH ||
               !VkKillerTopic::isValidMessage(message))
            {
                replyToClient(client, Reply_type::WRONG_MESSAGE);
                continue;
            }

            QTime time = QTime::currentTime();
            QDate date = QDate::currentDate();

            if (client->m_lastMessageTime.secsTo(time) < MESSAGING_COOLDOWN) {
                replyToClient(client, Reply_type::TOO_FAST_MESSAGING);
                continue;
            }
            client->m_lastMessageTime = time;

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

            if (loggingEnabled) {
                client->m_logs << "\nRequest: CREATE_TOPIC"
                << "\nParams: topicName =" << topicName
                << ", message =" << message;
            }

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

            if (loggingEnabled) {
                client->m_logs << "\nRequest: SET_NAME"
                << "\nParams: name =" << name;
            }

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

    if (loggingEnabled) {
        if (reply_type == Reply_type::OK && !msg.length())
            client->m_logs << "\nReply: OK";
        else if (reply_type == Reply_type::OK && msg.length())
            client->m_logs << "\nReply:" << msg;
        else
            client->m_logs << "Request error:" << QString::number(reply_type);
    }
}


void VkKillerServer::disconnectClient() {
    VkKillerClient* client = static_cast<VkKillerClient*>(sender());

    if (loggingEnabled)
        client->m_logs << "\n### Client has disconnected ###";

    client->close();
    m_clients.erase(client->id());
}
