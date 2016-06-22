QT -= gui

CONFIG += c++11

TARGET = qdb
win32: CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += \
    connection.h \
    echoservice.h \
    filepullservice.h \
    filepushservice.h \
    processservice.h \
    service.h


SOURCES += \
    connection.cpp \
    echoservice.cpp \
    filepullservice.cpp \
    filepushservice.cpp \
    main.cpp \
    processservice.cpp \
    service.cpp


INCLUDEPATH += $$PWD/../libqdb

LIBS = -L$$OUT_PWD/../libqdb -lqdb
QMAKE_RPATHDIR += ../libqdb

unix {
    target.path = /usr/bin
    INSTALLS += target
}

