#include <QByteArray>
#include <QDataStream>
#include <QTextCodec>
#include <QTime>
#include <QDate>

#include "vkkiller_server.h"
#include "vkkiller_client.h"
#include "vkkiller_topic.h"
#include "vkkiller_request_reply.h"


VkKillerServer::VkKillerServer(QObject* parent):
    QTcpServer(parent)
{}


VkKillerServer::~VkKillerServer() {
    for (auto& client: m_clients)
        client.second->close();

    m_clients.clear();
    m_topics.clear();

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
    std::lock_guard<std::mutex> locker(m_globalSynchMutex);
    m_clients[socketDescriptor] = std::make_unique<VkKillerClient>(socketDescriptor);

    connect(m_clients[socketDescriptor].get(), SIGNAL(&VkKillerServer::disconnected()),
            this, SLOT(disconnectClient()));

    connect(m_clients[socketDescriptor].get(), SIGNAL(&VkKillerServer::readyRead()),
            this, SLOT(processClientRequest()));
}


void VkKillerServer::processClientRequest() {
    VkKillerClient* client = static_cast<VkKillerClient*>(sender());
    QDataStream 	in(client);
    quint16			blockSize = 0;

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

        if (request == Request_type::SET_NAME) {
            QString name;
            in >> name;

            if (name.length() > VkKillerClient::MAX_NAME_LENGTH) {
                replyToClient(client, Reply_type::WRONG_NAME);
                return;
            }

            client->m_name = std::move(name);
        }
        else if (request == Request_type::GET_TOPICS_LIST) {

        }
        else if (request == Request_type::GET_TOPIC_RATING) {
            quint16 topicNum;
            in >> topicNum;

            if (topicNum < 0 || topicNum >= m_topics.size()) {
                replyToClient(client, Reply_type::UNKNOWN_TOPIC);
                return;
            }
            // send rating
        }
        else if (request == Request_type::CREATE_TOPIC) {
            if (m_topics.size() >= MAX_TOPICS_AMOUNT) {
                replyToClient(client, Reply_type::FAILED_TOPIC_CREATE);
                return;
            }

            QString topicName;
            in >> topicName;

            if (topicName.length() > VkKillerTopic::MAX_NAME_LENGTH) {
                replyToClient(client, Reply_type::WRONG_TOPIC_NAME);
                return;
            }

            std::lock_guard<std::mutex> locker(m_topicCreatingMutex);
            m_topics.push_back(std::make_unique<VkKillerTopic>(topicName));
        }
        else if (request == Request_type::TEXT_MESSAGE) {
            quint16 topicNum;
            in >> topicNum;

            if (topicNum < 0 || topicNum >= m_topics.size()) {
                replyToClient(client, Reply_type::UNKNOWN_TOPIC);
                return;
            }

            QString message;
            in >> message;

            if (message.length() > MAX_MESSAGE_LENGTH) {
                replyToClient(client, Reply_type::WRONG_MESSAGE);
                return;
            }

            QTime time = QTime::currentTime();
            QDate date = QDate::currentDate();

            m_topics[topicNum]->addMessage(client->name(), time, date, message);
            replyToClient(client, Reply_type::OK);
        }
        else if (request == Request_type::GET_TOPIC_HISTORY) {

        }
        else if (request == Request_type::GET_LASTEST_MESSAGES_FROM_TOPIC) {

        }
        else {
            replyToClient(client, Reply_type::UNKNOWN_REQUEST);
            return;
        }
    }
}

void VkKillerServer::replyToClient(VkKillerClient* client, quint8 reply_type, const QString& msg) {
    QByteArray 	buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    out << quint16(0) << reply_type << msg;
    out.device()->seek(0);
    out << quint16(buffer.size() - sizeof(quint16));
    client->write(buffer);
}


void VkKillerServer::disconnectClient() {
    VkKillerClient* client = static_cast<VkKillerClient*>(sender());
    m_clients.erase(client->socketDescriptor());
    client->close();
}