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
#include "handshakeexecutor.h"

#include "configuration.h"
#include "libqdb/stream.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtNetwork/qnetworkinterface.h>

QString rndisFunctionPath()
{
    return Configuration::gadgetConfigFsDir() + "/functions/" + Configuration::rndisFunctionName();
}

QString deviceIpAddress()
{
    QFile file{rndisFunctionPath() + "/ifname"};
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not find network interface name from RNDIS configuration at" << rndisFunctionPath();
        return "";
    }

    const auto interfaceName = QString{file.readAll()}.trimmed();
    const auto interface = QNetworkInterface::interfaceFromName(interfaceName);
    const auto addressEntries = interface.addressEntries();

    for (const auto &entry : addressEntries) {
        const auto ip = entry.ip();
        if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
            qDebug() << "Device IP address:" << ip.toString();
            return ip.toString();
        }
    }
    return "";
}

QString deviceSerial()
{
    QFile file{Configuration::gadgetConfigFsDir() + "/strings/0x409/serialnumber"};
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not find device serial number from configfs configuration at" << Configuration::gadgetConfigFsDir();
        return "";
    }
    return QString{file.readAll()}.trimmed();
}

QString hostSideMac()
{
    QFile file{rndisFunctionPath() + "/host_addr"};
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not find host MAC address from RNDIS configuration at" << rndisFunctionPath();
        return "";
    }
    return QString{file.readAll()}.trimmed();
}

HandshakeExecutor::HandshakeExecutor(Stream *stream)
    : m_stream{stream}
{
    if (m_stream)
        connect(m_stream, &Stream::packetAvailable, this, &Executor::receive);
}

void HandshakeExecutor::receive(StreamPacket packet)
{
    Q_UNUSED(packet);

    qDebug() << "Responding to handshake with device information.";
    StreamPacket response;
    response << deviceSerial();
    response << hostSideMac();
    response << deviceIpAddress();
    m_stream->write(response);
}
