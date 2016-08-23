QT -= gui
QT += dbus

CONFIG += c++11

TARGET = qdb
win32: CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    connection.h \
    echoservice.h \
    filepullservice.h \
    filepushservice.h \
    networkmanagercontrol.h \
    processservice.h \
    service.h

SOURCES += \
    connection.cpp \
    echoservice.cpp \
    filepullservice.cpp \
    filepushservice.cpp \
    main.cpp \
    networkmanagercontrol.cpp \
    processservice.cpp \
    service.cpp

INCLUDEPATH += $$PWD/../libqdb

unix {
    LIBS = -L$$OUT_PWD/../libqdb -lqdb
    QMAKE_RPATHDIR += ../libqdb
    target.path = /usr/bin
    INSTALLS += target
}

win32 {

CONFIG(debug, debug|release) {
    LIBQDBDIR = $$OUT_PWD/../libqdb/debug
} else {
    LIBQDBDIR = $$OUT_PWD/../libqdb/release
}

    LIBS = -L$$LIBQDBDIR -lqdb

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
