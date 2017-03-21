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
#include "networkconfigurationexecutor.h"

#include "networkconfiguration.h"
#include "libqdb/stream.h"

#include <QtCore/qloggingcategory.h>

Q_LOGGING_CATEGORY(configurationExecC, "qdb.executors.networkconfiguration")

NetworkConfigurationExecutor::NetworkConfigurationExecutor(Stream *stream)
    : m_stream{stream}
{
    if (m_stream)
        connect(m_stream, &Stream::packetAvailable, this, &Executor::receive);
}

void NetworkConfigurationExecutor::receive(StreamPacket packet)
{
    QString subnetString;
    packet >> subnetString;

    if (subnetString.isEmpty()) {
        simpleResponse(ConfigurationResult::Failure);
        return;
    }

    auto *networkConfiguration = NetworkConfiguration::instance();

    const auto result = networkConfiguration->set(subnetString);
    if (result == ConfigurationResult::AlreadySet) {
        StreamPacket response;
        const auto value = static_cast<uint32_t>(result);
        response << value;
        response << networkConfiguration->subnet();
        m_stream->write(response);
        return;
    }
    simpleResponse(result);
}

void NetworkConfigurationExecutor::simpleResponse(ConfigurationResult result)
{
    StreamPacket response;
    uint32_t value = static_cast<uint32_t>(result);
    response << value;
    m_stream->write(response);
}
