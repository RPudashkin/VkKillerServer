#ifndef VKKILLER_TOPIC_H
#define VKKILLER_TOPIC_H

#include <QTime>
#include <QDate>
#include <mutex>

class QString;
class VkKillerServer;


class VkKillerTopic {
    friend class VkKillerServer;

public:
    VkKillerTopic(const QString& topicName);
   ~VkKillerTopic();

    VkKillerTopic(const VkKillerTopic&) 			= delete;
    VkKillerTopic(VkKillerTopic&&) 					= delete;
    VkKillerTopic& operator=(const VkKillerTopic&) 	= delete;
    VkKillerTopic& operator=(VkKillerTopic&&) 		= delete;

    QString name() 		const noexcept;
    qint16	rating() 	const noexcept;

    void addMessage(const QString& authorName,
                    const QTime&   time,
                    const QDate&   date,
                    const QString& message);
private:
    struct Entry {
        Entry(const QString& authorName,
              const QTime&   time,
              const QDate& 	 date,
              const QString& message);
        ~Entry();

        QString authorName;
        QTime	time;
        QDate	date;
        QString	message;

        Entry() 									= delete;
        Entry(const Entry&) 						= delete;
        Entry(Entry&&) 								= delete;
        Entry& operator=(const Entry&) 				= delete;
        Entry& operator=(Entry&&) 					= delete;
    };

private:
    QString 			m_name;
    qint16				m_rating;
    std::mutex			m_newMessageMutex;

    // 150 characters per topic name this is maximum
    static constexpr quint8 MAX_NAME_LENGTH = 150;
};

#endif // VKKILLER_TOPIC_H