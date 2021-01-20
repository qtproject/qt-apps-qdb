qt_internal_add_test(tst_stream
    SOURCES
        ../../libqdb/abstractconnection.cpp ../../libqdb/abstractconnection.h
        ../../libqdb/protocol/protocol.h
        ../../libqdb/protocol/qdbmessage.cpp ../../libqdb/protocol/qdbmessage.h
        ../../libqdb/protocol/qdbtransport.cpp ../../libqdb/protocol/qdbtransport.h
        ../../libqdb/stream.cpp ../../libqdb/stream.h
        ../../libqdb/streampacket.cpp ../../libqdb/streampacket.h
        tst_stream.cpp
    INCLUDE_DIRECTORIES
        ../../
)
