#include <QString>
#include <QTime>
#include <QDate>
#include "vkkiller_topic.h"


VkKillerTopic::VkKillerTopic(const QString& topicName):
    m_name 		(topicName),
    m_rating	(0)
{}


VkKillerTopic::~VkKillerTopic() {}


VkKillerTopic::Entry::Entry(
        const QString& authorName,
        const QTime&   time,
        const QDate&   date,
        const QString& message):
    authorName	(authorName),
    time		(time),
    date		(date),
    message		(message)
{}


VkKillerTopic::Entry::~Entry() {}


QString VkKillerTopic::name()  const noexcept {
    return m_name;
}


qint16 VkKillerTopic::rating() const noexcept {
    return m_rating;
}


void VkKillerTopic::addMessage(
    const QString& authorName,
    const QTime&   time,
    const QDate&   date,
    const QString& message)
{
    std::lock_guard<std::mutex> locker(m_newMessageMutex);
}