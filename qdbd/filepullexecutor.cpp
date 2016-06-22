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
#include "filepullexecutor.h"

#include "../utils/make_unique.h"
#include "filepullcommon.h"
#include "protocol/services.h"
#include "stream.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>

FilePullExecutor::FilePullExecutor(Stream *stream)
    : m_stream{stream},
      m_source{nullptr},
      m_transferring{false}
{
    if (m_stream)
        connect(m_stream, &Stream::packetAvailable, this, &Executor::receive);
}

void FilePullExecutor::receive(StreamPacket packet)
{
    uint32_t typeValue;
    packet >> typeValue;
    auto type = toFilePullPacketType(typeValue);
    switch (type) {
    case FilePullOpen: {
            QString sourcePath;
            packet >> sourcePath;
            if (openSource(sourcePath))
                transferBlock();
            break;
        }
    case FilePullWasRead:
        qDebug() << "File pull read acknowledged.";
        if (m_transferring)
            transferBlock();
        else
            closeSource();
        break;
    case FilePullError:
        qDebug() << "FilePullError from host";
        closeSource();
        break;
    default:
        qFatal("Unsupported FilePushPacketType %d in ProcessExecutor::receive", type);
    }
}

bool FilePullExecutor::openSource(const QString &path)
{
    qDebug() << "Opening source file" << path;
    m_source = make_unique<QFile>(path);
    bool opened = m_source->open(QIODevice::ReadOnly);

    StreamPacket packet;

    if (!opened) {
        packet << FilePullError
               << QString{"Could not open \"%1\" on device"}.arg(m_source->fileName());
    } else {
        packet << FilePullOpened;
    }

    return m_stream->write(packet) && opened;
}

void FilePullExecutor::transferBlock()
{
    QByteArray block = m_source->read(fileTransferBlockSize);
    m_transferring = !m_source->atEnd();

    StreamPacket packet;
    packet << FilePullRead << block;

    m_stream->write(packet);
}

void FilePullExecutor::closeSource()
{
    qDebug() << "Closing source file" << m_source->fileName();
    m_source->close();

    StreamPacket packet;
    packet << FilePullEnd;

    m_stream->write(packet);
}
