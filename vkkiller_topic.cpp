#include <QString>
#include <QMutexLocker>
#include <cmath>

#include "vkkiller_topic.h"
#include "vkkiller_server_constants.h"

using Server_constant::SEPARATING_CH;


VkKillerTopic::VkKillerTopic(QObject* parent):
    QObject     (parent),
    m_name      (""),
    m_rating    (0),
    m_closed    (true)
{
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
    QMutexLocker locker(&m_mutex);
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
    QMutexLocker locker(&m_mutex);
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


int VkKillerTopic::rating() const noexcept {
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
    QMutexLocker locker(&m_mutex);
    m_history.emplace_back(authorName, authorId, time, date, message);
}


QString VkKillerTopic::getPackedHistory(size_t msgNum) const noexcept {
    QString outstr = "";
    size_t  last = m_history.size() - 1;

    for (size_t i = msgNum; i < last; ++i) {
        outstr += m_history[i].authorName                   + SEPARATING_CH
                + QString::number(m_history[i].authorId)    + SEPARATING_CH
                + m_history[i].time.toString()              + SEPARATING_CH
                + m_history[i].date.toString("dd.MM.yyyy")  + SEPARATING_CH
                + m_history[i].message                      + SEPARATING_CH;
    }

    outstr += m_history[last].authorName                  + SEPARATING_CH
           + QString::number(m_history[last].authorId)    + SEPARATING_CH
           + m_history[last].time.toString()              + SEPARATING_CH
           + m_history[last].date.toString("dd.MM.yyyy")  + SEPARATING_CH
           + m_history[last].message;


    return outstr;
}


void VkKillerTopic::updateRating() noexcept {
    QTime currTime    = QTime::currentTime();
    QDate currDate    = QDate::currentDate();

    int secsLife      = m_openTime.secsTo(currTime);
    int daysLife      = m_openDate.daysTo(currDate);
    int hoursLife     = secsLife / 3600 + daysLife * 24;

    int   lastMsg     = m_history.size() - 1;
    QTime lastMsgTime = m_history[lastMsg].time;
    QDate lastMsgDate = m_history[lastMsg].date;

    int secsLastMsg   = m_openTime.secsTo(lastMsgTime);
    int daysLastMsg   = m_openDate.daysTo(lastMsgDate);
    int hoursLastMsg  = secsLastMsg / 3600 + daysLastMsg * 24;

    int alpha         = 1 + std::abs((int)MESSAGES_RESERVED - lastMsg + 1);
    int beta          = hoursLife + std::pow(hoursLastMsg, 2);
    //m_rating          = 1000 / (alpha * beta);

    if (m_rating <= 0)
        close();
}