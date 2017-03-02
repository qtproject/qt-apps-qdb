TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
    tst_qdbmessage.pro \
    tst_stream.pro \

config_libusb10 {
    SUBDIRS += \
        servicetest.pro \
        streamtest.pro
}
