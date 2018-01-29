#ifndef VKKILLER_CLIENT_H
#define VKKILLER_CLIENT_H

#include <QTcpSocket>

class VkKillerServer;


class VkKillerClient: private QTcpSocket {
    friend class VkKillerServer;

public:
    VkKillerClient(qintptr socketDescriptor, QObject* parent = nullptr);

    VkKillerClient(const VkKillerClient&)            = delete;
    VkKillerClient(VkKillerClient&&)                 = delete;
    VkKillerClient& operator=(const VkKillerClient&) = delete;
    VkKillerClient& operator=(VkKillerClient&&)      = delete;

    QString name() const noexcept;
    qintptr id()   const noexcept;

    static bool isValidName(const QString& name) noexcept;

private:
    QString     m_name;
    qintptr     m_id;
    quint16     m_selectedTopicNum;
    size_t      m_lastReadMsgNum;
};

#endif // VKKILLER_CLIENT_H
