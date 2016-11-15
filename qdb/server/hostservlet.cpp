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
#include "hostservlet.h"

#include "hostmessages.h"

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsonobject.h>
#include <QtNetwork/qlocalsocket.h>

QJsonObject deviceInformationToJsonObject(const DeviceInformation &deviceInfo)
{
    QJsonObject info;
    info["serial"] = deviceInfo.serial;
    info["hostMac"] = deviceInfo.hostMac;
    info["ipAddress"] = deviceInfo.ipAddress;
    return info;
}

ServletId newServletId()
{
    static ServletId nextId = 0;
    return ++nextId;
}

HostServlet::HostServlet(QLocalSocket *socket, DeviceManager &deviceManager)
    : m_id{newServletId()},
      m_socket{socket},
      m_deviceManager{deviceManager}

{

}

HostServlet::~HostServlet()
{
    m_socket->deleteLater();
}

void HostServlet::close()
{
    m_socket->waitForBytesWritten();
    m_socket->disconnectFromServer();
}

ServletId HostServlet::id() const
{
    return m_id;
}

void HostServlet::handleDisconnection()
{
    emit done(m_id);
}

void HostServlet::handleRequest()
{
    const auto requestBytes = m_socket->readLine();
    const auto request = QJsonDocument::fromJson(requestBytes);
    const auto type = requestType(request.object());

    switch (type) {
    case RequestType::Devices:
        replyDevices();
        break;
    case RequestType::WatchDevices:
        startWatchingDevices();
        break;
    case RequestType::StopServer:
        stopServer();
        break;
    case RequestType::Unknown:
        qWarning() << "Got invalid request from client:" << requestBytes;
        const QJsonObject response = initializeResponse(ResponseType::InvalidRequest);
        m_socket->write(serialiseResponse(response));
        close();
        break;
    }
}

void HostServlet::replyDevices()
{
    QJsonArray infoArray;
    const auto deviceInfos = m_deviceManager.listDevices();
    for (const auto &deviceInfo : deviceInfos)
        infoArray << deviceInformationToJsonObject(deviceInfo);

    QJsonObject response = initializeResponse(ResponseType::Devices);
    response["devices"] = infoArray;

    if (!m_socket || !m_socket->isWritable()) {
        qWarning() << "Could not reply to the client";
        return;
    }
    m_socket->write(serialiseResponse(response));
    qDebug() << "Replied device information to the client";
    close();
}

void HostServlet::replyNewDevice(const DeviceInformation &deviceInfo)
{
    QJsonObject response = initializeResponse(ResponseType::NewDevice);
    response["device"] = deviceInformationToJsonObject(deviceInfo);

    if (!m_socket || !m_socket->isWritable()) {
        qWarning() << "Could not reply to the client";
        return;
    }
    m_socket->write(serialiseResponse(response));
    qDebug() << "Sent new device information to the client";
}

void HostServlet::replyDisconnectedDevice(const QString &serial)
{
    QJsonObject response = initializeResponse(ResponseType::DisconnectedDevice);
    response["serial"] = serial;

    if (!m_socket || !m_socket->isWritable()) {
        qWarning() << "Could not reply to the client";
        return;
    }
    m_socket->write(serialiseResponse(response));
    qDebug() << "Sent disconnected device information to the client";
}

void HostServlet::startWatchingDevices()
{
    qDebug() << "Starting to watch devices";
    connect(&m_deviceManager, &DeviceManager::newDeviceInfo, this, &HostServlet::replyNewDevice);
    connect(&m_deviceManager, &DeviceManager::disconnectedDevice, this, &HostServlet::replyDisconnectedDevice);

    const auto deviceInfos = m_deviceManager.listDevices();
    for (const auto &deviceInfo : deviceInfos)
        replyNewDevice(deviceInfo);
    qDebug() << "Reported initial devices to watcher";
}

void HostServlet::stopServer()
{
    QJsonObject response = initializeResponse(ResponseType::Stopping);

    if (!m_socket || !m_socket->isWritable()) {
        qWarning() << "Could not reply to the client";
        return;
    }
    m_socket->write(serialiseResponse(response));
    qDebug() << "Acknowledged stopping";

    emit serverStopRequested();
    // All servlets, including this one will be closed during shutdown
}
