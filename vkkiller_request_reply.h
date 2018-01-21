#ifndef VKKILLER_REQUEST_REPLY__H
#define VKKILLER_REQUEST_REPLY_H

enum class Request_type: quint8 {
    SET_NAME						= 0x01,
    CREATE_TOPIC					= 0x02,
    TEXT_MESSAGE  	   				= 0x03,
    GET_TOPIC_HISTORY   			= 0x04,
    GET_LASTEST_MESSAGES_AT_TOPIC	= 0x05,
};


enum class Reply_type: quint8 {
    OK								= 0x01,
    WRONG_NAME 						= 0x02,
    FAILED_TOPIC_CREATE 			= 0x03,
    WRONG_TOPIC_NAME 				= 0x04,
    UNKNOWN_TOPIC 					= 0x05,
    WRONG_MESSAGE 					= 0x06
};

#endif // VKKILLER_REQUEST_REPLY_H