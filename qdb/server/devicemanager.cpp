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
#include "devicemanager.h"

#include "networkconfigurator.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>
#include <QtDBus/QDBusObjectPath>

Q_LOGGING_CATEGORY(devicesC, "qdb.devices");

DeviceManager::DeviceManager(QObject *parent)
    : QObject{parent},
      m_deviceEnumerator{},
      m_incompleteDevices{},
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

void DeviceManager::handleDeviceConfigured(UsbDevice device, bool success)
{
    if (success) {
        qCDebug(devicesC) << "Configured device" << device.serial << "successfully";
        fetchDeviceInformation(device);
    } else {
        qCWarning(devicesC) << "Failed to configure device" << device.serial;
        // Discard the device
    }
}

void DeviceManager::handleDeviceInformation(UsbDevice device, DeviceInformationFetcher::Info info)
{
    if (info.hostMac.isEmpty()) {
        qCWarning(devicesC) << "Could not fetch device information from" << device.serial;
        return; // Discard the device
    }

    if (info.ipAddress.isEmpty()) {
        qCDebug(devicesC) << "Incomplete information received for" << info.serial;
        m_incompleteDevices.enqueue(device);
        QTimer::singleShot(1000, this, &DeviceManager::fetchIncomplete);
    } else {
        qCDebug(devicesC) << "Complete info received for" << info.serial;
    }

    auto iter = std::find_if(m_deviceInfos.begin(), m_deviceInfos.end(),
                             [&](const DeviceInformation &oldInfo) {
                                 return info.serial == oldInfo.serial;
                             });
    if (iter == m_deviceInfos.end()) {
        qCDebug(devicesC) << "Added new info for" << info.serial;
        DeviceInformation newInfo{info.serial, info.hostMac, info.ipAddress, device.address,
                                  device.reservation};
        m_deviceInfos.push_back(newInfo);
        emit newDeviceInfo(newInfo);
    } else if (iter->hostMac != info.hostMac || iter->ipAddress != info.ipAddress) {
        // Serial is the same, since we found iter based on it
        iter->hostMac = info.hostMac;
        iter->ipAddress = info.ipAddress;
        qCDebug(devicesC) << "Replaced old info for" << info.serial;
        emit newDeviceInfo(*iter);
    }
}

void DeviceManager::handlePluggedInDevice(UsbDevice device)
{
    qCDebug(devicesC) << "Device" << device.serial << "plugged in at" << device.address.busNumber << ":" << device.address.deviceAddress;
    configureDevice(device);
}

void DeviceManager::handleUnpluggedDevice(UsbAddress address)
{
    qCDebug(devicesC) << "Device unplugged from" << address.busNumber << ":" << address.deviceAddress;

    auto deviceAddressMatches = [&](const UsbDevice &device) {
        return device.address == address;
    };

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

void DeviceManager::configureDevice(UsbDevice device)
{
    qCDebug(devicesC) << "Configuring device" << device.serial;
    auto *configurator = new NetworkConfigurator{&m_pool, device};
    connect(configurator, &NetworkConfigurator::configured, configurator, &QObject::deleteLater);
    connect(configurator, &NetworkConfigurator::configured,
            this, &DeviceManager::handleDeviceConfigured);

    configurator->configure();
}

void DeviceManager::fetchDeviceInformation(UsbDevice device)
{
    qCDebug(devicesC) << "Fetching device information for" << device.serial;
    auto *fetcher = new DeviceInformationFetcher{m_pool.connect(device), device};
    connect(fetcher, &DeviceInformationFetcher::fetched, fetcher, &QObject::deleteLater);
    connect(fetcher, &DeviceInformationFetcher::fetched, this, &DeviceManager::handleDeviceInformation);

    fetcher->fetch();
}

void DeviceManager::fetchIncomplete()
{
    if (m_incompleteDevices.empty())
        return;

    fetchDeviceInformation(m_incompleteDevices.dequeue());
}



