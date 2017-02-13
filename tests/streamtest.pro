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
}

win32 {
    CONFIG(debug, debug|release) {
        LIBS += -L$$OUT_PWD/../libqdb/debug
    } else {
        LIBS += -L$$OUT_PWD/../libqdb/release
    }
    LIBS += -lqdb
}
