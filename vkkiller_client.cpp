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


void VkKillerClient::move(VkKillerClient&& client) noexcept {
    m_name 						= std::move(client.m_name);
    m_logs						= std::move(client.m_logs);
    m_id   						= client.m_id;
    m_selectedTopicNum 			= client.m_selectedTopicNum;
    m_lastReadMsgNum 			= client.m_lastReadMsgNum;
    m_loggingEnabled 			= client.m_loggingEnabled;
    client.m_name 				= "anonymous";
    client.m_id					= 0;
    client.m_selectedTopicNum 	= 0;
    client.m_lastReadMsgNum 	= 0;
    client.m_loggingEnabled 	= false;
    client.m_logs.clear();

    setSocketDescriptor(client.socketDescriptor());
    client.close();
}


VkKillerClient::VkKillerClient(VkKillerClient&& client): QTcpSocket() {
    if (this != &client)
        move(std::move(client));
}


VkKillerClient& VkKillerClient::operator=(VkKillerClient&& client) {
    if (this != &client)
        move(std::move(client));
    return *this;
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
    if (!m_loggingEnabled) return;

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