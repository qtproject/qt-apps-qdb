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

#include "configuration.h"
#include "libqdb/stream.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qprocess.h>

Q_LOGGING_CATEGORY(configurationC, "qdb.executors.networkconfiguration")

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
        failedResponse();
        return;
    }

    QProcess process;
    process.start(Configuration::networkScript(), QStringList{"--set", subnetString});
    process.waitForFinished();
    if (process.exitCode() != 0) {
        qCWarning(configurationC) << "Using script to configure the network failed";
        failedResponse();
        return;
    }

    StreamPacket response;
    uint32_t value = 1u;
    response << value;
    m_stream->write(response);
    qCDebug(configurationC) << "Configured device network to" << subnetString;
}

void NetworkConfigurationExecutor::failedResponse()
{
    StreamPacket response;
    uint32_t value = 0u;
    response << value;
    m_stream->write(response);
}
