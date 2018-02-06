#include <QString>
#include <QHostAddress>
#include <QByteArray>
#include <QDataStream>
#include <QDate>
#include <QMutexLocker>

#include "vkkiller_server.h"
#include "vkkiller_client.h"
#include "vkkiller_request_reply.h"

#include <iostream>


VkKillerServer::VkKillerServer(QObject* parent):
    QTcpServer          (parent),
    m_openTopicsAmount  (0),
    m_loggingEnabled    (false)
{}


VkKillerServer::~VkKillerServer() {
    stop();
}


bool VkKillerServer::start(const QHostAddress& address, quint16 port, QString* errMsg) noexcept {
    if (!listen(address, port)) {
        if (errMsg != nullptr)
            *errMsg = errorString();
        close();
        return false;
    }

    return true;
}


void VkKillerServer::stop() noexcept {
    for (auto& client: m_clients) {
        client->setSocketDescriptor(-1);
        emit clientDisconnected(client);
    }
    
    m_clients.clear();
    close();
}


void VkKillerServer::enableLogging(bool flag) noexcept {
    m_loggingEnabled = flag;

    if (!m_loggingEnabled) {
        for (auto& client: m_clients) {
            client->m_loggingEnabled = false;
            client->m_logs.clear();
        }
    }
    else {
        for (auto& client: m_clients)
            client->m_loggingEnabled = true;
    }
}


void VkKillerServer::enableLoggingFor(const VkKillerClient* client, bool flag) noexcept {
    auto it = m_clients.find(client->id());
    if (it == m_clients.end()) return;
    it.value()->m_loggingEnabled = flag;
}


void VkKillerServer::incomingConnection(qintptr socketDescriptor) {
    m_clients[socketDescriptor] = new VkKillerClient(socketDescriptor, this);
    VkKillerClient* client = m_clients[socketDescriptor];

    connect(client, SIGNAL(disconnected()), this, SLOT(disconnectClient    ()));
    connect(client, SIGNAL(readyRead   ()), this, SLOT(processClientRequest()));

    emit clientConnected(client);

    if (m_loggingEnabled)
        client->addEntryToLogs("Client has connected");
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

        if (request == Request_type::GET_TOPIC_HISTORY) {
            quint16 topicNum;
            in >> topicNum;

            if (client->m_loggingEnabled) {
                QString entry = "Request: GET_TOPIC_HISTORY"
                        "\nParams: topicNum = "
                        + QString::number(topicNum);
                client->addEntryToLogs(entry);
            }

            if (topicNum >= m_topics.size() || m_topics[topicNum].closed()) {
                replyToClient(client, Reply_type::UNKNOWN_TOPIC);
                continue;
            }

            client->m_selectedTopicNum  = topicNum;
            client->m_lastReadMsgNum    = m_topics[topicNum].size() - 1;

            QString history = m_topics[topicNum].getPackedHistory();
            replyToClient(client, Reply_type::OK, history);
        } // GET_TOPIC_HISTORY
        else if (request == Request_type::GET_LAST_MESSAGES_FROM_TOPIC) {
            quint16 topicNum;
            in >> topicNum;

            if (client->m_loggingEnabled) {
                QString entry = "Request: GET_LAST_MESSAGES_FROM_TOPIC"
                                "\nParams: topicNum = "
                              + QString::number(topicNum);
                client->addEntryToLogs(entry);
            }

            if (topicNum >= m_topics.size() || m_topics[topicNum].closed()) {
                replyToClient(client, Reply_type::UNKNOWN_TOPIC);
                continue;
            }

            QString history;
            client->m_lastReadMsgNum = m_topics[topicNum].size() - 1;

            if (client->m_selectedTopicNum != topicNum) {
                client->m_selectedTopicNum  = topicNum;
                history = m_topics[topicNum].getPackedHistory();
            }
            else {
                size_t msgNum = client->m_lastReadMsgNum;
                history = m_topics[topicNum].getPackedHistory(msgNum);
            }
            replyToClient(client, Reply_type::OK, history);
        } // GET_LAST_MESSAGES_FROM_TOPIC
        else if (request == Request_type::GET_TOPICS_LIST) {
            if (client->m_loggingEnabled)
                client->addEntryToLogs("Request: GET_TOPICS_LIST");

            QString outstr = "";
            size_t  last = m_topics.size() - 1;

            for (size_t i = 0; i < last; ++i) {
                if (m_topics[i].closed()) continue;

                outstr += QString::number(i)                   + SEPARATING_CH
                       + m_topics[i].name()                    + SEPARATING_CH
                       + QString::number(m_topics[i].rating()) + SEPARATING_CH;
            }

            if (!m_topics[last].closed())
                outstr += QString::number(last) + SEPARATING_CH
                       + m_topics[last].name()  + SEPARATING_CH
                       + QString::number(m_topics[last].rating());

            replyToClient(client, Reply_type::OK, outstr);
        } // GET_TOPICS_LIST
        else if (request == Request_type::TEXT_MESSAGE) {
            quint16 topicNum;
            QString message;
            in >> topicNum >> message;

            QTime time = QTime::currentTime();
            QDate date = QDate::currentDate();

            if (client->m_loggingEnabled) {
                QString entry = "Request: TEXT_MESSAGE"
                                "\nParams: message = "
                                + message
                                + ",  topicNum = "
                                + QString::number(topicNum);
                client->addEntryToLogs(entry, time, date);
            }

            if (topicNum >= m_topics.size() || m_topics[topicNum].closed()) {
                replyToClient(client, Reply_type::UNKNOWN_TOPIC);
                continue;
            }

            if (!VkKillerTopic::isValidMessage(message)) {
                replyToClient(client, Reply_type::WRONG_MESSAGE);
                continue;
            }

            if (!client->m_lastMessageTime.isNull())
                if (client->m_lastMessageTime.secsTo(time) < MESSAGING_COOLDOWN) {
                    replyToClient(client, Reply_type::TOO_FAST_MESSAGING);
                    continue;
                }

            client->m_lastMessageTime = time;

            m_topics[topicNum].addMessage(client->name(), client->id(), time, date, message);
        } // TEXT_MESSAGE
        else if (request == Request_type::CREATE_TOPIC) {
            QString topicName, message;
            in >> topicName >> message;

            QTime time = QTime::currentTime();
            QDate date = QDate::currentDate();

            if (client->m_loggingEnabled) {
                QString entry = "Request: CREATE_TOPIC"
                                "\nParams: topicName = "
                                + topicName
                                + ", message = "
                                + message;
                client->addEntryToLogs(entry, time, date);
            }

            if (m_openTopicsAmount >= MAX_TOPICS_AMOUNT) {
                replyToClient(client, Reply_type::FAILED_TOPIC_CREATE);
                continue;
            }

            if (!VkKillerTopic::isValidTopicName(topicName)) {
                replyToClient(client, Reply_type::WRONG_TOPIC_NAME);
                continue;
            }

            if (!VkKillerTopic::isValidMessage(message)) {
                replyToClient(client, Reply_type::WRONG_MESSAGE);
                continue;
            } 

            client->m_lastMessageTime = time;

            QMutexLocker locker(&m_openTopicMutex);
            for (size_t i = 0; i < m_topics.size(); ++i)
                if (m_topics[i].closed()) {
                    if (!m_topics[i].open(topicName))
                        continue;

                    m_topics[i].addMessage(client->name(), client->id(), time, date, message);
                    m_openTopicsAmount++;
                    break;
                }
        } // CREATE_TOPIC
        else if (request == Request_type::SET_NAME) {
            QString name;
            in >> name;

            if (client->m_loggingEnabled) {
                QString entry = "Request: SET_NAME"
                                "\nParams: name = "
                                + name;
                client->addEntryToLogs(entry);
            }

            if (!VkKillerClient::isValidName(name)) {
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

    if (client->m_loggingEnabled) {
        QString entry;

        if (reply_type == Reply_type::OK && !msg.length())
            entry = "Reply: OK";
        else if (reply_type == Reply_type::OK && msg.length())
            entry = "Reply: " + msg;
        else
            entry = "Request error: " + QString::number(reply_type);

        client->addEntryToLogs(entry);
    }
}


void VkKillerServer::disconnectClient() {
    VkKillerClient* client = static_cast<VkKillerClient*>(sender());

    emit clientDisconnected(client);

    if (client->m_loggingEnabled)
        client->addEntryToLogs("Client has disconnected");

    client->close();
    m_clients.remove(client->id());
}