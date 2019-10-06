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
    ../include/libKitsuneProjectCommon/network_session/session.h \
    ../include/libKitsuneProjectCommon/network_session/session_handler.h \
    network_session/timer_thread.h \
    network_session/messages.h \
    network_session/callbacks.h \
    network_session/message_handler.h

SOURCES += \
    network_session/timer_thread.cpp \
    network_session/session_handler.cpp \
    network_session/message_handler.cpp

