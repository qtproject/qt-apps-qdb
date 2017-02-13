QT -= gui

TARGET = qdb
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    abstractconnection.cpp \
    interruptsignalhandler.cpp \
    protocol/qdbmessage.cpp \
    protocol/qdbtransport.cpp \
    stream.cpp \
    streampacket.cpp \

HEADERS += \
    abstractconnection.h \
    interruptsignalhandler.h \
    protocol/protocol.h \
    protocol/qdbmessage.h \
    protocol/qdbtransport.h \
    protocol/services.h \
    stream.h \
    streampacket.h \

unix {
    target.path = $$[QT_INSTALL_LIBS]
    INSTALLS += target
}

INCLUDEPATH += $$PWD/..
