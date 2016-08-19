QT -= gui
QT += testlib

win32: CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    ../client/connection.h \
    ../client/filepullservice.h \
    ../client/filepushservice.h \
    ../client/processservice.h \
    ../client/echoservice.h \
    ../client/service.h

SOURCES += \
    servicetest.cpp \
    ../client/connection.cpp \
    ../client/filepullservice.cpp \
    ../client/filepushservice.cpp \
    ../client/processservice.cpp \
    ../client/echoservice.cpp \
    ../client/service.cpp

INCLUDEPATH += $$PWD/../libqdb

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
