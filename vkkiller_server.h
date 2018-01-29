#ifndef VKKILLER_SERVER_H
#define VKKILLER_SERVER_H

#include <QTcpServer>
#include <array>
#include <memory>
#include <mutex>
#include <map>

class QString;
class QHostAddress;
class VkKillerClient;
class VkKillerTopic;


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
    using uPtrToTopic  = std::unique_ptr<VkKillerTopic>;

    static constexpr quint16 MAX_TOPICS_AMOUNT      = 150;
    static constexpr quint16 MAX_MESSAGE_LENGTH     = 300;
    static constexpr quint8  MAX_CLIENT_NAME_LENGTH = 32;
    static constexpr quint8  MAX_TOPIC_NAME_LENGTH  = 150;

    std::map<qintptr, uPtrToClient>             m_clients;
    std::array<uPtrToTopic, MAX_TOPICS_AMOUNT>  m_topics;
    std::mutex                                  m_globalSynchMutex;
    std::mutex                                  m_openTopicMutex;
    quint16                                     m_openTopicsAmount;

    void replyToClient(VkKillerClient* client, quint8 reply_type, const QString& msg = "") noexcept;
};

#endif // VKKILLER_SERVER_H
