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
    ../include/libKitsunemimiProjectNetwork/session.h \
    ../include/libKitsunemimiProjectNetwork/session_controller.h \
    callbacks.h \
    message_definitions.h \
    messages_processing/session_processing.h \
    messages_processing/heartbeat_processing.h \
    messages_processing/error_processing.h \
    internal_session_interface.h \
    session_handler.h \
    messages_processing/multiblock_data_processing.h \
    messages_processing/singleblock_data_processing.h \
    multiblock_io.h \
    reply_handler.h \
    message_blocker_handler.h

SOURCES += \
    session.cpp \
    session_constroller.cpp \
    session_handler.cpp \
    multiblock_io.cpp \
    replay_handler.cpp \
    message_blocker_handler.cpp

