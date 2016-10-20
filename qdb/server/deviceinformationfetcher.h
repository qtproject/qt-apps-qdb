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

struct DeviceInformation
{
    QString serial;
    QString hostMac;
    QString ipAddress;
    UsbAddress usbAddress;
};

bool operator==(const DeviceInformation &lhs, const DeviceInformation &rhs);
bool operator!=(const DeviceInformation &lhs, const DeviceInformation &rhs);

class DeviceInformationFetcher : public QObject
{
    Q_OBJECT
public:
    explicit DeviceInformationFetcher(UsbDevice device);

signals:
    void fetched(DeviceInformation deviceInfo);

public slots:
    void fetch();

private slots:
    void handshakeResponse(QString serial, QString hostMac, QString ipAddress);

private:
    Connection *m_connection; // owned by this class, deletion set up in constructor
    UsbAddress m_deviceAddress;
    bool m_connected;
};

#endif // DEVICEINFORMATIONFETCHER_H
