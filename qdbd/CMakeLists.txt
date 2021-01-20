qt_internal_add_executable(qdbd
    SOURCES
        configuration.cpp configuration.h
        createexecutor.cpp createexecutor.h
        echoexecutor.cpp echoexecutor.h
        executor.cpp executor.h
        handshakeexecutor.cpp handshakeexecutor.h
        main.cpp
        networkconfiguration.cpp networkconfiguration.h
        networkconfigurationexecutor.cpp networkconfigurationexecutor.h
        server.cpp server.h
        usb-gadget/usbgadget.cpp usb-gadget/usbgadget.h
        usb-gadget/usbgadgetcontrol.cpp usb-gadget/usbgadgetcontrol.h
        usb-gadget/usbgadgetreader.cpp usb-gadget/usbgadgetreader.h
        usb-gadget/usbgadgetwriter.cpp usb-gadget/usbgadgetwriter.h
    DEFINES
        QDB_VERSION="${QDB_VERSION}"
    INCLUDE_DIRECTORIES
        ${PROJECT_SOURCE_DIR}
    PUBLIC_LIBRARIES
        Qt::Network
        libqdb
)
