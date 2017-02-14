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
#ifndef DEVICEINFORMATIONFETCHER_H
#define DEVICEINFORMATIONFETCHER_H

#include "connection.h"
#include "usb-host/usbdevice.h"

#include <QtCore/qobject.h>

class DeviceInformationFetcher : public QObject
{
    Q_OBJECT
public:
    struct Info
    {
        QString serial;
        QString hostMac;
        QString ipAddress;
    };

    DeviceInformationFetcher(std::shared_ptr<Connection> connection, UsbDevice device);

signals:
    void fetched(UsbDevice device, Info deviceInfo);

public slots:
    void fetch();

private slots:
    void handshakeResponse(QString serial, QString hostMac, QString ipAddress);

private:
    std::shared_ptr<Connection> m_connection;
    UsbDevice m_device;
};

#endif // DEVICEINFORMATIONFETCHER_H
