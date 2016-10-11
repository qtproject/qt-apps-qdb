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
#ifndef NETWORKMANAGERCONTROL_H
#define NETWORKMANAGERCONTROL_H

#include <QtCore/qvariant.h>
#include <QtDBus/qdbusconnection.h>
QT_BEGIN_NAMESPACE
class QDBusObjectPath;
QT_END_NAMESPACE

class NetworkManagerControl
{
public:
    NetworkManagerControl();

    bool activateOrCreateConnection(const QDBusObjectPath &devicePath, const QString &serial, const QString &macAddress);
    QVariant createConnection(const QString &serial, const QString &macAddress);
    QVariant findConnectionsByMac(const QString &macAddress);
    QVariant findNetworkDeviceByMac(const QString &macAddress);
    bool isActivated(const QString &devicePath);
    bool isConnectionUsingLinkLocal(const QString &connectionPath);
    bool isDeviceUsingLinkLocal(const QString &devicePath);
    QVariant listNetworkDevices();

private:
    QDBusConnection m_bus;
};


#endif // NETWORKMANAGERCONTROL_H
