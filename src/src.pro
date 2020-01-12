QT       -= qt core gui

TARGET = KitsunemimiProjectNetwork
TEMPLATE = lib
CONFIG += c++14
VERSION = 0.1.0

LIBS += -L../../libKitsunemimiCommon/src -lKitsunemimiCommon
LIBS += -L../../libKitsunemimiCommon/src/debug -lKitsunemimiCommon
LIBS += -L../../libKitsunemimiCommon/src/release -lKitsunemimiCommon
INCLUDEPATH += ../../libKitsunemimiCommon/include

LIBS += -L../../libKitsunemimiNetwork/src -lKitsunemimiNetwork
LIBS += -L../../libKitsunemimiNetwork/src/debug -lKitsunemimiNetwork
LIBS += -L../../libKitsunemimiNetwork/src/release -lKitsunemimiNetwork
INCLUDEPATH += ../../libKitsunemimiNetwork/include

LIBS += -L../../libKitsunemimiPersistence/src -lKitsunemimiPersistence
LIBS += -L../../libKitsunemimiPersistence/src/debug -lKitsunemimiPersistence
LIBS += -L../../libKitsunemimiPersistence/src/release -lKitsunemimiPersistence
INCLUDEPATH += ../../libKitsunemimiPersistence/include

LIBS +=  -lssl -lcrypt

INCLUDEPATH += $$PWD \
               $$PWD/../include

HEADERS += \
    ../include/libKitsunemimiProjectNetwork/network_session/session.h \
    network_session/timer_thread.h \
    network_session/callbacks.h \
    network_session/message_definitions.h \
    network_session/messages_processing/session_processing.h \
    network_session/messages_processing/heartbeat_processing.h \
    network_session/messages_processing/error_processing.h \
    ../include/libKitsunemimiProjectNetwork/network_session/session_controller.h \
    network_session/internal_session_interface.h \
    network_session/session_handler.h \
    network_session/messages_processing/multiblock_data_processing.h \
    network_session/messages_processing/singleblock_data_processing.h

SOURCES += \
    network_session/timer_thread.cpp \
    network_session/session.cpp \
    network_session/session_constroller.cpp \
    network_session/internal_session_interface.cpp \
    network_session/session_handler.cpp

