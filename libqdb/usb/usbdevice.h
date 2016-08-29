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

class LibUsbDevice {
public:
    LibUsbDevice(std::shared_ptr<libusb_device *> devices, int index)
        : m_deviceList{devices},
          m_index{index}

    {
    }

    libusb_device *pointer()
    {
        return m_deviceList.get()[m_index];
    }

private:
    std::shared_ptr<libusb_device *> m_deviceList;
    int m_index;

};

struct UsbDevice
{
    QString serial;
    LibUsbDevice usbDevice;
    UsbInterfaceInfo interfaceInfo;
};

#endif // USBDEVICE_H
