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
#ifndef USBDEVICEENUMERATOR_H
#define USBDEVICEENUMERATOR_H

#include "usbdevice.h"

#include <QtCore/qobject.h>
#include <QtCore/qtimer.h>

#include <vector>

class UsbDeviceEnumerator : public QObject
{
    Q_OBJECT
public:
    UsbDeviceEnumerator();
    ~UsbDeviceEnumerator();

    std::vector<UsbDevice> listUsbDevices();
    void startMonitoring();
    void stopMonitoring();

signals:
    void devicePluggedIn(UsbDevice device);
    void deviceUnplugged(UsbAddress address);

private:
    void pollQdbDevices();

    QTimer m_pollTimer;
    std::vector<UsbDevice> m_qdbDevices;
};

#endif // USBDEVICEENUMERATOR_H
