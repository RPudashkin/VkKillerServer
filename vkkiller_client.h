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
    VkKillerClient(VkKillerClient&&);
    VkKillerClient& operator=(VkKillerClient&&);

    VkKillerClient(const VkKillerClient&)            = delete;
    VkKillerClient& operator=(const VkKillerClient&) = delete;

    QString      name   () const noexcept;
    qintptr      id     () const noexcept;
    QHostAddress address() const noexcept;
    QStringList  logs   () const noexcept;

    static bool isValidName(const QString& name) noexcept;

private:
    // Move VkKillerClient to *this
    void move(VkKillerClient&&) noexcept;

    void addEntryToLogs(const QString& entry,
                        const QTime&   time = QTime::currentTime(),
                        const QDate&   date = QDate::currentDate()) noexcept;

    QString     m_name;
    qintptr     m_id;
    QTime       m_lastMessageTime;
    quint16     m_selectedTopicNum;
    size_t      m_lastReadMsgNum;
    bool        m_loggingEnabled;
    QStringList m_logs;
};

#endif // VKKILLER_CLIENT_H
