QT -= gui
QT += testlib

win32: CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include($$PWD/../libusb_setup.pri)

HEADERS += \
    ../qdb/server/usb-host/usbcommon.h \
    ../qdb/server/usb-host/usbconnection.h \
    ../qdb/server/usb-host/usbconnectionreader.h \
    ../qdb/server/usb-host/usbdevice.h \
    ../qdb/server/usb-host/usbdeviceenumerator.h \

SOURCES += \
    ../qdb/server/usb-host/libusbcontext.cpp \
    ../qdb/server/usb-host/usbconnection.cpp \
    ../qdb/server/usb-host/usbconnectionreader.cpp \
    ../qdb/server/usb-host/usbdevice.cpp \
    ../qdb/server/usb-host/usbdeviceenumerator.cpp \
    streamtest.cpp \

INCLUDEPATH += $$PWD/../

unix {
    LIBS += -L$$OUT_PWD/../libqdb -lqdb
    QMAKE_RPATHDIR += ../libqdb
}

win32 {
    HEADERS += \
        ../libqdb/protocol/qdbmessage.h \
        ../libqdb/protocol/qdbtransport.h

    SOURCES += \
        ../libqdb/protocol/qdbmessage.cpp \
        ../libqdb/protocol/qdbtransport.cpp

    CONFIG(debug, debug|release) {
        LIBQDBDIR = $$OUT_PWD/../libqdb/debug
    } else {
        LIBQDBDIR = $$OUT_PWD/../libqdb/release
    }
    LIBS += -L$$LIBQDBDIR -lqdb
}
