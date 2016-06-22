QT -= gui
QT += testlib

CONFIG += c++11
CONFIG += testcase

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_stream.cpp \

INCLUDEPATH += $$PWD/../libqdb

LIBS = -L$$OUT_PWD/../libqdb -lqdb
QMAKE_RPATHDIR += ../libqdb
