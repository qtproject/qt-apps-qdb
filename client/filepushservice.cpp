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
#include "filepushservice.h"

#include "../utils/make_unique.h"
#include "connection.h"
#include "filepushcommon.h"
#include "protocol/services.h"

#include <QtCore/qbytearray.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>

FilePushService::FilePushService(Connection *connection)
    : m_connection{connection},
      m_hostPath{},
      m_devicePath{},
      m_source{nullptr},
      m_transferring{false}
{

}

FilePushService::~FilePushService()
{
    if (m_source && m_source->isOpen())
        m_source->close();
}

void FilePushService::initialize()
{
    m_connection->createStream(tagBuffer(FilePushTag), [=](Stream *stream) {
        this->streamCreated(stream);
    });
}

bool FilePushService::push(const QString &hostPath, const QString& devicePath)
{
    if (!m_stream) {
        qCritical() << "No valid stream in FilePushService when trying to send";
        return false;
    }
    m_hostPath = hostPath;
    m_devicePath = devicePath;

    StreamPacket packet;
    packet << FilePushOpen << m_devicePath;

    return m_stream->write(packet);
}

void FilePushService::receive(StreamPacket packet)
{
    uint32_t typeValue;
    packet >> typeValue;
    auto type = toFilePushPacketType(typeValue);
    switch (type) {
    case FilePushOpened:
        qDebug() << "FilePushOpened";
        transferBlock();
        break;
    case FilePushWritten:
        qDebug() << "FilePushWritten";
        if (m_transferring)
            transferBlock();
        else
            endTransfer();
        break;
    case FilePushError:
        m_transferring = false;
        qDebug() << "FilePushError";
        emit error("Error on device while pushing");
        break;
    default:
        qFatal("Unsupported FilePushPacketType %d", type);
    }
}

bool FilePushService::openSource()
{
    m_source = make_unique<QFile>(m_hostPath);
    if (!m_source->open(QIODevice::ReadOnly)) {
        StreamPacket packet;
        packet << FilePushError;

        m_stream->write(packet);

        emit error("Could not open " + m_source->fileName() + " on host");
        return false;
    }
    return true;
}

void FilePushService::transferBlock()
{
    if (!m_transferring) {
        if (!openSource())
            return;
        m_transferring = true;
    }

    QByteArray block = m_source->read(fileTransferBlockSize);
    m_transferring = !m_source->atEnd();

    StreamPacket packet;
    packet << FilePushWrite << block;

    m_stream->write(packet);
}

void FilePushService::endTransfer()
{
    StreamPacket packet;
    packet << FilePushEnd;

    m_stream->write(packet);

    emit pushed();
}
