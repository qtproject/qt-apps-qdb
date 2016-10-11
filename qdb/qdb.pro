QT -= gui
QT += dbus network

CONFIG += c++11

TARGET = qdb
win32: CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    client/client.h \
    server/connection.h \
    server/deviceinformationfetcher.h \
    server/echoservice.h \
    server/handshakeservice.h \
    server/hostserver.h \
    server/networkmanagercontrol.h \
    server/service.h \

SOURCES += \
    client/client.cpp \
    main.cpp \
    server/connection.cpp \
    server/deviceinformationfetcher.cpp \
    server/echoservice.cpp \
    server/handshakeservice.cpp \
    server/hostserver.cpp \
    server/networkmanagercontrol.cpp \
    server/service.cpp \

INCLUDEPATH += $$PWD/../

unix {
    LIBS = -L$$OUT_PWD/../libqdb -lqdb
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
