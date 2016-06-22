QT       -= gui

TARGET = qdb
TEMPLATE = lib

DEFINES += QDB_LIBRARY

SOURCES += \
    abstractconnection.cpp \
    interruptsignalhandler.cpp \
    protocol/qdbmessage.cpp \
    protocol/qdbtransport.cpp \
    stream.cpp \
    streampacket.cpp \
    usb/usbconnection.cpp \
    usb/usbconnectionreader.cpp \

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
    usb/usbconnection.h \
    usb/usbconnectionreader.h \

INCLUDEPATH += $$PWD

INCLUDEPATH += $$[QT_SYSROOT]/usr/include/libusb-1.0/
LIBS += -lusb-1.0

unix {
    target.path = /usr/lib
    INSTALLS += target
}
