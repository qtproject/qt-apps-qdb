qt_internal_add_test(tst_subnet
    SOURCES
        ../../qdb/server/subnet.cpp ../../qdb/server/subnet.h
        tst_subnet.cpp
    INCLUDE_DIRECTORIES
        ..
    PUBLIC_LIBRARIES
        Qt::Network
)
