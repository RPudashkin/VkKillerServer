#include "vkkiller_client.h"
#include "vkkiller_topic.h"


VkKillerClient::VkKillerClient(qintptr socketDescriptor, QObject* parent):
    QTcpSocket			(parent),
    m_name 			  	("anonymous"),
    m_topicInOwnership	(nullptr),
    m_selectedTopic		(nullptr)
{
    setSocketDescriptor(socketDescriptor);
}


VkKillerClient::~VkKillerClient()
{
    m_topicInOwnership 	= nullptr;
    m_selectedTopic		= nullptr;
}


QString VkKillerClient::name() const noexcept {
    return m_name;
}