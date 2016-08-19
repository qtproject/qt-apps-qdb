QT       -= gui

TARGET = qdb
TEMPLATE = lib

DEFINES += LIBQDB_LIBRARY

SOURCES += \
    abstractconnection.cpp \
    protocol/qdbmessage.cpp \
    protocol/qdbtransport.cpp \
    stream.cpp \
    streampacket.cpp \
    usb/usbconnection.cpp \
    usb/usbconnectionreader.cpp \
    interruptsignalhandler.cpp \

HEADERS += \
    abstractconnection.h \
    filepullcommon.h \
    filepushcommon.h \
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
    interruptsignalhandler.h \

unix {
    LIBS += -lusb-1.0
    target.path = /usr/lib
    INSTALLS += target
}

win32 {
    LIBS += -llibusb-1.0
}

INCLUDEPATH += $$PWD
INCLUDEPATH += $$[QT_SYSROOT]/usr/include/libusb-1.0/
