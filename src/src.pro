QT       -= qt core gui

TARGET = KitsuneProjectCommon
TEMPLATE = lib
CONFIG += c++14
VERSION = 0.1.0

LIBS += -L../../libKitsuneCommon/src -lKitsuneCommon
LIBS += -L../../libKitsuneCommon/src/debug -lKitsuneCommon
LIBS += -L../../libKitsuneCommon/src/release -lKitsuneCommon
INCLUDEPATH += ../../libKitsuneCommon/include/libKitsuneCommon

LIBS += -L../../libKitsuneNetwork/src -lKitsuneNetwork
LIBS += -L../../libKitsuneNetwork/src/debug -lKitsuneNetwork
LIBS += -L../../libKitsuneNetwork/src/release -lKitsuneNetwork
INCLUDEPATH += ../../libKitsuneNetwork/include/libKitsuneNetwork

LIBS +=  -lssl -lcrypt

INCLUDEPATH += $$PWD \
               $$PWD/../include/libKitsuneProjectCommon

HEADERS += \
    session_protocol/timer_thread.h \
    session_protocol/session.h \
    session_protocol/session_handler.h \
    session_protocol/session_message_trigger.h \
    session_protocol/session_connection_trigger.h \
    session_protocol/messages.h

SOURCES += \
    session_protocol/timer_thread.cpp \
    session_protocol/session_handler.cpp \
    session_protocol/session_message_trigger.cpp \
    session_protocol/session_connection_trigger.cpp

