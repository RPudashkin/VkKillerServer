#ifndef VKKILLER_TOPIC_H
#define VKKILLER_TOPIC_H

#include <QMutex>
#include <QTime>
#include <QTimer>


class VkKillerTopic: private QObject {
    Q_OBJECT

public:
    explicit VkKillerTopic(QObject* parent = nullptr);
    VkKillerTopic(VkKillerTopic&&);
    VkKillerTopic& operator=(VkKillerTopic&&);

    VkKillerTopic(const VkKillerTopic&)             = delete;
    VkKillerTopic& operator=(const VkKillerTopic&)  = delete;

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

    // Thread-safe operation
    void addMessage(const QString& authorName,
                    const size_t   authorId,
                    const QTime&   time,
                    const QDate&   date,
                    const QString& message) noexcept;

    // Get a packed in QString topic history
    // beginning with selected message number
    QString getPackedHistory(size_t msgNum = 0) const noexcept;

private:
    // Move topic to *this
    void move(VkKillerTopic&& topic) noexcept;

    struct Entry {
        Entry(const QString& authorName,
              const size_t   authorId,
              const QTime&   time,
              const QDate&   date,
              const QString& message);

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
    static constexpr int    UPDATE_RATING_FREQUENCY = 300000; // every 5 minutes
    static constexpr size_t MESSAGES_RESERVED       = 300;

    QString             m_name;
    QTime               m_openTime;
    QDate               m_openDate;
    int                 m_rating;
    bool                m_closed;
    QMutex              m_mutex;
    std::vector<Entry>  m_history; // full topic history
    QTimer              m_updateRatingTimer;
};

#endif // VKKILLER_TOPIC_H