#ifndef VKKILLER_CLIENT_H
#define VKKILLER_CLIENT_H

#include <QTcpSocket>
#include <QTime>
#include <QStringList>

class VkKillerServer;
class QHostAddress;


class VkKillerClient: private QTcpSocket {
    friend class VkKillerServer;

public:
    VkKillerClient(qintptr socketDescriptor, QObject* parent = nullptr);

    VkKillerClient(const VkKillerClient&)            = delete;
    VkKillerClient(VkKillerClient&&)                 = delete;
    VkKillerClient& operator=(const VkKillerClient&) = delete;
    VkKillerClient& operator=(VkKillerClient&&)      = delete;

    QString      name   () const noexcept;
    qintptr      id     () const noexcept;
    QHostAddress address() const noexcept;
    QStringList  logs   () const noexcept;

    void addEntryToLogs(const QString& entry,
                        const QTime&   time = QTime::currentTime(),
                        const QDate&   date = QDate::currentDate()) noexcept;

    static bool isValidName(const QString& name) noexcept;

private:
    QString     m_name;
    qintptr     m_id;
    QTime       m_lastMessageTime;
    quint16     m_selectedTopicNum;
    size_t      m_lastReadMsgNum;
    bool        m_loggingEnabled;
    QStringList m_logs;
};

#endif // VKKILLER_CLIENT_H
