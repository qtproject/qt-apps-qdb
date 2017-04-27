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
#include "usbdevice.h"

#include <libusb.h>

#include <QtCore/qdebug.h>

LibUsbDevice::LibUsbDevice()
    : LibUsbDevice{nullptr}
{

}

LibUsbDevice::LibUsbDevice(libusb_device *device)
    : m_device{device}
{
    if (m_device)
        libusb_ref_device(m_device);
}

LibUsbDevice::LibUsbDevice(const LibUsbDevice &rhs)
{
    m_device = rhs.m_device;
    if (m_device)
        libusb_ref_device(m_device);
}

LibUsbDevice::LibUsbDevice(LibUsbDevice &&rhs)
{
    m_device = rhs.m_device;
    rhs.m_device = nullptr;
}

LibUsbDevice::~LibUsbDevice()
{
    if (m_device)
        libusb_unref_device(m_device);
}

LibUsbDevice &LibUsbDevice::operator=(const LibUsbDevice &rhs)
{
    if (this != &rhs) {
        m_device = rhs.m_device;
        if (m_device)
            libusb_ref_device(m_device);
    }
    return *this;
}

libusb_device *LibUsbDevice::pointer()
{
    return m_device;
}

bool operator==(const UsbAddress &lhs, const UsbAddress &rhs)
{
    return lhs.busNumber == rhs.busNumber && lhs.deviceAddress == rhs.deviceAddress;
}

bool operator<(const UsbAddress &lhs, const UsbAddress &rhs)
{
    return std::tie(lhs.busNumber, lhs.deviceAddress) < std::tie(rhs.busNumber, rhs.deviceAddress);
}
