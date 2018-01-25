#ifndef VKKILLER_REQUEST_REPLY_H
#define VKKILLER_REQUEST_REPLY_H


namespace Request_type {
    const static quint8 SET_NAME 							= 0x01;
    const static quint8	GET_TOPICS_LIST						= 0x02;
    const static quint8 CREATE_TOPIC 						= 0x03;
    const static quint8 TEXT_MESSAGE 						= 0x04;
    const static quint8 GET_TOPIC_HISTORY 					= 0x05;
    const static quint8 GET_LASTEST_MESSAGES_FROM_TOPIC 	= 0x06;
    const static quint8 GET_TOPIC_RATING					= 0x07;
}


namespace Reply_type {
    const static quint8 OK									= 0x01;
    const static quint8 WRONG_NAME 							= 0x02;
    const static quint8 FAILED_TOPIC_CREATE 				= 0x03;
    const static quint8 WRONG_TOPIC_NAME 					= 0x04;
    const static quint8 UNKNOWN_TOPIC 						= 0x05;
    const static quint8 WRONG_MESSAGE 						= 0x06;
    const static quint8 UNKNOWN_REQUEST						= 0x07;
}

#endif // VKKILLER_REQUEST_REPLY_H