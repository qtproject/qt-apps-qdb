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
#include "filepullservice.h"

#include "../utils/make_unique.h"
#include "connection.h"
#include "filepullcommon.h"
#include "protocol/services.h"

#include <QtCore/qbytearray.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>

FilePullService::FilePullService(Connection *connection)
    : m_connection{connection},
      m_hostPath{},
      m_devicePath{},
      m_sink{nullptr}
{

}

FilePullService::~FilePullService() = default;

void FilePullService::initialize()
{
    m_connection->createStream(tagBuffer(FilePullTag), [=](Stream *stream) {
        this->streamCreated(stream);
    });
}

bool FilePullService::pull(const QString &devicePath, const QString &hostPath)
{
    if (!m_stream) {
        qCritical() << "No valid stream in FilePullService when trying to send";
        return false;
    }
    m_devicePath = devicePath;
    m_hostPath = hostPath;

    StreamPacket packet;
    packet << FilePullOpen << m_devicePath;

    return m_stream->write(packet);
}

void FilePullService::receive(StreamPacket packet)
{
    Q_ASSERT(m_stream);
    uint32_t typeValue;
    packet >> typeValue;
    auto type = toFilePullPacketType(typeValue);
    switch (type) {
    case FilePullOpened:
        qDebug() << "Opened device file for pull";
        openSink();
        break;
    case FilePullRead: {
            QByteArray fileData;
            packet >> fileData;
            writeToSink(fileData);
            break;
        }
    case FilePullEnd:
        qDebug() << "Pull complete";
        closeSink();
        emit pulled();
        break;
    case FilePullError: {
            QString errorMessage;
            packet >> errorMessage;
            qWarning() << "Error on device while pulling:" << errorMessage;
            emit error(errorMessage);
            m_stream->requestClose();
            break;
        }
    default:
        qFatal("Unsupported FilePullPacketType %d", type);
    }
}

void FilePullService::openSink()
{
    Q_ASSERT(m_stream);
    qDebug() << "Opening sink file" << m_hostPath;
    m_sink = make_unique<QFile>(m_hostPath);
    if (!m_sink->open(QIODevice::WriteOnly)) {
        StreamPacket packet;
        packet << FilePullError;
        m_stream->write(packet);
        auto errorMessage = QString{"Could not open sink file \"%1\" on host"}.arg(m_sink->fileName());
        qDebug() << "Error on host:" << errorMessage;
        emit error(errorMessage);
        m_stream->requestClose();
    }
}

void FilePullService::writeToSink(const QByteArray &data)
{
    Q_ASSERT(m_stream);

    if (!m_sink->isOpen()) {
        qDebug() << "Skipping write to closed sink";
        return;
    }

    auto written = m_sink->write(data);

    StreamPacket packet;

    if (written != data.size()) {
        qDebug() << "Error in writing to sink";
        packet << FilePullError;
        m_stream->write(packet);
        m_stream->requestClose();
    } else {
        packet << FilePullWasRead;
        m_stream->write(packet);
    }
}

void FilePullService::closeSink()
{
    Q_ASSERT(m_stream);
    qDebug() << "Closing sink file" << m_sink->fileName();
    m_sink->close();
}
