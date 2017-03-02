QT -= gui
QT += network testlib

CONFIG += c++11
CONFIG += testcase

TARGET = tst_subnet
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/../

HEADERS += \
    ../qdb/server/subnet.h \

SOURCES += \
    ../qdb/server/subnet.cpp \
    tst_subnet.cpp \
