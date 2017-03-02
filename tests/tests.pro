TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
    tst_qdbmessage.pro \
    tst_stream.pro \
    tst_subnet.pro \

config_libusb10 {
    SUBDIRS += \
        servicetest.pro \
        streamtest.pro
}
