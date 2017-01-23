unix {
    CONFIG += link_pkgconfig
    PKGCONFIG += libusb-1.0
}
win32 {
    INCLUDEPATH += $$(LIBUSB_PATH)\include\libusb-1.0
    LIBS += -L$$(LIBUSB_PATH)\MS32\dll -llibusb-1.0
}
