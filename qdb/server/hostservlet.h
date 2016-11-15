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
#ifndef HOSTSERVLET_H
#define HOSTSERVLET_H

#include "devicemanager.h"

#include <QtCore/qobject.h>
QT_BEGIN_NAMESPACE
class QLocalSocket;
QT_END_NAMESPACE

using ServletId = uint32_t;

// Takes ownership of the passed QLocalSocket
class HostServlet : public QObject
{
    Q_OBJECT
public:
    HostServlet(QLocalSocket *socket, DeviceManager &deviceManager);
    ~HostServlet();

    void close();
    ServletId id() const;

signals:
    void done(ServletId id);
    void serverStopRequested();

public slots:
    void handleDisconnection();
    void handleRequest();

private:
    void replyDevices();
    void replyNewDevice(const DeviceInformation &deviceInfo);
    void replyDisconnectedDevice(const QString &serial);
    void startWatchingDevices();
    void stopServer();

    ServletId m_id;
    QLocalSocket *m_socket;
    DeviceManager &m_deviceManager;
};

#endif // HOSTSERVLET_H
