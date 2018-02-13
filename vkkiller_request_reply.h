#ifndef VKKILLER_REQUEST_REPLY_H
#define VKKILLER_REQUEST_REPLY_H

namespace Request_type {
    constexpr quint8 SET_NAME                       = 1;
    constexpr quint8 GET_TOPICS_LIST                = 2;
    constexpr quint8 CREATE_TOPIC                   = 3;
    constexpr quint8 TEXT_MESSAGE                   = 4;
    constexpr quint8 GET_TOPIC_HISTORY              = 5;
    constexpr quint8 GET_LAST_MESSAGES_FROM_TOPIC   = 6;
}

namespace Reply_type {
    constexpr quint8 CONNECTED                      = 1;
    constexpr quint8 TOPICS_LIST                    = 2;
    constexpr quint8 TOPIC_HISTORY                  = 3;
    constexpr quint8 LAST_MESSAGES                  = 4;
    constexpr quint8 THERE_IS_NEW_TOPIC				= 5;

    constexpr quint8 WRONG_NAME                     = 6;
    constexpr quint8 FAILED_TOPIC_CREATE            = 7;
    constexpr quint8 WRONG_TOPIC_NAME               = 8;
    constexpr quint8 UNKNOWN_TOPIC                  = 9;
    constexpr quint8 WRONG_MESSAGE                  = 10;
    constexpr quint8 UNKNOWN_REQUEST                = 11;
    constexpr quint8 TOO_FAST_MESSAGING             = 12;
    constexpr quint8 TOO_FAST_TOPIC_CREATING        = 13;
}

#endif // VKKILLER_REQUEST_REPLY_H