QT -= gui
QT += testlib

CONFIG += c++11
CONFIG += testcase

TARGET = tst_qdbmessage
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += \
    tst_qdbmessage.cpp \
    ../libqdb/protocol/qdbmessage.cpp
