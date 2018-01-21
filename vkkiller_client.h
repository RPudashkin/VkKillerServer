#ifndef VKKILLER_CLIENT_H
#define VKKILLER_CLIENT_H

#include <QTcpSocket>


class VkKillerServer;
class VkKillerTopic;


class VkKillerClient: private QTcpSocket {
    friend class VkKillerServer;

public:
    VkKillerClient(qintptr socketDescriptor, QObject* parent = nullptr);
   ~VkKillerClient();

    VkKillerClient(const VkKillerClient&) 			 = delete;
    VkKillerClient(VkKillerClient&&) 	  			 = delete;
    VkKillerClient& operator=(const VkKillerClient&) = delete;
    VkKillerClient& operator=(VkKillerClient&&) 	 = delete;

    QString name() const noexcept;

private:
    QString 		m_name;
    VkKillerTopic* 	m_topicInOwnership;
    VkKillerTopic*  m_selectedTopic;
};

#endif // VKKILLER_CLIENT_H