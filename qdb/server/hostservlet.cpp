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

#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtNetwork/qlocalsocket.h>

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
    const auto requestObject = request.object();

    if (requestObject["request"] == "devices") {
        replyDeviceInformation();
    } else if (requestObject["request"] == "stop-server") {
        stopServer();
    } else {
        qWarning() << "Got invalid request from client:" << requestBytes;
        m_socket->write(QByteArray{"{\"response\":\"invalid-request\""});
        close();
    }
}

void HostServlet::replyDeviceInformation()
{
    QJsonObject obj;
    QJsonArray infoArray;
    const auto deviceInfos = m_deviceManager.listDevices();
    for (const auto &deviceInfo : deviceInfos) {
        QJsonObject info;
        info["serial"] = deviceInfo.serial;
        info["hostMac"] = deviceInfo.hostMac;
        info["ipAddress"] = deviceInfo.ipAddress;
        infoArray << info;
    }

    obj["devices"] = infoArray;
    const QByteArray response = QJsonDocument{obj}.toJson(QJsonDocument::Compact);

    if (!m_socket || !m_socket->isWritable()) {
        qWarning() << "Could not reply to the client";
        return;
    }
    m_socket->write(response);
    qDebug() << "Replied device information to the client";
    close();
}

void HostServlet::stopServer()
{
    QJsonObject obj;
    obj["response"] = "stopping";

    const QByteArray response = QJsonDocument{obj}.toJson(QJsonDocument::Compact);

    if (!m_socket || !m_socket->isWritable()) {
        qWarning() << "Could not reply to the client";
        return;
    }
    m_socket->write(response);
    qDebug() << "Acknowledged stopping";

    emit serverStopRequested();
    // All servlets, including this one will be closed during shutdown
}
