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
#include "hostserver.h"

#include "libqdb/interruptsignalhandler.h"
#include "libqdb/qdbconstants.h"
#include "networkmanagercontrol.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qtimer.h>
#include <QtDBus/QDBusObjectPath>
#include <QtNetwork/qlocalserver.h>
#include <QtNetwork/qlocalsocket.h>

void configureUsbNetwork(const QString &serial, const QString &macAddress)
{
    qDebug() << "Configuring network for" << serial << "at" << macAddress;
    NetworkManagerControl networkManager;
    auto deviceResult = networkManager.findNetworkDeviceByMac(macAddress);
    if (!deviceResult.isValid()) {
        qWarning() << "Could not find network device" << macAddress;
        return;
    } else {
        const auto networkCard = deviceResult.toString();
        if (networkManager.isActivated(networkCard)) {
            qDebug() << networkCard << "is activated";
            if (networkManager.isDeviceUsingLinkLocal(networkCard)) {
                qInfo() << networkCard << "is already using a link-local IP";
                return;
            }
        }
        if (!networkManager.activateOrCreateConnection(QDBusObjectPath{networkCard}, serial, macAddress))
            qWarning() << "Could not setup network settings for the USB Ethernet interface";
    }
}

int hostServer(QCoreApplication &app, const QCommandLineParser &parser)
{
    QString filterRules;
    if (!parser.isSet("debug-transport"))
        filterRules.append("transport=false\n");
    if (!parser.isSet("debug-connection"))
        filterRules.append("connection=false\n");
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
      m_client{nullptr},
      m_devices{},
      m_deviceInfos{},
      m_fetchIndex{-1}
{

}

void HostServer::listen()
{
#ifdef Q_OS_UNIX
    QFile::remove(QDir::tempPath() + qdbSocketName);
#endif
    if (!m_localServer.listen(qdbSocketName)) {
        qCritical() << "Could not start listening with QLocalServer: "
                    << m_localServer.errorString();
        close();
        return;
    }
    connect(&m_localServer, &QLocalServer::newConnection, this, &HostServer::handleConnection);
    qDebug() << "Host server started listening.";
}

void HostServer::close()
{
    m_localServer.close();
    emit closed();
}

void HostServer::handleConnection()
{
    Q_ASSERT_X(!m_client, "HostServer::handleConnection", "concurrent connections are not implemented"); // TODO
    m_client = m_localServer.nextPendingConnection();
    if (!m_client) {
        qCritical() << "Did not get a connection from client";
        close();
        return;
    }
    QObject::connect(m_client, &QLocalSocket::disconnected, this, &HostServer::handleDisconnection);
    QObject::connect(m_client, &QIODevice::readyRead, this, &HostServer::handleRequest);
}

void HostServer::handleDisconnection()
{
    m_client->deleteLater();
    m_client = nullptr;
}

void HostServer::handleDeviceInformation(DeviceInformation deviceInfo)
{
    m_deviceInfos.push_back(deviceInfo);
    ++m_fetchIndex;

    if (m_fetchIndex < m_devices.size())
        fetchDeviceInformation();
    else
        finishFetching();
}

void HostServer::handleRequest()
{
    const auto requestBytes = m_client->readLine(1000);
    const auto request = QJsonDocument::fromJson(requestBytes);
    const auto requestObject = request.object();

    if (requestObject["request"] == "devices") {
        startFetching();
    } else {
        qWarning() << "Got invalid request from client:" << requestBytes;
        m_client->disconnectFromServer();
    }
}

void HostServer::startFetching()
{
    m_devices = listUsbDevices();

    qDebug() << "USB devices:";
    for (const auto &device : m_devices) {
        qDebug() << "    " << device.serial;
    }

    Q_ASSERT_X(m_fetchIndex == -1, "HostServer::startFetching", "handling concurrent requests not implemented"); // TODO
    m_fetchIndex = 0;
    m_deviceInfos.clear();
    if (m_devices.empty())
        finishFetching();
    else
        fetchDeviceInformation();
}

void HostServer::finishFetching()
{
    m_fetchIndex = -1;

    qDebug() << "Configuring network for all devices";
    for (const auto deviceInfo : m_deviceInfos)
        configureUsbNetwork(deviceInfo.serial, deviceInfo.hostMac);

    QJsonObject obj;
    QJsonArray infoArray;
    for (const auto deviceInfo : m_deviceInfos) {
        QJsonObject info;
        info["serial"] = deviceInfo.serial;
        info["hostMac"] = deviceInfo.hostMac;
        info["ipAddress"] = deviceInfo.ipAddress;
        infoArray << info;
    }
    obj["devices"] = infoArray;
    const QByteArray response = QJsonDocument{obj}.toJson(QJsonDocument::Compact);

    if (!m_client || !m_client->isWritable()) {
        qWarning() << "Could not reply to the client";
        return;
    }
    m_client->write(response);
    m_client->waitForBytesWritten();
    m_client->disconnectFromServer();
    qDebug() << "Replied to the client";
}

void HostServer::fetchDeviceInformation()
{
    qDebug() << "Fetching device information for" << m_fetchIndex;
    auto *fetcher = new DeviceInformationFetcher{m_devices[m_fetchIndex]};
    connect(fetcher, &DeviceInformationFetcher::fetched, fetcher, &QObject::deleteLater);
    connect(fetcher, &DeviceInformationFetcher::fetched, this, &HostServer::handleDeviceInformation);

    fetcher->fetch();
}
