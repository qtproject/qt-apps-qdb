QT -= gui
QT += testlib

CONFIG += c++11
CONFIG += testcase

CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    ../libqdb/abstractconnection.h \
    ../libqdb/stream.h \
    ../libqdb/streampacket.h \
    ../libqdb/protocol/protocol.h \
    ../libqdb/protocol/qdbmessage.h \
    ../libqdb/protocol/qdbtransport.h \

SOURCES += \
    ../libqdb/abstractconnection.cpp \
    ../libqdb/stream.cpp \
    ../libqdb/streampacket.cpp \
    ../libqdb/protocol/qdbmessage.cpp \
    ../libqdb/protocol/qdbtransport.cpp \
    tst_stream.cpp \

INCLUDEPATH += $$PWD/../
