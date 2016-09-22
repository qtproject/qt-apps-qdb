TEMPLATE = subdirs

load(configure)
!qtCompileTest(libusb10): error("Could not find libusb-1.0, which is mandatory")

SUBDIRS += \
    libqdb \
    client \
    test

client.depends += libqdb
test.depends += libqdb

unix {
    SUBDIRS += \
        qdbd \

    qdbd.depends += libqdb
}
