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
#include "handshakeexecutor.h"

#include "configuration.h"
#include "libqdb/stream.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qloggingcategory.h>
#include <QtNetwork/qnetworkinterface.h>

Q_LOGGING_CATEGORY(handshakeC, "qdb.executors.handshake");

QString rndisFunctionPath()
{
    return Configuration::gadgetConfigFsDir() + "/functions/" + Configuration::rndisFunctionName();
}

QString deviceIpAddress()
{
    QFile file{rndisFunctionPath() + "/ifname"};
    if (!file.open(QIODevice::ReadOnly)) {
        qCCritical(handshakeC) << "Could not find network interface name from RNDIS configuration at" << rndisFunctionPath();
        return "";
    }

    const auto interfaceName = QString{file.readAll()}.trimmed();
    const auto interface = QNetworkInterface::interfaceFromName(interfaceName);
    const auto addressEntries = interface.addressEntries();

    for (const auto &entry : addressEntries) {
        const auto ip = entry.ip();
        if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
            qCDebug(handshakeC) << "Device IP address:" << ip.toString();
            return ip.toString();
        }
    }
    return "";
}

QString deviceSerial()
{
    QFile file{Configuration::gadgetConfigFsDir() + "/strings/0x409/serialnumber"};
    if (!file.open(QIODevice::ReadOnly)) {
        qCCritical(handshakeC) << "Could not find device serial number from configfs configuration at" << Configuration::gadgetConfigFsDir();
        return "";
    }
    return QString{file.readAll()}.trimmed();
}

QString hostSideMac()
{
    QFile file{rndisFunctionPath() + "/host_addr"};
    if (!file.open(QIODevice::ReadOnly)) {
        qCCritical(handshakeC) << "Could not find host MAC address from RNDIS configuration at" << rndisFunctionPath();
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

    StreamPacket response;
    response << deviceSerial();
    response << hostSideMac();
    response << deviceIpAddress();
    m_stream->write(response);
    qCDebug(handshakeC) << "Responded to handshake with device information";
}
