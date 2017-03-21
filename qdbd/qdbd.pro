QT += core network
QT -= gui

CONFIG += c++11

TARGET = qdbd
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

include($$PWD/../version.pri)

SOURCES += \
    configuration.cpp \
    createexecutor.cpp \
    echoexecutor.cpp \
    executor.cpp \
    handshakeexecutor.cpp \
    main.cpp \
    networkconfiguration.cpp \
    networkconfigurationexecutor.cpp \
    server.cpp \
    usb-gadget/usbgadget.cpp \
    usb-gadget/usbgadgetcontrol.cpp \
    usb-gadget/usbgadgetreader.cpp \
    usb-gadget/usbgadgetwriter.cpp \

HEADERS += \
    configuration.h \
    createexecutor.h \
    echoexecutor.h \
    executor.h \
    handshakeexecutor.h \
    networkconfigurationexecutor.h \
    networkconfiguration.h \
    server.h \
    usb-gadget/usbgadget.h \
    usb-gadget/usbgadgetcontrol.h \
    usb-gadget/usbgadgetreader.h \
    usb-gadget/usbgadgetwriter.h \

INCLUDEPATH += $$PWD/../

LIBS += -L$$OUT_PWD/../libqdb -lqdb

unix {
    target.path = $$[QT_INSTALL_BINS]
    INSTALLS += target
}
