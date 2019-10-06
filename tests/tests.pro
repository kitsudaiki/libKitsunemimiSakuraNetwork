include(../defaults.pri)

QT -= qt core gui

CONFIG   -= app_bundle
CONFIG += c++14 console

LIBS += -L../../libKitsuneCommon/src -lKitsuneCommon
LIBS += -L../../libKitsuneCommon/src/debug -lKitsuneCommon
LIBS += -L../../libKitsuneCommon/src/release -lKitsuneCommon
INCLUDEPATH += ../../libKitsuneCommon/include/libKitsuneCommon

LIBS += -L../../libKitsuneNetwork/src -lKitsuneNetwork
LIBS += -L../../libKitsuneNetwork/src/debug -lKitsuneNetwork
LIBS += -L../../libKitsuneNetwork/src/release -lKitsuneNetwork
INCLUDEPATH += ../../libKitsuneNetwork/include/libKitsuneNetwork

LIBS += -L../../libKitsunePersistence/src -lKitsunePersistence
LIBS += -L../../libKitsunePersistence/src/debug -lKitsunePersistence
LIBS += -L../../libKitsunePersistence/src/release -lKitsunePersistence
INCLUDEPATH += ../../libKitsunePersistence/include/libKitsunePersistence

LIBS +=  -lssl -lcrypt
LIBS +=  -lboost_filesystem -lboost_system -lsqlite3

INCLUDEPATH += $$PWD

LIBS += -L../src -lKitsuneProjectCommon


SOURCES += \
    main.cpp
