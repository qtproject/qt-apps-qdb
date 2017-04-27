/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Debug Bridge.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "../subnet.h"
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
    SubnetReservation reservation;
};

#endif // USBDEVICE_H
