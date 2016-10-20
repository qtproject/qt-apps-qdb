QT -= gui

TARGET = qdb
TEMPLATE = lib

DEFINES += LIBQDB_LIBRARY

SOURCES += \
    abstractconnection.cpp \
    interruptsignalhandler.cpp \
    protocol/qdbmessage.cpp \
    protocol/qdbtransport.cpp \
    stream.cpp \
    streampacket.cpp \

HEADERS += \
    abstractconnection.h \
    filepullcommon.h \
    filepushcommon.h \
    interruptsignalhandler.h \
    libqdb_global.h \
    processcommon.h \
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
