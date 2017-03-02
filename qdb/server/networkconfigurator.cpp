/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

    const std::pair<Subnet, bool> configuration = findUnusedSubnet();
    if (!configuration.second) {
        qCCritical(configuratorC) << "Could not find a free subnet to use for the network of device"
                                  << m_device.serial;
        emit configured(m_device, false);
        return;
    }

    const auto &subnet = configuration.first;
    const auto subnetString
            = QString{"%1/%2"}.arg(subnet.address.toString()).arg(subnet.prefixLength);

    qCDebug(configuratorC) << "Using subnet" << subnetString << "for" << m_device.serial;

    auto *service = new NetworkConfigurationService{m_connection.get()};

    connect(this, &NetworkConfigurator::configured,
            service, &QObject::deleteLater);
    connect(service, &NetworkConfigurationService::response,
            this, &NetworkConfigurator::handleResponse);
    connect(service, &Service::initialized, [=]() {
        service->configure(subnetString);
    });

    service->initialize();
}

void NetworkConfigurator::handleResponse(bool success)
{
    emit configured(m_device, success);
}
