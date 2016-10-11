/******************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Debug Bridge.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/
#include "devicemanagement.h"
#include "scopeguard.h"
#include "usbcommon.h"
#include "usbconnection.h"
#include "qdbconstants.h"

#include <QtCore/qdebug.h>

#include <libusb.h>

bool isQdbInterface(const libusb_interface &interface)
{
    const libusb_interface_descriptor *descriptor = &interface.altsetting[0];
    return descriptor->bInterfaceClass == qdbUsbClassId && descriptor->bInterfaceSubClass == qdbUsbSubclassId;
}

std::pair<bool, UsbInterfaceInfo> findQdbInterface(libusb_device *device)
{
    libusb_config_descriptor *config;
    const int ret = libusb_get_active_config_descriptor(device, &config);
    if (ret) {
        qCritical() << "Could not get config descriptor" << libusb_error_name(ret);
        return std::make_pair(false, UsbInterfaceInfo{});
    }
    ScopeGuard configGuard = [&]() {
        libusb_free_config_descriptor(config);
    };

    const auto last = config->interface + config->bNumInterfaces;
    const auto qdbInterface = std::find_if(config->interface, last, isQdbInterface);
    if (qdbInterface == last) {
        return std::make_pair(false, UsbInterfaceInfo{});;
    }

    const int inEndpointIndex = 1;
    const int outEndpointIndex = 0;

    const libusb_interface_descriptor *interface = &qdbInterface->altsetting[0];
    const auto interfaceNumber = interface->bInterfaceNumber;
    const auto inAddress = interface->endpoint[inEndpointIndex].bEndpointAddress;
    const auto outAddress = interface->endpoint[outEndpointIndex].bEndpointAddress;
    const UsbInterfaceInfo info{interfaceNumber, inAddress, outAddress};
    return std::make_pair(true, info);
}

QString getSerialNumber(libusb_device *device, libusb_device_handle *handle)
{
    QString serial{"???"};

    libusb_device_descriptor desc;
    int ret = libusb_get_device_descriptor(device, &desc);
    if (ret) {
        qCritical() << "Could not get device descriptor" << libusb_error_name(ret);
        return serial;
    }
    auto serialIndex = desc.iSerialNumber;

    const uint16_t englishUsLangId = 0x409;
    const int bufferSize = 255; // USB string descriptor size field is a single byte
    unsigned char buffer[bufferSize];
    int length = libusb_get_string_descriptor(handle, serialIndex, englishUsLangId, buffer, bufferSize);
    if (length <= 0) {
        qWarning() << "Could not get string descriptor of serial number:" << libusb_error_name(length);
        return serial;
    }
    // length is the length in bytes and UTF-16 characters consist of two bytes
    Q_ASSERT(length % 2 == 0);
    serial = QString::fromUtf16(reinterpret_cast<unsigned short*>(buffer), length / 2);
    return serial;
}

std::vector<UsbDevice> listUsbDevices()
{
    if (!libUsbContext()) {
        qDebug() << "Not initialized libusb in DeviceManager";
        return std::vector<UsbDevice>{};
    }

    libusb_device **devicesParam;
    ssize_t deviceCount = libusb_get_device_list(libUsbContext(), &devicesParam);
    std::shared_ptr<libusb_device *> devices{devicesParam,
                                              [](libusb_device **pointer) {
                                                  libusb_free_device_list(pointer, 1);
                                              }};

    if (deviceCount < 0) {
        qCritical() << "USB devices could not be listed:" << libusb_error_name(deviceCount);
        return std::vector<UsbDevice>{};
    }

    std::vector<UsbDevice> qdbDevices;
    for (int i = 0; i < deviceCount; ++i) {
        libusb_device *device = devices.get()[i];

        const auto interfaceResult = findQdbInterface(device);
        if (!interfaceResult.first) {
            // No QDB interface found, not a QDB device
            continue;
        }

        libusb_device_handle *handle;
        int ret = libusb_open(device, &handle);
        if (ret) {
            qDebug() << "Could not open USB device for checking serial number:" << libusb_error_name(ret);
            continue;
        }
        ScopeGuard deviceGuard = [=]() {
            libusb_close(handle);
        };
        const auto serial = getSerialNumber(device, handle);

        const UsbDevice usbDevice{serial, LibUsbDevice{devices, i}, interfaceResult.second};
        qdbDevices.push_back(usbDevice);
    }
    return qdbDevices;
}
