QT -= gui
QT += testlib

win32: CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += streamtest.cpp

INCLUDEPATH += $$PWD/../libqdb

unix {
LIBS = -L$$OUT_PWD/../libqdb -lqdb
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
LIBS = -L$$LIBQDBDIR -lqdb
}

