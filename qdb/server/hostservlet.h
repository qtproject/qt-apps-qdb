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
