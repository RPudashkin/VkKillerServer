QT += network core gui

CONFIG += c++14
CONFIG -= app_bundle

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = VkKillerServer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    vkkiller_server.cpp 	        \
    vkkiller_client.cpp             \
    vkkiller_topic.cpp              \
    mainwindow.cpp                  \
    logs_dialog.cpp                 \
    main.cpp                        \

HEADERS += \
    vkkiller_server.h               \
    vkkiller_client.h               \
    vkkiller_topic.h                \
    vkkiller_request_reply.h        \
    vkkiller_server_constants.h     \
    mainwindow.h                    \
    logs_dialog.h

FORMS  += \
    mainwindow.ui                   \
    logs_dialog.ui