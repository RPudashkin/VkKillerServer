﻿#ifndef VKKILLER_SERVER_H
#define VKKILLER_SERVER_H

#include <QTcpServer>
#include <QMutex>
#include <array>
#include "vkkiller_topic.h"

class QString;
class QHostAddress;
class VkKillerClient;


class VkKillerServer: public QTcpServer {
    Q_OBJECT

public:
    explicit VkKillerServer(QObject* parent = nullptr);
   ~VkKillerServer();

    VkKillerServer(const VkKillerServer&)               = delete;
    VkKillerServer(VkKillerServer&)                     = delete;
    VkKillerServer& operator=(const VkKillerServer&)    = delete;
    VkKillerServer& operator=(VkKillerServer&&)         = delete;

    bool start(const QHostAddress& address, quint16 port, QString* errMsg = nullptr) noexcept;
    void stop                ()                                                      noexcept;
    void enableLogging       (bool flag)                                             noexcept;
    void enableLoggingFor    (const VkKillerClient* client, bool flag)               noexcept;

signals:
    void clientConnected     (const VkKillerClient* client);
    void clientDisconnected  (const VkKillerClient* client);

protected:
    void incomingConnection  (qintptr socketDescriptor);

private slots:
    void processClientRequest();
    void disconnectClient    ();

private:
    static constexpr quint16 MAX_TOPICS_AMOUNT      = 150;
    static constexpr quint16 MAX_MESSAGE_LENGTH     = 300;
    static constexpr quint8  MAX_CLIENT_NAME_LENGTH = 32;
    static constexpr quint8  MAX_TOPIC_NAME_LENGTH  = 150;
    static constexpr int     MESSAGING_COOLDOWN     = 15;

    QMap<quintptr, VkKillerClient*>                 m_clients;
    std::array<VkKillerTopic, MAX_TOPICS_AMOUNT>    m_topics;
    QMutex                                          m_openTopicMutex;
    quint16                                         m_openTopicsAmount;
    bool                                            m_loggingEnabled;

    void replyToClient(VkKillerClient* client, quint8 reply_type, const QString& msg = "") noexcept;
};

#endif // VKKILLER_SERVER_H
