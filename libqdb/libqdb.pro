QT       -= gui

TARGET = qdb
TEMPLATE = lib

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb-1.0
}
win32 {
    LIBS += -llibusb-1.0
}

DEFINES += LIBQDB_LIBRARY

SOURCES += \
    abstractconnection.cpp \
    interruptsignalhandler.cpp \
    protocol/qdbmessage.cpp \
    protocol/qdbtransport.cpp \
    stream.cpp \
    streampacket.cpp \
    usb/devicemanagement.cpp \
    usb/libusbcontext.cpp \
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
    usb/devicemanagement.h \
    usb/usbcommon.h \
    usb/usbconnection.h \
    usb/usbconnectionreader.h \
    usb/usbdevice.h \

unix {
    target.path = $$[QT_INSTALL_LIBS]
    INSTALLS += target
}

INCLUDEPATH += $$PWD/..
