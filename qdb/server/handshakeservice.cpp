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
#include "handshakeservice.h"

#include "connection.h"
#include "libqdb/protocol/services.h"
#include "libqdb/stream.h"

#include <QtCore/qdebug.h>

HandshakeService::HandshakeService(Connection *connection)
    : m_connection{connection}
{

}

HandshakeService::~HandshakeService()
{
    if (m_stream)
        m_stream->requestClose();
}

void HandshakeService::initialize()
{
    m_connection->createStream(tagBuffer(HandshakeTag), [=](Stream *stream) {
        this->streamCreated(stream);
    });
}

bool HandshakeService::hasStream() const
{
    return m_stream != nullptr;
}

void HandshakeService::ask()
{
    if (!m_stream) {
        qCritical() << "No valid stream in HandshakeService when trying to send";
        return;
    }
    StreamPacket packet{};
    packet << 0;
    m_stream->write(packet);
}

void HandshakeService::close()
{
    m_stream->requestClose();
}

void HandshakeService::receive(StreamPacket packet)
{
    QString serial;
    QString macAddress;
    QString deviceIpAddress;
    packet >> serial >> macAddress >> deviceIpAddress;

    emit response(serial, macAddress, deviceIpAddress);
}
