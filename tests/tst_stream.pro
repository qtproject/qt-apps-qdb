QT -= gui
QT += testlib

CONFIG += c++11
CONFIG += testcase

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_stream.cpp \

INCLUDEPATH += $$PWD/../

unix {
    LIBS += -L$$OUT_PWD/../libqdb -lqdb
    QMAKE_RPATHDIR += ../libqdb
}

win32 {
    HEADERS += \
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

    LIBS += -L$$LIBQDBDIR -lqdb
}

