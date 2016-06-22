QT -= gui
QT += testlib

win32: CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += streamtest.cpp

INCLUDEPATH += $$PWD/../libqdb

LIBS = -L$$OUT_PWD/../libqdb -lqdb
QMAKE_RPATHDIR += ../libqdb

