#include "vkkiller_client.h"


VkKillerClient::VkKillerClient(qintptr socketDescriptor, QObject* parent):
    QTcpSocket          (parent),
    m_name              ("anonymous"),
    m_id                (socketDescriptor),
    m_selectedTopicNum  (0),
    m_lastReadMsgNum    (0)
{
    setSocketDescriptor(socketDescriptor);
}


QString VkKillerClient::name() const noexcept {
    return m_name;
}


qintptr VkKillerClient::id() const noexcept {
    return m_id;
}


bool VkKillerClient::isValidName(const QString& name) noexcept {
    for (QChar ch: name) {
        if (!(ch.isLetterOrNumber() || ch == '#' || ch == '_'))
            return false;
    }

    return true;
}