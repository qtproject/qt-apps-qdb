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
#ifndef HOSTSERVER_H
#define HOSTSERVER_H

#include "deviceinformationfetcher.h"
#include "devicemanager.h"
#include "hostservlet.h"

#include <QtCore/qobject.h>
#include <QtNetwork/qlocalserver.h>
QT_BEGIN_NAMESPACE
class QCoreApplication;
class QCommandLineParser;
QT_END_NAMESPACE

int execHostServer(const QCoreApplication &app, const QCommandLineParser &parser);

class HostServer : public QObject
{
    Q_OBJECT
public:
    explicit HostServer(QObject *parent = nullptr);

    void listen();

signals:
    void closed();

public slots:
    void close();

private slots:
    void handleClient();
    void handleDoneClient(ServletId servletId);
    void handleNewDeviceInfo(DeviceInformation info);
    void handleDisconnectedDevice(QString serial);

private:
    QLocalServer m_localServer;
    std::list<HostServlet> m_servlets;
    DeviceManager m_deviceManager;
};

#endif // HOSTSERVER_H
