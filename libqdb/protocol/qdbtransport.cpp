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
#include "qdbtransport.h"

#include "libqdb/protocol/protocol.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtCore/QIODevice>
#include <QtCore/qloggingcategory.h>

Q_LOGGING_CATEGORY(transportC, "qdb.transport");

QdbTransport::QdbTransport(QIODevice *io)
    : m_io{io}
{

}

QdbTransport::~QdbTransport()
{

}

bool QdbTransport::open()
{
    connect(m_io.get(), &QIODevice::readyRead, this, &QdbTransport::messageAvailable, Qt::QueuedConnection);
    return m_io->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
}

bool QdbTransport::send(const QdbMessage &message)
{
    int messageSize = qdbHeaderSize + message.data().size();
    QByteArray buf{messageSize, '\0'};
    QDataStream stream{&buf, QIODevice::WriteOnly};
    stream << message;

    int count = m_io->write(buf.constData(), messageSize);
    if (count != messageSize) {
        qCCritical(transportC) << "Could not write entire message of" << messageSize << "bytes, only wrote" << count;
        return false;
    }

    qCDebug(transportC) << "TX:" << message;
    return true;
}

QdbMessage QdbTransport::receive()
{
    QByteArray buf{qdbMessageSize, '\0'};
    int count = m_io->read(buf.data(), buf.size());
    if (count < qdbHeaderSize) {
        qCCritical(transportC) << "Could only read" << count << "bytes out of package header's" << qdbHeaderSize;
        return QdbMessage{QdbMessage::Invalid, 0, 0};
    }
    QDataStream stream{buf};
    QdbMessage message;
    stream >> message;
    qCDebug(transportC) << "RX:" << message;

    return message;
}
