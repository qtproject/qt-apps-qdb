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
#include "connectionpool.h"

#include "connection.h"
#include "libqdb/protocol/qdbtransport.h"
#include "usb-host/usbconnection.h"
#include "usb-host/usbdevice.h"

#include <QtCore/qloggingcategory.h>

Q_LOGGING_CATEGORY(connectionPoolC, "qdb.connectionpool")

std::shared_ptr<Connection> ConnectionPool::connect(const UsbDevice &device)
{
    if (m_connections.contains(device.serial)) {
        std::shared_ptr<Connection> existingConnection = m_connections[device.serial].lock();
        if (existingConnection) {
            qDebug(connectionPoolC) << "Using existing connection to" << device.serial;
            return existingConnection;
        } else {
            qDebug(connectionPoolC) << "Existing connection to" << device.serial << "expired, creating new one";
            m_connections.remove(device.serial);
        }
    }

    auto connection = std::make_shared<Connection>(new QdbTransport{new UsbConnection{device}});
    m_connections[device.serial] = std::weak_ptr<Connection>(connection);

    if (!connection->initialize()) {
        qCCritical(connectionPoolC) << "Could not initialize connection to" << device.serial;
        return std::shared_ptr<Connection>(nullptr);
    }

    connection->connect();
    qCDebug(connectionPoolC) << "Initialized connection to" << device.serial;

    return connection;
}
