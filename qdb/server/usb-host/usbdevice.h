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
#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "usbcommon.h"

#include <QtCore/qstring.h>

#include <cstdint>
#include <memory>

struct libusb_device;

// Class that wraps the libusb reference count incrementing and decrementing for
// a libusb_device
class LibUsbDevice {
public:
    LibUsbDevice();
    LibUsbDevice(libusb_device *device);
    LibUsbDevice(const LibUsbDevice &rhs);
    LibUsbDevice(LibUsbDevice&& rhs);
    ~LibUsbDevice();

    LibUsbDevice &operator=(const LibUsbDevice &rhs);

    libusb_device *pointer();

private:
    libusb_device *m_device;
};

struct UsbAddress
{
    uint8_t busNumber;
    uint8_t deviceAddress;
};

bool operator<(const UsbAddress &lhs, const UsbAddress &rhs);
bool operator==(const UsbAddress &lhs, const UsbAddress &rhs);

struct UsbDevice
{
    QString serial;
    UsbAddress address;
    LibUsbDevice usbDevice;
    UsbInterfaceInfo interfaceInfo;
};

#endif // USBDEVICE_H
