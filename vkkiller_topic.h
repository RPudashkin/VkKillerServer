#ifndef VKKILLER_TOPIC_H
#define VKKILLER_TOPIC_H

class QString;
class VkKillerServer;
class VkKillerClient;


class VkKillerTopic {
    friend class VkKillerServer;

public:
    VkKillerTopic(const QString &topicName, VkKillerClient* owner);
   ~VkKillerTopic();

    VkKillerTopic(const VkKillerTopic&) 			= delete;
    VkKillerTopic(VkKillerTopic&&) 					= delete;
    VkKillerTopic& operator=(const VkKillerTopic&) 	= delete;
    VkKillerTopic& operator=(VkKillerTopic&&) 		= delete;

    QString name() 		const noexcept;
    qint16	rating() 	const noexcept;

private:
    QString 			m_name;
    VkKillerClient*		m_owner;
    qint16				m_rating;
};

#endif // VKKILLER_TOPIC_H