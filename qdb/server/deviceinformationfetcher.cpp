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
#include "deviceinformationfetcher.h"

#include "connection.h"
#include "handshakeservice.h"
#include "libqdb/protocol/qdbtransport.h"
#include "usb-host/usbconnection.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

Q_LOGGING_CATEGORY(deviceInfoC, "qdb.devices.info");

bool operator==(const DeviceInformation &lhs, const DeviceInformation &rhs)
{
    return lhs.serial == rhs.serial
        && lhs.hostMac == rhs.hostMac
        && lhs.ipAddress == rhs.ipAddress
        && lhs.usbAddress == rhs.usbAddress;
}

bool operator!=(const DeviceInformation &lhs, const DeviceInformation &rhs)
{
    return !(lhs == rhs);
}

DeviceInformationFetcher::DeviceInformationFetcher(std::shared_ptr<Connection> connection,
                                                   UsbDevice device)
    : m_connection{connection},
      m_deviceAddress(device.address) // uniform initialization with {} fails with GCC 4.9
{

}

void DeviceInformationFetcher::fetch()
{
    if (!m_connection || m_connection->state() == ConnectionState::Disconnected) {
        qCWarning(deviceInfoC) << "Not fetching device information due to no connection";
        emit fetched(DeviceInformation{"", "", "", m_deviceAddress});
        return;
    }

    auto *service = new HandshakeService{m_connection.get()};

    connect(this, &DeviceInformationFetcher::fetched,
            service, &QObject::deleteLater);
    connect(service, &HandshakeService::response,
            this, &DeviceInformationFetcher::handshakeResponse);
    connect(service, &Service::initialized, [=]() {
        service->ask();
    });

    service->initialize();
}

void DeviceInformationFetcher::handshakeResponse(QString serial, QString hostMac, QString ipAddress)
{
    qCDebug(deviceInfoC) << "Fetched device information:";
    qCDebug(deviceInfoC) << "    Device serial:" << serial;
    qCDebug(deviceInfoC) << "    Host-side MAC address:" << hostMac;
    qCDebug(deviceInfoC) << "    Device IP address:" << ipAddress;
    emit fetched(DeviceInformation{serial, hostMac, ipAddress, m_deviceAddress});
}
