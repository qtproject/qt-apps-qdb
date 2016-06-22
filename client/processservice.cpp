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
#include "processservice.h"

#include "connection.h"
#include "processcommon.h"
#include "protocol/services.h"
#include "stream.h"

#include <QtCore/qbytearray.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>

ProcessService::ProcessService(Connection *connection)
    : m_connection{connection}
{

}

void ProcessService::initialize()
{
    m_connection->createStream(tagBuffer(ProcessTag), [=](Stream *stream) {
        this->streamCreated(stream);
    });
}

bool ProcessService::execute(const QString &process, const QStringList &arguments)
{
    if (!m_stream) {
        qCritical() << "No valid stream in ProcessService when trying to send";
        return false;
    }

    StreamPacket packet;
    packet << ProcessStart << process << arguments;

    return m_stream->write(packet);
}

QByteArray ProcessService::read()
{
    if (m_reads.isEmpty()) {
        qWarning() << "ProcessService::read(): read from empty queue";
        return QByteArray{};
    }
    return m_reads.dequeue();
}

qint64 ProcessService::write(const QByteArray &data)
{
    if (!m_stream) {
        qCritical() << "No valid stream in ProcessService when trying to write";
        return -1;
    }

    StreamPacket packet;
    packet << ProcessWrite << data;

    return m_stream->write(packet) ? data.size() : -1;
}

void ProcessService::receive(StreamPacket packet)
{
    uint32_t typeValue;
    packet >> typeValue;
    auto type = toProcessPacketType(typeValue);
    switch (type) {
    case ProcessStarted: {
            emit started();
            break;
        }
    case ProcessRead: {
            QByteArray buffer;
            packet >> buffer;
            m_reads.enqueue(buffer);
            emit readyRead();
            break;
        }
    case ProcessError: {
            uint32_t errorValue;
            packet >> errorValue;
            emit executionError(static_cast<QProcess::ProcessError>(errorValue));
            break;
        }
    case ProcessFinished: {
            int32_t exitCode;
            bool normalExit;
            QByteArray output;
            packet >> exitCode >> normalExit >> output;

            QProcess::ExitStatus exitStatus = normalExit ? QProcess::NormalExit : QProcess::CrashExit;

            emit executed(exitCode, exitStatus, output);
            break;
        }
    default:
        Q_ASSERT_X(false, "ProcessService::receive", "Unsupported ProcessPacketType");
        break;
    }

}
