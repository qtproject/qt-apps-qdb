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
