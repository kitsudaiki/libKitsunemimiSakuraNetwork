include(../../defaults.pri)

QT -= qt core gui

CONFIG   -= app_bundle
CONFIG += c++17 console

LIBS += -L../../src -lKitsunemimiSakuraNetwork
INCLUDEPATH += $$PWD

LIBS += -L../../../libKitsunemimiCommon/src -lKitsunemimiCommon
LIBS += -L../../../libKitsunemimiCommon/src/debug -lKitsunemimiCommon
LIBS += -L../../../libKitsunemimiCommon/src/release -lKitsunemimiCommon
INCLUDEPATH += ../../../libKitsunemimiCommon/include

LIBS += -L../../../libKitsunemimiNetwork/src -lKitsunemimiNetwork
LIBS += -L../../../libKitsunemimiNetwork/src/debug -lKitsunemimiNetwork
LIBS += -L../../../libKitsunemimiNetwork/src/release -lKitsunemimiNetwork
INCLUDEPATH += ../../../libKitsunemimiNetwork/include

LIBS +=  -lssl -lcrypt
LIBS +=  -lboost_program_options -lboost_system


SOURCES += \
    main.cpp \
    test_session.cpp

HEADERS += \
    test_session.h

