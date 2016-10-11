TEMPLATE = subdirs

load(configure)
!qtCompileTest(libusb10): error("Could not find libusb-1.0, which is mandatory")

SUBDIRS += \
    libqdb \
    qdb \
    tests \

qdb.depends += libqdb
tests.depends += libqdb

unix {
    SUBDIRS += \
        qdbd \

    qdbd.depends += libqdb
}
