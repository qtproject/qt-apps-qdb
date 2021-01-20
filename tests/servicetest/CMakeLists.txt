qt_internal_add_executable(servicetest
    SOURCES
        ../../qdb/server/connection.cpp ../../qdb/server/connection.h
        ../../qdb/server/echoservice.cpp ../../qdb/server/echoservice.h
        ../../qdb/server/service.cpp ../../qdb/server/service.h
        ../../qdb/server/usb-host/libusbcontext.cpp
        ../../qdb/server/usb-host/usbcommon.h
        ../../qdb/server/usb-host/usbconnection.cpp ../../qdb/server/usb-host/usbconnection.h
        ../../qdb/server/usb-host/usbconnectionreader.cpp ../../qdb/server/usb-host/usbconnectionreader.h
        ../../qdb/server/usb-host/usbdevice.cpp ../../qdb/server/usb-host/usbdevice.h
        ../../qdb/server/usb-host/usbdeviceenumerator.cpp ../../qdb/server/usb-host/usbdeviceenumerator.h
        servicetest.cpp
    INCLUDE_DIRECTORIES
        ${LIBUSB_INCLUDE_DIR}
        ../../
    PUBLIC_LIBRARIES
        Qt::Test
        libUsb::libUsb
        libqdb
)
