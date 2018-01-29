QT -= gui console
QT += network core

CONFIG += c++14
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    vkkiller_server.cpp \
    vkkiller_client.cpp \
    vkkiller_topic.cpp

HEADERS += \
    vkkiller_server.h \
    vkkiller_client.h \
    vkkiller_topic.h \
    vkkiller_request_reply.h
