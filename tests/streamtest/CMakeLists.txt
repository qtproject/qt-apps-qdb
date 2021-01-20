qt_internal_add_executable(streamtest
    SOURCES
        ../../qdb/server/usb-host/libusbcontext.cpp
        ../../qdb/server/usb-host/usbcommon.h
        ../../qdb/server/usb-host/usbconnection.cpp ../../qdb/server/usb-host/usbconnection.h
        ../../qdb/server/usb-host/usbconnectionreader.cpp ../../qdb/server/usb-host/usbconnectionreader.h
        ../../qdb/server/usb-host/usbdevice.cpp ../../qdb/server/usb-host/usbdevice.h
        ../../qdb/server/usb-host/usbdeviceenumerator.cpp ../../qdb/server/usb-host/usbdeviceenumerator.h
        streamtest.cpp
    INCLUDE_DIRECTORIES
        ${LIBUSB_INCLUDE_DIR}
        ../../
    PUBLIC_LIBRARIES
        Qt::Test
        libqdb
        libUsb::libUsb
)
