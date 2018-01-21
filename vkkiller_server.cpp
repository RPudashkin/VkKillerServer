#include <QByteArray>
#include <QDataStream>

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
            this, SLOT(readDataFromClient()));
}


void VkKillerServer::readDataFromClient() {
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

        QByteArray bytesBlock;
        in >> bytesBlock;

        // Process bytesBlock here
    }
}


void VkKillerServer::disconnectClient() {
    VkKillerClient* client = static_cast<VkKillerClient*>(sender());
    m_clients.erase(client->socketDescriptor());
    client->close();
}