QT -= gui
QT += dbus network

CONFIG += c++11

TARGET = qdb
win32: CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb-1.0
}
win32 {
    LIBS += -llibusb-1.0
}

HEADERS += \
    client/client.h \
    server/connection.h \
    server/deviceinformationfetcher.h \
    server/devicemanager.h \
    server/echoservice.h \
    server/handshakeservice.h \
    server/hostserver.h \
    server/logging.h \
    server/networkmanagercontrol.h \
    server/service.h \
    server/usb-host/usbcommon.h \
    server/usb-host/usbconnection.h \
    server/usb-host/usbconnectionreader.h \
    server/usb-host/usbdeviceenumerator.h \
    server/usb-host/usbdevice.h \

SOURCES += \
    client/client.cpp \
    main.cpp \
    server/connection.cpp \
    server/deviceinformationfetcher.cpp \
    server/devicemanager.cpp \
    server/echoservice.cpp \
    server/handshakeservice.cpp \
    server/hostserver.cpp \
    server/logging.cpp \
    server/networkmanagercontrol.cpp \
    server/service.cpp \
    server/usb-host/libusbcontext.cpp \
    server/usb-host/usbconnection.cpp \
    server/usb-host/usbconnectionreader.cpp \
    server/usb-host/usbdevice.cpp \
    server/usb-host/usbdeviceenumerator.cpp \

INCLUDEPATH += $$PWD/../

unix {
    LIBS += -L$$OUT_PWD/../libqdb -lqdb
    QMAKE_RPATHDIR += ../libqdb
    target.path = $$[QT_INSTALL_BINS]
    INSTALLS += target
}

win32 {
    CONFIG(debug, debug|release) {
        LIBQDBDIR = $$OUT_PWD/../libqdb/debug
    } else {
        LIBQDBDIR = $$OUT_PWD/../libqdb/release
    }

    LIBS += -L$$LIBQDBDIR -lqdb

    SOURCES += \
        $$PWD/../libqdb/protocol/qdbmessage.cpp \
        $$PWD/../libqdb/protocol/qdbtransport.cpp \
        $$PWD/../libqdb/stream.cpp \
        $$PWD/../libqdb/streampacket.cpp \
        $$PWD/../libqdb/abstractconnection.cpp \
        $$PWD/../libqdb/interruptsignalhandler.cpp

    HEADERS += \
        $$PWD/../libqdb/protocol/qdbmessage.h \
        $$PWD/../libqdb/protocol/qdbtransport.h \
        $$PWD/../libqdb/stream.h \
        $$PWD/../libqdb/streampacket.h \
        $$PWD/../libqdb/abstractconnection.h \
        $$PWD/../libqdb/interruptsignalhandler.h
}
