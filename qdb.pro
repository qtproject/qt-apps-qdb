TEMPLATE = subdirs

SUBDIRS += \
    libqdb \
    client \
    qdbd \
    test

client.depends += libqdb
qdbd.depends += libqdb
test.depends += libqdb
