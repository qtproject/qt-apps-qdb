TEMPLATE = subdirs

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
