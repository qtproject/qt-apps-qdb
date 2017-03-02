/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
#include "networkconfigurationservice.h"

#include "connection.h"
#include "libqdb/protocol/services.h"
#include "libqdb/stream.h"

#include <QtCore/qloggingcategory.h>

Q_LOGGING_CATEGORY(configurationC, "qdb.services.networkconfiguration")

NetworkConfigurationService::NetworkConfigurationService(Connection *connection)
    : m_connection{connection},
      m_responded{false}
{

}

NetworkConfigurationService::~NetworkConfigurationService()
{
    if (m_stream)
        m_stream->requestClose();
}

void NetworkConfigurationService::initialize()
{
    connect(m_connection, &Connection::disconnected,
            this, &NetworkConfigurationService::handleDisconnected);
    m_connection->createStream(tagBuffer(NetworkConfigurationTag), [=](Stream *stream) {
        this->streamCreated(stream);
    });
}

void NetworkConfigurationService::configure(QString subnet)
{
    if (!m_stream) {
        qCCritical(configurationC)
                << "No valid stream in NetworkConfigurationService when trying to send";
        return;
    }
    StreamPacket packet;
    packet << subnet;
    m_stream->write(packet);
}

void NetworkConfigurationService::receive(StreamPacket packet)
{
    uint32_t value;
    packet >> value;

    m_responded = true;
    emit response(value == 1);
}

void NetworkConfigurationService::onStreamClosed()
{
    Service::onStreamClosed();
    failedResponse();
}

void NetworkConfigurationService::handleDisconnected()
{
    failedResponse();
}

void NetworkConfigurationService::failedResponse()
{
    if (!m_responded) {
        emit response(false);
        m_responded = true;
    }
}
