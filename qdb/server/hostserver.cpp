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
#include "hostserver.h"

#include "libqdb/interruptsignalhandler.h"
#include "libqdb/qdbconstants.h"
#include "logging.h"

#include <QtCore/qcommandlineparser.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qloggingcategory.h>
#include <QtNetwork/qlocalsocket.h>

Q_LOGGING_CATEGORY(hostServerC, "qdb.hostserver");

int execHostServer(const QCoreApplication &app, const QCommandLineParser &parser)
{
    setupLogging();

    QString filterRules;
    if (!parser.isSet("debug-transport"))
        filterRules.append("qdb.transport.debug=false\n");
    if (!parser.isSet("debug-connection"))
        filterRules.append("qdb.connection.debug=false\n");
    QLoggingCategory::setFilterRules(filterRules);

    InterruptSignalHandler signalHandler;
    HostServer hostServer;
    QObject::connect(&signalHandler, &InterruptSignalHandler::interrupted, &hostServer, &HostServer::close);
    QObject::connect(&hostServer, &HostServer::closed, &app, &QCoreApplication::quit);
    QTimer::singleShot(0, &hostServer, &HostServer::listen);

    return app.exec();
}

HostServer::HostServer(QObject *parent)
    : QObject{parent},
      m_localServer{},
      m_servlets{},
      m_deviceManager{}
{

}

void HostServer::listen()
{
#ifdef Q_OS_UNIX
    QString socketPath = QDir::cleanPath(QDir::tempPath()) + QChar{'/'};
    socketPath += qdbSocketName;
    QFile::remove(socketPath);
#endif
    if (!m_localServer.listen(qdbSocketName)) {
        qCCritical(hostServerC) << "Could not start listening with QLocalServer: "
                                << m_localServer.errorString();
        close();
        return;
    }
    connect(&m_localServer, &QLocalServer::newConnection, this, &HostServer::handleClient);
    qCDebug(hostServerC) << "Started listening";

    connect(&m_deviceManager, &DeviceManager::newDeviceInfo, this, &HostServer::handleNewDeviceInfo);
    connect(&m_deviceManager, &DeviceManager::disconnectedDevice, this, &HostServer::handleDisconnectedDevice);
    m_deviceManager.start();
}

void HostServer::close()
{
    qCDebug(hostServerC) << "Shutting down";
    m_localServer.close();
    while (!m_servlets.empty())
        m_servlets.front().close(); // closing results in being erased from the list
    emit closed();
}

void HostServer::handleClient()
{
    QLocalSocket *socket = m_localServer.nextPendingConnection();
    if (!socket) {
        qCCritical(hostServerC) << "Did not get a connection from client";
        close();
        return;
    }
    m_servlets.emplace_back(socket, m_deviceManager);
    auto servlet = &m_servlets.back();

    connect(socket, &QLocalSocket::disconnected, servlet, &HostServlet::handleDisconnection);
    connect(socket, &QIODevice::readyRead, servlet, &HostServlet::handleRequest);

    connect(servlet, &HostServlet::done, this, &HostServer::handleDoneClient);
    connect(servlet, &HostServlet::serverStopRequested, this, &HostServer::close);
}

void HostServer::handleDoneClient(ServletId servletId)
{
    auto iter = std::find_if(m_servlets.begin(), m_servlets.end(),
                             [=](const HostServlet &servlet) {
                                 return servlet.id() == servletId;
                             });
    if (iter != m_servlets.end())
        m_servlets.erase(iter);
    else
        qCWarning(hostServerC) << "Could not find done servlet" << servletId << "when trying to remove it";
}

void HostServer::handleNewDeviceInfo(DeviceInformation info)
{
    qCDebug(hostServerC) << "New device information about" << info.serial;
}

void HostServer::handleDisconnectedDevice(QString serial)
{
    qCDebug(hostServerC) << "Disconnected" << serial;
}
