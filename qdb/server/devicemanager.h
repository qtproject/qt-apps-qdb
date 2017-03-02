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
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "connectionpool.h"
#include "deviceinformationfetcher.h"
#include "usb-host/usbdeviceenumerator.h"

#include <QtCore/qobject.h>
#include <QtCore/qqueue.h>

struct DeviceInformation
{
    QString serial;
    QString hostMac;
    QString ipAddress;
    UsbAddress usbAddress;
};

class DeviceManager : public QObject
{
    Q_OBJECT
public:
    explicit DeviceManager(QObject *parent = nullptr);

    std::vector<DeviceInformation> listDevices();
    void start();

signals:
    void newDeviceInfo(DeviceInformation info);
    void disconnectedDevice(QString serial);

private slots:
    void handleDeviceConfigured(UsbDevice device, bool success);
    void handleDeviceInformation(UsbDevice device, DeviceInformationFetcher::Info info);
    void handlePluggedInDevice(UsbDevice device);
    void handleUnpluggedDevice(UsbAddress address);

private:
    void configureDevice(UsbDevice device);
    void fetchDeviceInformation(UsbDevice device);
    void fetchIncomplete();

    UsbDeviceEnumerator m_deviceEnumerator;
    QQueue<UsbDevice> m_incompleteDevices;
    std::vector<DeviceInformation> m_deviceInfos;
    ConnectionPool m_pool;
};

#endif // DEVICEMANAGER_H
