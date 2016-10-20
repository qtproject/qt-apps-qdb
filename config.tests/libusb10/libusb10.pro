unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb-1.0
}

win32 {
    LIBS += -llibusb-1.0
}

SOURCES = libusb10.cpp
