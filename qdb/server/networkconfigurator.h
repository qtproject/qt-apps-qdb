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
#ifndef NETWORKCONFIGURATOR_H
#define NETWORKCONFIGURATOR_H

#include "usb-host/usbdevice.h"
class Connection;
class ConnectionPool;

#include <QtCore/qobject.h>

#include <memory>
#include <vector>

class NetworkConfigurator : public QObject
{
    Q_OBJECT
public:
    NetworkConfigurator(ConnectionPool *pool, UsbDevice device);

    void configure();

signals:
    void configured(UsbDevice device, bool success);

private slots:
    void handleResponse(bool success);

private:
    std::shared_ptr<Connection> m_connection;
    UsbDevice m_device;
};

#endif // NETWORKCONFIGURATOR_H
