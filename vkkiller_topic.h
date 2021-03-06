﻿#ifndef VKKILLER_TOPIC_H
#define VKKILLER_TOPIC_H

#include <QMutex>
#include <QTime>
#include <QTimer>
#include <QMap>

class VkKillerClient;


class VkKillerTopic: private QObject {
    Q_OBJECT

public:
    explicit VkKillerTopic(QObject* parent = nullptr);
    ~VkKillerTopic();

    VkKillerTopic(const VkKillerTopic&)             = delete;
    VkKillerTopic& operator=(const VkKillerTopic&)  = delete;
    VkKillerTopic(VkKillerTopic&&)                  = delete;
    VkKillerTopic& operator=(VkKillerTopic&&)       = delete;


    // Open a topic for discussion
    // By default all topics are closed
    // Thread-safe operation
    bool open(const QString& topicName) noexcept;

    bool    closed() const noexcept;
    QString name  () const noexcept;
    int     rating() const noexcept;
    size_t  size  () const noexcept;

    static bool isValidTopicName(const QString& topicName) noexcept;
    static bool isValidMessage  (const QString& message)   noexcept;

    // Thread-safe operations
    bool addReader(VkKillerClient* client) noexcept;
    void delReader(VkKillerClient* client) noexcept;

    QMap<size_t, VkKillerClient*> getReaders() const noexcept;

    // Thread-safe operation
    void addMessage(const QString& authorName,
                    const size_t   authorId,
                    const QTime&   time,
                    const QDate&   date,
                    const QString& message) noexcept;

    void addMessage(QString&&      authorName,
                    const size_t   authorId,
                    const QTime&   time,
                    const QDate&   date,
                    QString&&      message) noexcept;


    // Get a packed in QString topic history
    // beginning with selected message number
    QString getPackedHistory(size_t msgNum = 0) const noexcept;

private:
    struct Entry {
        Entry(const QString& authorName,
              const size_t   authorId,
              const QTime&   time,
              const QDate&   date,
              const QString& message);

        Entry(QString&&      authorName,
              const size_t   authorId,
              const QTime&   time,
              const QDate&   date,
              QString&&      message);

        Entry(Entry&&);
        Entry& operator=(Entry&&);

        // Move entry to *this
        void move(Entry&& entry) noexcept;

        QString authorName;
        size_t  authorId;
        QTime   time;
        QDate   date;
        QString message;

        Entry(const Entry&)             = delete;
        Entry& operator=(const Entry&)  = delete;
    };

    // Thread-safe operation
    void close() noexcept;

private slots:
    void updateRating() noexcept;

private:
    static constexpr int UPDATE_RATING_FREQUENCY = 180000; // every 3 minutes
    static constexpr int MESSAGES_RESERVED       = 300;
    static           int m_openTopicsAmount;

    QString                         m_name;
    QTime                           m_openTime;
    QDate                           m_openDate;
    int                             m_rating;
    bool                            m_closed;
    QMutex                          m_synchReadersMutex;
    QMutex                          m_synchWritersMutex;
    QMap<size_t, VkKillerClient*>   m_readers;
    std::vector<Entry>              m_history; // full topic history
    QTimer                          m_updateRatingTimer;
};

#endif // VKKILLER_TOPIC_H