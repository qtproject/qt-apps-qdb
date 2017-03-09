load(configure)
!daemon_only:!qtCompileTest(libusb10): error("Could not find libusb-1.0, which is mandatory for host parts of QDB")

load(qt_parts)

CONFIG -= qt_example_installs

SUBDIRS += libqdb
sub_tests.depends += libqdb

!daemon_only {
    SUBDIRS += qdb

    qdb.depends += libqdb
    # Tests share files with qdb and parallel compilation in that case may lead to error C1083
    # with MSVC. Avoid that by building tests after the main project.
    sub_tests.depends += qdb
}

unix {
    SUBDIRS += qdbd

    qdbd.depends += libqdb
}
