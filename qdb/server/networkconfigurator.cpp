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
#include "networkconfigurator.h"

#include "connection.h"
#include "connectionpool.h"
#include "networkconfigurationservice.h"
#include "subnet.h"

#include <QtCore/qloggingcategory.h>

Q_LOGGING_CATEGORY(configuratorC, "qdb.networkconfiguration")

NetworkConfigurator::NetworkConfigurator(ConnectionPool *pool, UsbDevice device)
    : m_connection{pool->connect(device)},
      m_device(device) // uniform initialization with {} fails in MSVC 2013 with error C2797
{

}

void NetworkConfigurator::configure()
{
    if (!m_connection || m_connection->state() == ConnectionState::Disconnected) {
        qCWarning(configuratorC) << "Could not configure device" << m_device.serial
                                 << "due to no connection";
        emit configured(m_device, false);
        return;
    }

    SubnetReservation reservation = findUnusedSubnet();
    if (!reservation) {
        qCCritical(configuratorC) << "Could not find a free subnet to use for the network of device"
                                  << m_device.serial;
        emit configured(m_device, false);
        return;
    }

    m_device.reservation = reservation;
    const auto subnet = reservation->subnet();
    const auto subnetString
            = QString{"%1/%2"}.arg(subnet.address.toString()).arg(subnet.prefixLength);

    qCDebug(configuratorC) << "Using subnet" << subnetString << "for" << m_device.serial;

    auto *service = new NetworkConfigurationService{m_connection.get()};

    connect(this, &NetworkConfigurator::configured,
            service, &QObject::deleteLater);
    connect(service, &NetworkConfigurationService::response,
            this, &NetworkConfigurator::handleResponse);
    connect(service, &NetworkConfigurationService::alreadySetResponse,
            this, &NetworkConfigurator::handleAlreadySetResponse);
    connect(service, &Service::initialized, [=]() {
        service->configure(subnetString);
    });

    service->initialize();
}

void NetworkConfigurator::handleAlreadySetResponse(QString subnet)
{
    const QStringList parts = subnet.split(QLatin1Char{'/'});
    if (parts.size() != 2) {
        qCCritical(configuratorC) << "Invalid already set subnet from device" << m_device.serial
                                  << ":" << subnet;
        emit configured(m_device, false);
        return;
    }

    const QHostAddress address{parts[0]};
    const int prefixLength = parts[1].toInt();
    if (address.isNull() || prefixLength < 1 || prefixLength > 32) {
        qCCritical(configuratorC) << "Invalid already set subnet from device" << m_device.serial
                                  << ":" << subnet;
        emit configured(m_device, false);
        return;
    }

    Subnet subnetStruct{address, prefixLength};
    SubnetReservation reservation = SubnetPool::instance()->reserve(subnetStruct);
    if (!reservation) {
        qCWarning(configuratorC) << "Could not reserve already set subnet" << subnet
                                 << "for device" << m_device.serial;
        emit configured(m_device, false);
        return;
    }

    qCDebug(configuratorC) << "Reused already set configuration" << subnet << "for device"
                           << m_device.serial;
    m_device.reservation = reservation;
    emit configured(m_device, true);
}

void NetworkConfigurator::handleResponse(ConfigurationResult result)
{
    emit configured(m_device, result == ConfigurationResult::Success);
}
