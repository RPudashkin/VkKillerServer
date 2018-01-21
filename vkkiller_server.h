#ifndef VKKILLER_SERVER_H
#define VKKILLER_SERVER_H

#include <QTcpServer>
#include <QHostAddress>
#include <QString>
#include <vector>
#include <memory>
#include <mutex>
#include <map>


class VkKillerClient;
class VkKillerTopic;


class VkKillerServer: private QTcpServer {
    Q_OBJECT

public:
    VkKillerServer(QObject* parent = nullptr);
   ~VkKillerServer();

    VkKillerServer(const VkKillerServer&) 				= delete;
    VkKillerServer(VkKillerServer&) 	  				= delete;
    VkKillerServer& operator=(const VkKillerServer&) 	= delete;
    VkKillerServer& operator=(VkKillerServer&&) 		= delete;

    bool start(const QHostAddress& address, quint16 port, QString* errMsg = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor);

private slots:
    void readDataFromClient();
    void disconnectClient();

private:
    using uPtrToClient = std::unique_ptr<VkKillerClient>;
    using uPtrToTopic  = std::unique_ptr<VkKillerTopic>;

    std::map<qintptr, uPtrToClient> 	m_clients;
    std::vector<uPtrToTopic> 			m_topics;
    std::mutex							m_globalSynchMutex;
};

#endif // VKKILLER_SERVER_H