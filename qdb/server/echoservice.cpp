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
