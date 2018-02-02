#include <QHostAddress>
#include "vkkiller_client.h"


VkKillerClient::VkKillerClient(qintptr socketDescriptor, QObject* parent):
    QTcpSocket          (parent),
    m_name              ("anonymous"),
    m_id                (socketDescriptor),
    m_selectedTopicNum  (0),
    m_lastReadMsgNum    (0),
    m_loggingEnabled    (false)

{
    setSocketDescriptor(socketDescriptor);
}


QString VkKillerClient::name() const noexcept {
    return m_name;
}


qintptr VkKillerClient::id() const noexcept {
    return m_id;
}


QHostAddress VkKillerClient::address() const noexcept {
    return peerAddress();
}


QStringList VkKillerClient::logs() const noexcept {
    return m_logs;
}


void VkKillerClient::addEntryToLogs(
        const QString& entry,
        const QTime&   time,
        const QDate&   date) noexcept
{
    QString tmp = "["
            + time.toString()
            + "]    ["
            + date.toString("dd.MM.yyyy")
            + "]\n"
            + entry
            + "\n";
    m_logs << tmp;
}


bool VkKillerClient::isValidName(const QString& name) noexcept {
    for (QChar ch: name) {
        if (!(ch.isLetterOrNumber() || ch == '#' || ch == '_'))
            return false;
    }

    return true;
}
