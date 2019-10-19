QT       -= qt core gui

TARGET = KitsuneProjectCommon
TEMPLATE = lib
CONFIG += c++14
VERSION = 0.1.0

LIBS += -L../../libKitsuneCommon/src -lKitsuneCommon
LIBS += -L../../libKitsuneCommon/src/debug -lKitsuneCommon
LIBS += -L../../libKitsuneCommon/src/release -lKitsuneCommon
INCLUDEPATH += ../../libKitsuneCommon/include

LIBS += -L../../libKitsuneNetwork/src -lKitsuneNetwork
LIBS += -L../../libKitsuneNetwork/src/debug -lKitsuneNetwork
LIBS += -L../../libKitsuneNetwork/src/release -lKitsuneNetwork
INCLUDEPATH += ../../libKitsuneNetwork/include

LIBS += -L../../libKitsunePersistence/src -lKitsunePersistence
LIBS += -L../../libKitsunePersistence/src/debug -lKitsunePersistence
LIBS += -L../../libKitsunePersistence/src/release -lKitsunePersistence
INCLUDEPATH += ../../libKitsunePersistence/include

LIBS +=  -lssl -lcrypt

INCLUDEPATH += $$PWD \
               $$PWD/../include

HEADERS += \
    ../include/libKitsuneProjectCommon/network_session/session.h \
    network_session/timer_thread.h \
    network_session/callbacks.h \
    network_session/message_definitions.h \
    network_session/messages_processing/session_processing.h \
    network_session/messages_processing/heartbeat_processing.h \
    network_session/messages_processing/error_processing.h \
    ../include/libKitsuneProjectCommon/network_session/session_controller.h \
    network_session/internal_session_interface.h \
    network_session/session_handler.h

SOURCES += \
    network_session/timer_thread.cpp \
    network_session/session.cpp \
    network_session/session_constroller.cpp \
    network_session/internal_session_interface.cpp \
    network_session/session_handler.cpp

