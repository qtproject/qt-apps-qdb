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
#include "echoservice.h"

#include "connection.h"
#include "libqdb/protocol/services.h"
#include "libqdb/stream.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

Q_LOGGING_CATEGORY(echoC, "qdb.services.echo");

EchoService::EchoService(Connection *connection)
    : m_connection{connection}
{

}

EchoService::~EchoService()
{
    if (m_stream)
        m_stream->requestClose();
}

void EchoService::initialize()
{
    m_connection->createStream(tagBuffer(EchoTag), [=](Stream *stream) {
        this->streamCreated(stream);
    });
}

bool EchoService::hasStream() const
{
    return m_stream != nullptr;
}

void EchoService::send(const QString &string)
{
    if (!m_stream) {
        qCCritical(echoC) << "No valid stream in EchoService when trying to send";
        return;
    }
    StreamPacket packet{string.toUtf8()};
    m_stream->write(packet);
}

void EchoService::close()
{
    m_stream->requestClose();
}

void EchoService::receive(StreamPacket packet)
{
    emit echo(QString{packet.buffer()});
}
