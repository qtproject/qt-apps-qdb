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
#include "filepushexecutor.h"

#include "../utils/make_unique.h"
#include "filepushcommon.h"
#include "stream.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>

FilePushExecutor::FilePushExecutor(Stream *stream)
    : m_stream{stream},
      m_sink{nullptr}
{
    if (m_stream)
        connect(m_stream, &Stream::packetAvailable, this, &Executor::receive);
}

void FilePushExecutor::receive(StreamPacket packet)
{
    uint32_t typeValue;
    packet >> typeValue;
    auto type = toFilePushPacketType(typeValue);
    switch (type) {
    case FilePushOpen: {
            QString sinkPath;
            packet >> sinkPath;
            openSink(sinkPath);
            break;
        }
    case FilePushWrite: {
            QByteArray fileData;
            packet >> fileData;
            writeToSink(fileData);
            break;
        }
    case FilePushEnd:
        closeSink();
        break;
    case FilePushError:
        qDebug() << "FilePushError from host";
        if (m_sink)
            m_sink->remove();
        break;
    default:
        qFatal("Unsupported FilePushPacketType %d in ProcessExecutor::receive", type);
    }
}

void FilePushExecutor::openSink(const QString &path)
{
    qDebug() << "Opening sink file" << path;
    m_sink = make_unique<QFile>(path);
    StreamPacket packet;

    if (!m_sink->open(QIODevice::WriteOnly))
        packet << FilePushError;
    else
        packet << FilePushOpened;

    m_stream->write(packet);
}

void FilePushExecutor::writeToSink(const QByteArray &data)
{
    auto written = m_sink->write(data);

    StreamPacket packet;

    if (written != data.size())
        packet << FilePushError;
    else
        packet << FilePushWritten;

    m_stream->write(packet);
}

void FilePushExecutor::closeSink()
{
    qDebug() << "Closing sink file" << m_sink->fileName();
    m_sink->close();
}
