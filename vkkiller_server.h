#ifndef VKKILLER_SERVER_H
#define VKKILLER_SERVER_H

#include <QTcpServer>
#include <QMutex>
#include <array>
#include <memory>
#include <map>
#include "vkkiller_topic.h"

class QString;
class QHostAddress;
class VkKillerClient;


class VkKillerServer: private QTcpServer {
    Q_OBJECT

public:
    explicit VkKillerServer(QObject* parent = nullptr);
   ~VkKillerServer();

    VkKillerServer(const VkKillerServer&)               = delete;
    VkKillerServer(VkKillerServer&)                     = delete;
    VkKillerServer& operator=(const VkKillerServer&)    = delete;
    VkKillerServer& operator=(VkKillerServer&&)         = delete;

    bool start(const QHostAddress& address, quint16 port, QString* errMsg = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor);

private slots:
    void processClientRequest();
    void disconnectClient();

private:
    using uPtrToClient = std::unique_ptr<VkKillerClient>;

    static constexpr quint16 MAX_TOPICS_AMOUNT      = 150;
    static constexpr quint16 MAX_MESSAGE_LENGTH     = 300;
    static constexpr quint8  MAX_CLIENT_NAME_LENGTH = 32;
    static constexpr quint8  MAX_TOPIC_NAME_LENGTH  = 150;

    std::map<qintptr, uPtrToClient>                 m_clients;
    std::array<VkKillerTopic, MAX_TOPICS_AMOUNT>    m_topics;
    QMutex                                          m_globalSynchMutex;
    QMutex                                          m_openTopicMutex;
    quint16                                         m_openTopicsAmount;

    void replyToClient(VkKillerClient* client, quint8 reply_type, const QString& msg = "") noexcept;
};

#endif // VKKILLER_SERVER_H