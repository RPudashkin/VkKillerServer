#include <QStringBuilder>
#include <QHostAddress>
#include <QDataStream>
#include <QThread>
#include <QMutexLocker>

#include "vkkiller_server.h"
#include "vkkiller_client.h"
#include "vkkiller_request_reply.h"


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

    if (m_loggingEnabled)
        client->addEntryToLogs("Client has connected");

    emit clientConnected(client);

    QThread* thr = new QThread(this);
    connect(thr, &QThread::started, this, [this, client, thr]() {
        connect(client, SIGNAL(disconnected()), this, SLOT(disconnectClient    ()));
        connect(client, SIGNAL(disconnected()), thr,  SLOT(quit                ()));
        connect(client, SIGNAL(readyRead   ()), this, SLOT(processClientRequest()));

        replyToClient(client, Reply_type::CONNECTED);
    });

    thr->start();
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
                        % QString::number(topicNum);
                client->addEntryToLogs(entry);
            }

            m_topics[client->m_selectedTopicNum].delReader(client);
            bool clientHasAdded = m_topics[topicNum].addReader(client);

            if (topicNum >= m_topics.size() || !clientHasAdded) {
                replyToClient(client, Reply_type::UNKNOWN_TOPIC);
                break;
            }

            client->m_selectedTopicNum  = topicNum;
            client->m_lastReadMsgNum    = m_topics[topicNum].size() - 1;

            QString history = m_topics[topicNum].getPackedHistory();
            replyToClient(client, Reply_type::TOPIC_HISTORY, history);
        } // GET_TOPIC_HISTORY
        else if (request == Request_type::GET_LAST_MESSAGES_FROM_TOPIC) {
            quint16 topicNum;
            in >> topicNum;

            if (client->m_loggingEnabled) {
                QString entry = "Request: GET_LAST_MESSAGES_FROM_TOPIC"
                                "\nParams: topicNum = "
                              % QString::number(topicNum);
                client->addEntryToLogs(entry);
            }

            if (topicNum >= m_topics.size() || m_topics[topicNum].closed()) {
                replyToClient(client, Reply_type::UNKNOWN_TOPIC);
                break;
            }

            if (client->m_lastReadMsgNum == m_topics[topicNum].size() - 1) {
                replyToClient(client, Reply_type::LAST_MESSAGES);
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
            replyToClient(client, Reply_type::LAST_MESSAGES, history);
        } // GET_LAST_MESSAGES_FROM_TOPIC
        else if (request == Request_type::GET_TOPICS_LIST) {
            if (client->m_loggingEnabled)
                client->addEntryToLogs("Request: GET_TOPICS_LIST");

            replyToClient(client, Reply_type::TOPICS_LIST, getPackedTopicsList());
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
                                % message
                                % ",  topicNum = "
                                % QString::number(topicNum);
                client->addEntryToLogs(entry, time, date);
            }

            if (topicNum >= m_topics.size() || m_topics[topicNum].closed()) {
                replyToClient(client, Reply_type::UNKNOWN_TOPIC);
                break;
            }

            if (!VkKillerTopic::isValidMessage(message)) {
                replyToClient(client, Reply_type::WRONG_MESSAGE);
                break;
            }

            if (!client->m_lastMessageTime.isNull())
                if (client->m_lastMessageTime.secsTo(time) < MESSAGING_COOLDOWN) {
                    replyToClient(client, Reply_type::TOO_FAST_MESSAGING);
                    break;
                }

            m_topics[topicNum].addMessage(client->name(), client->id(), time, date, std::move(message));
            client->m_lastMessageTime = time;
            client->m_lastReadMsgNum  = m_topics[topicNum].size() - 1;

            auto topicReaders = m_topics[topicNum].getReaders();
            for (auto& reader: topicReaders) {
                QString history = m_topics[topicNum].getPackedHistory(client->m_lastReadMsgNum);
                replyToClient(reader, Reply_type::LAST_MESSAGES, history);
            }
        } // TEXT_MESSAGE
        else if (request == Request_type::CREATE_TOPIC) {
            QString topicName, message;
            in >> topicName >> message;

            QTime time = QTime::currentTime();
            QDate date = QDate::currentDate();

            if (client->m_loggingEnabled) {
                QString entry = "Request: CREATE_TOPIC"
                                "\nParams: topicName = "
                                % topicName
                                % ",  message = "
                                % message;
                client->addEntryToLogs(entry, time, date);
            }

            if (!client->m_lastTopicCreatingTime.isNull())
                if (client->m_lastTopicCreatingTime.secsTo(time) < TOPIC_CREATING_COOLDOWN) {
                    replyToClient(client, Reply_type::TOO_FAST_TOPIC_CREATING);
                    break;
                }

            if (m_openTopicsAmount >= MAX_TOPICS_AMOUNT) {
                replyToClient(client, Reply_type::FAILED_TOPIC_CREATE);
                break;
            }

            if (!VkKillerTopic::isValidTopicName(topicName)) {
                replyToClient(client, Reply_type::WRONG_TOPIC_NAME);
                break;
            }

            if (!VkKillerTopic::isValidMessage(message)) {
                replyToClient(client, Reply_type::WRONG_MESSAGE);
                break;
            } 

            client->m_lastTopicCreatingTime = time;

            QMutexLocker locker(&m_openTopicMutex);
            for (size_t i = 0; i < m_topics.size(); ++i)
                if (m_topics[i].closed()) {
                    if (!m_topics[i].open(topicName))
                        continue;

                    m_topics[i].addMessage(client->name(), client->id(), time, date, std::move(message));
                    m_openTopicsAmount++;
                    break;
                }

            QString packedTopicList = getPackedTopicsList();
            for (auto& client: m_clients)
                replyToClient(client, Reply_type::TOPICS_LIST, packedTopicList);
        } // CREATE_TOPIC
        else if (request == Request_type::SET_NAME) {
            QString name;
            in >> name;

            if (client->m_loggingEnabled) {
                QString entry = "Request: SET_NAME"
                                "\nParams: name = "
                                % name;
                client->addEntryToLogs(entry);
            }

            if (!VkKillerClient::isValidName(name)) {
                replyToClient(client, Reply_type::WRONG_NAME);
                break;
            }

            client->m_name = std::move(name);
        } // SET_NAME
        else replyToClient(client, Reply_type::UNKNOWN_REQUEST);
    }
}


QString VkKillerServer::getPackedTopicsList() const noexcept {
    QString packedTopicsList = "";
    size_t  last = m_topics.size() - 1;

    for (size_t i = 0; i < last; ++i) {
        if (m_topics[i].closed()) continue;

        packedTopicsList = packedTopicsList
               % QString::number(i)                    % SEPARATING_CH
               % m_topics[i].name()                    % SEPARATING_CH
               % QString::number(m_topics[i].rating()) % SEPARATING_CH;
    }

    if (!m_topics[last].closed())
        packedTopicsList = packedTopicsList
               % QString::number(last)  % SEPARATING_CH
               % m_topics[last].name()  % SEPARATING_CH
               % QString::number(m_topics[last].rating());

    return packedTopicsList;
}


inline void VkKillerServer::replyToClient(VkKillerClient* client, quint8 reply_type, const QString& msg) noexcept {
    QByteArray  buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    out << quint16(0) << reply_type;
    if (!msg.isEmpty())
        out << msg;

    out.device()->seek(0);
    out << quint16(buffer.size() - sizeof(quint16));
    client->write(buffer);

    if (client->m_loggingEnabled) {
        QString entry;

        if (reply_type == Reply_type::CONNECTED      ||
            reply_type == Reply_type::TOPICS_LIST    ||
            reply_type == Reply_type::TOPIC_HISTORY  ||
            reply_type == Reply_type::LAST_MESSAGES)
        {
            if (!msg.isEmpty())
                entry = "Reply: " % msg;
            else return;
        }
        else
            entry = "Request error: " % QString::number(reply_type);

        client->addEntryToLogs(entry);
    }
}


void VkKillerServer::disconnectClient() {
    VkKillerClient* client = static_cast<VkKillerClient*>(sender());

    emit clientDisconnected(client);

    if (client->m_loggingEnabled)
        client->addEntryToLogs("Client has disconnected");

    m_topics[client->m_selectedTopicNum].delReader(client);
    client->close();

    QMutexLocker locker(&m_deleteClientMutex);
    m_clients.remove(client->id());
}