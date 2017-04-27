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

    const auto result = static_cast<ConfigurationResult>(value);
    if (result != ConfigurationResult::Success
            && result != ConfigurationResult::Failure
            && result != ConfigurationResult::AlreadySet) {
        qCCritical(configurationC) << "Unknown network configuration result" << value
                                   << "received from device";
        failedResponse();
    }

    if (result == ConfigurationResult::AlreadySet) {
        QString subnet;
        packet >> subnet;

        m_responded = true;
        emit alreadySetResponse(subnet);
        return;
    }

    m_responded = true;
    emit response(result);
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
        emit response(ConfigurationResult::Failure);
        m_responded = true;
    }
}
