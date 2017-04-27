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
#include "handshakeservice.h"

#include "connection.h"
#include "libqdb/protocol/services.h"
#include "libqdb/stream.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

Q_LOGGING_CATEGORY(handshakeC, "qdb.services.handshake");

HandshakeService::HandshakeService(Connection *connection)
    : m_connection{connection},
      m_responded{false}
{

}

HandshakeService::~HandshakeService()
{
    if (m_stream)
        m_stream->requestClose();
}

void HandshakeService::initialize()
{
    connect(m_connection, &Connection::disconnected, this, &HandshakeService::handleDisconnected);
    m_connection->createStream(tagBuffer(HandshakeTag), [=](Stream *stream) {
        this->streamCreated(stream);
    });
}

void HandshakeService::ask()
{
    if (!m_stream) {
        qCCritical(handshakeC) << "No valid stream in HandshakeService when trying to send";
        return;
    }
    StreamPacket packet{};
    packet << 0;
    m_stream->write(packet);
}

void HandshakeService::receive(StreamPacket packet)
{
    QString serial;
    QString macAddress;
    QString deviceIpAddress;
    packet >> serial >> macAddress >> deviceIpAddress;

    m_responded = true;
    emit response(serial, macAddress, deviceIpAddress);
}

void HandshakeService::onStreamClosed()
{
    Service::onStreamClosed();
    failedResponse();
}

void HandshakeService::handleDisconnected()
{
    failedResponse();
}

void HandshakeService::failedResponse()
{
    if (!m_responded) {
        emit response("", "", "");
        m_responded = true;
    }
}
