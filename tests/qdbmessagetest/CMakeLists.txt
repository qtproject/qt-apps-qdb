qt_internal_add_test(tst_qdbmessage
    SOURCES
        ../../libqdb/protocol/protocol.h
        ../../libqdb/protocol/qdbmessage.cpp ../../libqdb/protocol/qdbmessage.h
        tst_qdbmessage.cpp
    INCLUDE_DIRECTORIES
        ..
    PUBLIC_LIBRARIES
        Qt::Test
)
