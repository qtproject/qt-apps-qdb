load(configure)
!daemon_only:!qtCompileTest(libusb10): error("Could not find libusb-1.0, which is mandatory for host parts of QDB")

load(qt_parts)

SUBDIRS += libqdb
sub_tests.depends += libqdb

!daemon_only {
    SUBDIRS += qdb

    qdb.depends += libqdb
}

unix {
    SUBDIRS += qdbd

    qdbd.depends += libqdb
}
