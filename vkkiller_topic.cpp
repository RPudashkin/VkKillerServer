#include "vkkiller_client.h"
#include "vkkiller_topic.h"


VkKillerTopic::VkKillerTopic(const QString& topicName, VkKillerClient* owner):
    m_name 		(topicName),
    m_owner		(owner),
    m_rating	(0)
{}


VkKillerTopic::~VkKillerTopic()
{
    m_owner = nullptr;
}


QString VkKillerTopic::name()  const noexcept {
    return m_name;
}

qint16 VkKillerTopic::rating() const noexcept {
    return m_rating;
}