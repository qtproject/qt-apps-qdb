unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb-1.0
}
win32 {
    INCLUDEPATH += $$(LIBUSB_PATH)\include\libusb-1.0
    contains(QT_ARCH, x86_64) {
        LIBS += -L$$(LIBUSB_PATH)\MS64\static
    } else {
        LIBS += -L$$(LIBUSB_PATH)\MS32\static
    }
    LIBS += -llibusb-1.0
}
