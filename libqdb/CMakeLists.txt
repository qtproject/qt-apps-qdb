qt_internal_add_cmake_library(libqdb
    INSTALL_DIRECTORY "$$[QT_INSTALL_LIBS]"
    STATIC
    SOURCES
        abstractconnection.cpp abstractconnection.h
        interruptsignalhandler.h
        protocol/protocol.h
        protocol/qdbmessage.cpp protocol/qdbmessage.h
        protocol/qdbtransport.cpp protocol/qdbtransport.h
        protocol/services.h
        stream.cpp stream.h
        streampacket.cpp streampacket.h
    INCLUDE_DIRECTORIES
        ..
    PUBLIC_LIBRARIES
        Qt::Core
)

qt_internal_extend_target(libqdb CONDITION WIN32
    SOURCES
        interruptsignalhandler_win.cpp
)

qt_internal_extend_target(libqdb CONDITION WIN64
    SOURCES
        interruptsignalhandler_win.cpp
)

qt_internal_extend_target(libqdb CONDITION UNIX
    SOURCES
        interruptsignalhandler_unix.cpp
)
