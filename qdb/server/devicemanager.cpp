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
#include "devicemanager.h"

#include "networkmanagercontrol.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>
#include <QtDBus/QDBusObjectPath>

Q_LOGGING_CATEGORY(devicesC, "qdb.devices");

DeviceManager::DeviceManager(QObject *parent)
    : QObject{parent},
      m_deviceEnumerator{},
      m_newDevices{},
      m_incompleteDevices{},
      m_fetchingDevice{},
      m_fetching{false},
      m_deviceInfos{}
{

}

std::vector<DeviceInformation> DeviceManager::listDevices()
{
    return m_deviceInfos;
}

void DeviceManager::start()
{
    connect(&m_deviceEnumerator, &UsbDeviceEnumerator::devicePluggedIn, this, &DeviceManager::handlePluggedInDevice);
    connect(&m_deviceEnumerator, &UsbDeviceEnumerator::deviceUnplugged, this, &DeviceManager::handleUnpluggedDevice);
    m_deviceEnumerator.startMonitoring();
}

void DeviceManager::handleDeviceInformation(DeviceInformation deviceInfo)
{
    m_fetching = false;
    if (deviceInfo.hostMac.isEmpty()) {
        qCWarning(devicesC) << "Could not fetch device information from" << m_fetchingDevice.serial;
        return; // Discard the device
    }

    qCDebug(devicesC) << "Configuring network for" << deviceInfo.serial;
    configureUsbNetwork(deviceInfo.serial, deviceInfo.hostMac);

    if (deviceInfo.ipAddress.isEmpty()) {
        qCDebug(devicesC) << "Incomplete information received for" << deviceInfo.serial;
        m_incompleteDevices.enqueue(m_fetchingDevice);
        QTimer::singleShot(1000, this, &DeviceManager::fetchIncomplete);
    } else {
        qCDebug(devicesC) << "Complete info received for" << deviceInfo.serial;
    }

    auto iter = std::find_if(m_deviceInfos.begin(), m_deviceInfos.end(),
                             [&](const DeviceInformation &oldInfo) {
                                 return deviceInfo.serial == oldInfo.serial;
                             });
    if (iter == m_deviceInfos.end()) {
        qCDebug(devicesC) << "Added new info for" << deviceInfo.serial;
        m_deviceInfos.push_back(deviceInfo);
        emit newDeviceInfo(deviceInfo);
    } else if (*iter != deviceInfo) {
        qCDebug(devicesC) << "Replaced old info for" << deviceInfo.serial;
        *iter = deviceInfo;
        emit newDeviceInfo(deviceInfo);
    }

    if (!m_newDevices.isEmpty())
        fetchDeviceInformation(m_newDevices.dequeue());
}

void DeviceManager::handlePluggedInDevice(UsbDevice device)
{
    qCDebug(devicesC) << "Device" << device.serial << "plugged in at" << device.address.busNumber << ":" << device.address.deviceAddress;
    if (m_fetching)
        m_newDevices.enqueue(device);
    else
        fetchDeviceInformation(device);
}

void DeviceManager::handleUnpluggedDevice(UsbAddress address)
{
    qCDebug(devicesC) << "Device unplugged from" << address.busNumber << ":" << address.deviceAddress;

    auto deviceAddressMatches = [&](const UsbDevice &device) {
        return device.address == address;
    };

    const auto newIter = std::find_if(m_newDevices.begin(), m_newDevices.end(), deviceAddressMatches);
    if (newIter != m_newDevices.end()) {
        m_newDevices.erase(newIter);
        return; // New devices can't yet be in the other lists.
    }

    const auto incompleteIter = std::find_if(m_incompleteDevices.begin(), m_incompleteDevices.end(),
                                             deviceAddressMatches);
    if (incompleteIter != m_incompleteDevices.end()) {
        m_incompleteDevices.erase(incompleteIter);
        // Incomplete devices already have information stored about them, so continue.
    }

    const auto infoIter = std::find_if(m_deviceInfos.begin(), m_deviceInfos.end(),
                                       [&](const DeviceInformation &info) {
                                           return info.usbAddress == address;
                                       });
    if (infoIter != m_deviceInfos.end()) {
        emit disconnectedDevice(infoIter->serial);
        m_deviceInfos.erase(infoIter);
    }
}

void DeviceManager::fetchDeviceInformation(UsbDevice device)
{
    qCDebug(devicesC) << "Fetching device information for" << device.serial;
    Q_ASSERT(!m_fetching);
    m_fetching = true;
    m_fetchingDevice = device;
    auto *fetcher = new DeviceInformationFetcher{m_pool.connect(device), device};
    connect(fetcher, &DeviceInformationFetcher::fetched, fetcher, &QObject::deleteLater);
    connect(fetcher, &DeviceInformationFetcher::fetched, this, &DeviceManager::handleDeviceInformation);

    fetcher->fetch();
}

void DeviceManager::fetchIncomplete()
{
    if (m_incompleteDevices.empty())
        return;

    if (m_fetching) {
        qCDebug(devicesC) << "Delaying fetching incomplete information due to fetch in progress";
        QTimer::singleShot(500, this, &DeviceManager::fetchIncomplete);
        return;
    }

    fetchDeviceInformation(m_incompleteDevices.dequeue());
}



