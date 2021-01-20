qt_internal_add_executable(qdb
    SOURCES
        client/client.cpp client/client.h
        hostmessages.cpp hostmessages.h
        main.cpp
        server/connection.cpp server/connection.h
        server/connectionpool.cpp server/connectionpool.h
        server/deviceinformationfetcher.cpp server/deviceinformationfetcher.h
        server/devicemanager.cpp server/devicemanager.h
        server/echoservice.cpp server/echoservice.h
        server/handshakeservice.cpp server/handshakeservice.h
        server/hostserver.cpp server/hostserver.h
        server/hostservlet.cpp server/hostservlet.h
        server/logging.cpp server/logging.h
        server/networkconfigurationservice.cpp server/networkconfigurationservice.h
        server/networkconfigurator.cpp server/networkconfigurator.h
        server/service.cpp server/service.h
        server/subnet.cpp server/subnet.h
        server/usb-host/libusbcontext.cpp
        server/usb-host/usbcommon.h
        server/usb-host/usbconnection.cpp server/usb-host/usbconnection.h
        server/usb-host/usbconnectionreader.cpp server/usb-host/usbconnectionreader.h
        server/usb-host/usbdevice.cpp server/usb-host/usbdevice.h
        server/usb-host/usbdeviceenumerator.cpp server/usb-host/usbdeviceenumerator.h
    DEFINES
        QDB_VERSION="${QDB_VERSION}"
    INCLUDE_DIRECTORIES
        ${PROJECT_SOURCE_DIR}
    PUBLIC_LIBRARIES
        Qt::DBus
        Qt::Network
        libqdb
        libUsb::libUsb
)
