﻿#include <QStringBuilder>
#include <QMutexLocker>
#include <cmath>

#include "vkkiller_topic.h"
#include "vkkiller_server_constants.h"
#include "vkkiller_client.h"


int VkKillerTopic::m_openTopicsAmount = 0;


VkKillerTopic::VkKillerTopic(QObject* parent):
    QObject     (parent),
    m_name      (""),
    m_rating    (0),
    m_closed    (true)
{
    m_history.reserve(MESSAGES_RESERVED);
    connect(&m_updateRatingTimer, SIGNAL(timeout()), this, SLOT(updateRating()));
}


VkKillerTopic::~VkKillerTopic() { close(); }


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


VkKillerTopic::Entry::Entry(
        QString&&      authorName,
        const size_t   authorId,
        const QTime&   time,
        const QDate&   date,
        QString&&      message):
    authorName      (std::move(authorName)),
    authorId        (authorId),
    time            (time),
    date            (date),
    message         (std::move(message))
{}


void VkKillerTopic::Entry::move(Entry&& entry) noexcept {
    authorName      = std::move(entry.authorName);
    message         = std::move(entry.message);
    time            = entry.time;
    date            = entry.date;
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
    QMutexLocker locker(&m_synchWritersMutex);
    if (!m_closed) return false;

    m_name      = topicName;
    m_openTime  = QTime::currentTime();
    m_openDate  = QDate::currentDate();
    m_rating    = 1;
    m_closed    = false;

    m_openTopicsAmount++;
    m_updateRatingTimer.start(UPDATE_RATING_FREQUENCY);
    return true;
}


void VkKillerTopic::close() noexcept {
    QMutexLocker locker(&m_synchWritersMutex);
    if (m_closed) return;

    m_name   = "";
    m_closed = true;
    m_rating = 0;
    m_history.clear();
    m_updateRatingTimer.stop();

    for (auto& reader: m_readers)
        reader = nullptr;
    m_readers.clear();

    m_openTopicsAmount--;
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


bool VkKillerTopic::isValidTopicName(const QString& topicName) noexcept {
    int len = topicName.length();

    if (len > Server_constant::MAX_TOPIC_NAME_LENGTH || !len)
        return false;

    if (topicName[len-1] == Server_constant::SEPARATING_CH)
        return false;

    return true;
}


bool VkKillerTopic::isValidMessage(const QString& message) noexcept {
    int len = message.length();

    if (len > Server_constant::MAX_MESSAGE_LENGTH || !len)
        return false;

    if (message[len-1] == Server_constant::SEPARATING_CH)
        return false;

    return true;
}


bool VkKillerTopic::addReader(VkKillerClient* client) noexcept {
    if (m_closed) return false;
    QMutexLocker locker(&m_synchReadersMutex);
    m_readers[client->id()] = client;
    return true;
}


void VkKillerTopic::delReader(VkKillerClient* client) noexcept {
    if (m_closed) return;
    QMutexLocker locker(&m_synchReadersMutex);
    m_readers.remove(client->id());
}


QMap<size_t, VkKillerClient*> VkKillerTopic::getReaders() const noexcept {
    return m_readers;
}


void VkKillerTopic::addMessage(
    const QString& authorName,
    const size_t   authorId,
    const QTime&   time,
    const QDate&   date,
    const QString& message) noexcept
{
    QMutexLocker locker(&m_synchWritersMutex);
    m_history.emplace_back(authorName, authorId, time, date, message);
}


void VkKillerTopic::addMessage(
    QString&&      authorName,
    const size_t   authorId,
    const QTime&   time,
    const QDate&   date,
    QString&&      message) noexcept
{
    QMutexLocker locker(&m_synchWritersMutex);
    m_history.emplace_back(std::move(authorName), authorId, time, date, std::move(message));
}


QString VkKillerTopic::getPackedHistory(size_t msgNum) const noexcept {
    using Server_constant::SEPARATING_CH;

    QString outstr = "";
    size_t  last = m_history.size() - 1;

    for (size_t i = msgNum; i < last; ++i) {
        outstr = outstr
                % m_history[i].authorName                   % SEPARATING_CH
                % QString::number(m_history[i].authorId)    % SEPARATING_CH
                % m_history[i].time.toString()              % SEPARATING_CH
                % m_history[i].date.toString("dd.MM.yyyy")  % SEPARATING_CH
                % m_history[i].message                      % SEPARATING_CH;
    }

    outstr = outstr
           % m_history[last].authorName                   % SEPARATING_CH
           % QString::number(m_history[last].authorId)    % SEPARATING_CH
           % m_history[last].time.toString()              % SEPARATING_CH
           % m_history[last].date.toString("dd.MM.yyyy")  % SEPARATING_CH
           % m_history[last].message;


    return outstr;
}


void VkKillerTopic::updateRating() noexcept {
    QTime currTime       = QTime::currentTime();
    QDate currDate       = QDate::currentDate();

    float secsLife       = m_openTime.secsTo(currTime);
    float daysLife       = m_openDate.daysTo(currDate);
    float minutesLife    = secsLife / 60.0f + daysLife * 1440.0f;

    int   msgAmount      = m_history.size();
    int   lastMsg        = msgAmount - 1;
    QTime lastMsgTime    = m_history[lastMsg].time;
    QDate lastMsgDate    = m_history[lastMsg].date;
    float secsLastMsg    = lastMsgTime.secsTo(currTime);
    float daysLastMsg    = lastMsgDate.daysTo(currDate);
    float minutesLastMsg = secsLastMsg / 60.0f + daysLastMsg * 1440.0f;

    float alpha          = std::abs(MESSAGES_RESERVED / 2 - msgAmount);
    float beta           = std::abs((int)Server_constant::MAX_TOPICS_AMOUNT / 2 - m_openTopicsAmount);
    float lambda         = 1.0f / 12.0f;
    m_rating             = 10000.0f / std::exp(std::pow(minutesLife * minutesLastMsg, lambda)) - alpha - beta;

    if (m_rating <= 0)
        close();
}