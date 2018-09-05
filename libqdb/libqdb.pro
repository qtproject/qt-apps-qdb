QT -= gui

TARGET = qdb
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    abstractconnection.cpp \
    protocol/qdbmessage.cpp \
    protocol/qdbtransport.cpp \
    stream.cpp \
    streampacket.cpp \

win32 {
    SOURCES += interruptsignalhandler_win.cpp
} else {
    SOURCES += interruptsignalhandler_unix.cpp
}

HEADERS += \
    abstractconnection.h \
    interruptsignalhandler.h \
    protocol/protocol.h \
    protocol/qdbmessage.h \
    protocol/qdbtransport.h \
    protocol/services.h \
    stream.h \
    streampacket.h \

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

INCLUDEPATH += $$PWD/..
