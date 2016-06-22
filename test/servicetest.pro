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

LIBS = -L$$OUT_PWD/../libqdb -lqdb
QMAKE_RPATHDIR += ../libqdb

