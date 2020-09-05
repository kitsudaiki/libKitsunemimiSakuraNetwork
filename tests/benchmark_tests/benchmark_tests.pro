include(../../defaults.pri)

QT -= qt core gui

CONFIG   -= app_bundle
CONFIG += c++14 console

LIBS += -L../../src -lKitsunemimiSakuraNetwork
INCLUDEPATH += $$PWD

LIBS += -L../../../libKitsunemimiCommon/src -lKitsunemimiCommon
LIBS += -L../../../libKitsunemimiCommon/src/debug -lKitsunemimiCommon
LIBS += -L../../../libKitsunemimiCommon/src/release -lKitsunemimiCommon
INCLUDEPATH += ../../../libKitsunemimiCommon/include

LIBS += -L../../../libKitsunemimiArgs/src -lKitsunemimiArgs
LIBS += -L../../../libKitsunemimiArgs/src/debug -lKitsunemimiArgs
LIBS += -L../../../libKitsunemimiArgs/src/release -lKitsunemimiArgs
INCLUDEPATH += ../../../libKitsunemimiArgs/include

LIBS += -L../../../libKitsunemimiNetwork/src -lKitsunemimiNetwork
LIBS += -L../../../libKitsunemimiNetwork/src/debug -lKitsunemimiNetwork
LIBS += -L../../../libKitsunemimiNetwork/src/release -lKitsunemimiNetwork
INCLUDEPATH += ../../../libKitsunemimiNetwork/include

LIBS += -L../../../libKitsunemimiPersistence/src -lKitsunemimiPersistence
LIBS += -L../../../libKitsunemimiPersistence/src/debug -lKitsunemimiPersistence
LIBS += -L../../../libKitsunemimiPersistence/src/release -lKitsunemimiPersistence
INCLUDEPATH += ../../../libKitsunemimiPersistence/include

LIBS +=  -lssl -lcrypt
LIBS +=  -lboost_filesystem -lboost_system


SOURCES += \
    main.cpp \
    test_session.cpp

HEADERS += \
    test_session.h

