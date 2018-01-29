#include <QString>
#include "vkkiller_topic.h"


VkKillerTopic::VkKillerTopic(QObject* parent):
    QObject     (parent),
    m_name      (""),
    m_rating    (0),
    m_closed    (true)
{
    constexpr size_t MESSAGES_RESERVED = 300;
    m_history.reserve(MESSAGES_RESERVED);

    connect(&m_updateRatingTimer, SIGNAL(timeout()), this, SLOT(updateRating()));
}


void VkKillerTopic::move(VkKillerTopic&& topic) noexcept {
    m_name          = std::move(topic.m_name);
    m_history       = std::move(topic.m_history);
    m_openTime      = std::move(topic.m_openTime);
    m_openDate      = std::move(topic.m_openDate);
    m_rating        = topic.m_rating;
    m_closed        = topic.m_closed;
    topic.m_rating  = 0;
    topic.m_closed  = true;
}


VkKillerTopic::VkKillerTopic(VkKillerTopic&& topic) {
    if (this != &topic)
        move(std::move(topic));
}


VkKillerTopic& VkKillerTopic::operator=(VkKillerTopic&& topic) {
    if (this != &topic)
        move(std::move(topic));
    return *this;
}


VkKillerTopic::Entry::Entry(
        const QString& authorName,
        const size_t   authorId,
        const QTime&   time,
        const QDate&   date,
        const QString& message):
    authorName      (authorName),
    authorId        (authorId),
    time            (time),
    date            (date),
    message         (message)
{}


void VkKillerTopic::Entry::move(Entry&& entry) noexcept {
    authorName      = std::move(entry.authorName);
    time            = std::move(entry.time);
    date            = std::move(entry.date);
    message         = std::move(entry.message);
    authorId        = entry.authorId;
}


VkKillerTopic::Entry::Entry(Entry&& entry) {
    if (this != &entry)
        Entry::move(std::move(entry));
}


VkKillerTopic::Entry& VkKillerTopic::Entry::operator=(Entry&& entry) {
    if (this != &entry)
        Entry::move(std::move(entry));
    return *this;
}

bool VkKillerTopic::open(const QString& topicName) noexcept {
    std::lock_guard<std::mutex> locker(m_mutex);
    if (!m_closed) return false;

    m_name      = topicName;
    m_openTime  = QTime::currentTime();
    m_openDate  = QDate::currentDate();
    m_rating    = 1;
    m_closed    = false;

    m_updateRatingTimer.start(UPDATE_RATING_FREQUENCY);

    return true;
}


void VkKillerTopic::close() noexcept {
    std::lock_guard<std::mutex> locker(m_mutex);
    if (m_closed) return;

    m_name   = "";
    m_closed = true;
    m_rating = 0;
    m_history.clear();
    m_updateRatingTimer.stop();
}


QString VkKillerTopic::name()  const noexcept {
    return m_name;
}


qint16 VkKillerTopic::rating() const noexcept {
    return m_rating;
}


size_t VkKillerTopic::size() const noexcept {
    return m_history.size();
}


bool VkKillerTopic::closed() const noexcept {
    return m_closed;
}


bool VkKillerTopic::isValidMessage(const QString& message) noexcept {
    int len = message.length();
    if (message[len-1] == SEPARATING_CH)
        return false;

    return true;
}


void VkKillerTopic::addMessage(
    const QString& authorName,
    const size_t   authorId,
    const QTime&   time,
    const QDate&   date,
    const QString& message) noexcept
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_history.push_back(Entry(authorName, authorId, time, date, message));
}


QString VkKillerTopic::getPackedHistory(size_t msgNum) const noexcept {
    QString outstr = "";

    for (size_t i = msgNum; i < m_history.size(); ++i) {
        outstr += m_history[i].authorName                   + SEPARATING_CH
                + QString::number(m_history[i].authorId)    + SEPARATING_CH
                + m_history[i].time.toString()              + SEPARATING_CH
                + m_history[i].date.toString()              + SEPARATING_CH
                + m_history[i].message                      + SEPARATING_CH;
    }

    return outstr;
}


void VkKillerTopic::updateRating() noexcept {
    if (m_rating <= 0)
        close();

    QTime currTime = QTime::currentTime();
    QDate currDate = QDate::currentDate();

    // some magic here
}
