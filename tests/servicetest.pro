QT -= gui
QT += testlib

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
    ../qdb/server/connection.h \
    ../qdb/server/echoservice.h \
    ../qdb/server/service.h \
    ../qdb/server/usb-host/usbcommon.h \
    ../qdb/server/usb-host/usbconnection.h \
    ../qdb/server/usb-host/usbconnectionreader.h \
    ../qdb/server/usb-host/usbdevice.h \
    ../qdb/server/usb-host/usbdeviceenumerator.h \

SOURCES += \
    ../qdb/server/connection.cpp \
    ../qdb/server/echoservice.cpp \
    ../qdb/server/service.cpp \
    ../qdb/server/usb-host/libusbcontext.cpp \
    ../qdb/server/usb-host/usbconnection.cpp \
    ../qdb/server/usb-host/usbconnectionreader.cpp \
    ../qdb/server/usb-host/usbdevice.cpp \
    ../qdb/server/usb-host/usbdeviceenumerator.cpp \
    servicetest.cpp \

INCLUDEPATH += $$PWD/../

unix {
    LIBS = -L$$OUT_PWD/../libqdb -lqdb
    QMAKE_RPATHDIR += ../libqdb
}

win32 {
    HEADERS += \
        ../libqdb/protocol/protocol.h \
        ../libqdb/protocol/qdbmessage.h \
        ../libqdb/protocol/qdbtransport.h \
        ../libqdb/stream.h \
        ../libqdb/abstractconnection.h \
        ../libqdb/streampacket.h

    SOURCES += \
        ../libqdb/protocol/qdbmessage.cpp \
        ../libqdb/protocol/qdbtransport.cpp \
        ../libqdb/stream.cpp \
        ../libqdb/abstractconnection.cpp \
        ../libqdb/streampacket.cpp

    CONFIG(debug, debug|release) {
        LIBQDBDIR = $$OUT_PWD/../libqdb/debug
    } else {
        LIBQDBDIR = $$OUT_PWD/../libqdb/release
    }
    LIBS = -L$$LIBQDBDIR -lqdb
}