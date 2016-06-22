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
#include "processexecutor.h"

#include "../utils/make_unique.h"
#include "processcommon.h"
#include "stream.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtCore/qprocess.h>

ProcessExecutor::ProcessExecutor(Stream *stream)
    : m_stream{stream},
      m_process{nullptr}
{
    if (m_stream) {
        connect(m_stream, &Stream::packetAvailable, this, &Executor::receive);
        connect(m_stream, &Stream::closed, this, &Executor::onStreamClosed);
    }
}

void ProcessExecutor::receive(StreamPacket packet)
{
    uint32_t typeValue;
    packet >> typeValue;
    auto type = toProcessPacketType(typeValue);
    switch (type) {
    case ProcessStart: {
            QString command;
            QStringList arguments;
            packet >> command >> arguments;
            startProcess(command, arguments);
            break;
        }
    case ProcessWrite: {
            QByteArray data;
            packet >> data;
            writeToProcess(data);
            break;
        }
    default:
        Q_ASSERT_X(false, "ProcessExecutor::receive", "Unsupported ProcessPacketType");
        break;
    }
}

void ProcessExecutor::onStarted()
{
    qDebug() << "Process started";

    StreamPacket packet;
    packet << ProcessStarted;

    if (m_stream)
        m_stream->write(packet);
}

void ProcessExecutor::onReadyRead()
{
    qDebug() << "Process readyRead";
    auto size = m_process->bytesAvailable();
    QByteArray read = m_process->read(size);

    StreamPacket packet;
    packet << ProcessRead;
    packet << read;

    if (m_stream)
        m_stream->write(packet);
}

void ProcessExecutor::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Process finished:" << exitCode;

    QByteArray output = m_process->readAll();

    StreamPacket packet;
    packet << ProcessFinished;
    packet << exitCode;
    packet << (exitStatus == QProcess::NormalExit);
    packet << output;

    if (m_stream)
        m_stream->write(packet);
}

void ProcessExecutor::onErrorOccurred(QProcess::ProcessError error)
{
    qDebug() << "Process error:" << error;

    StreamPacket packet;
    packet << ProcessError;
    uint32_t errorValue = static_cast<uint32_t>(error);
    packet << errorValue;

    if (m_stream)
        m_stream->write(packet);
}

void ProcessExecutor::onStreamClosed()
{
    m_stream = nullptr;
    if (m_process) {
        m_process->kill();
    }
}

void ProcessExecutor::startProcess(const QString &command, const QStringList &arguments)
{
    m_process = make_unique<QProcess>();
    // merge stdout and stderr
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process.get(), &QProcess::started, this, &ProcessExecutor::onStarted);
    connect(m_process.get(), &QProcess::readyRead, this, &ProcessExecutor::onReadyRead);
    connect(m_process.get(), static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &ProcessExecutor::onFinished);
    connect(m_process.get(), &QProcess::errorOccurred, this, &ProcessExecutor::onErrorOccurred);

    qDebug() << "Running" << command << arguments;
    m_process->start(command, arguments);
}

void ProcessExecutor::writeToProcess(const QByteArray &data)
{
    Q_ASSERT(m_process);

    qDebug() << "Writing to process:" << data;
    m_process->write(data);
}
