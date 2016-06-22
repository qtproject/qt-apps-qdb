QT += core
QT -= gui

CONFIG += c++11

TARGET = qdbd
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    createexecutor.cpp \
    echoexecutor.cpp \
    executor.cpp \
    filepullexecutor.cpp \
    filepushexecutor.cpp \
    main.cpp \
    processexecutor.cpp \
    server.cpp \
    usb-gadget/usbgadget.cpp \
    usb-gadget/usbgadgetreader.cpp \
    usb-gadget/usbgadgetwriter.cpp \


HEADERS += \
    createexecutor.h \
    echoexecutor.h \
    executor.h \
    filepullexecutor.h \
    filepushexecutor.h \
    processexecutor.h \
    server.h \
    usb-gadget/usbgadget.h \
    usb-gadget/usbgadgetreader.h \
    usb-gadget/usbgadgetwriter.h \

INCLUDEPATH += $$PWD/../libqdb

LIBS = -L$$OUT_PWD/../libqdb -lqdb
QMAKE_RPATHDIR += ../libqdb

unix {
    target.path = /usr/bin
    INSTALLS += target
}
