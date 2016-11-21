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
#include "stream.h"

#include "abstractconnection.h"
#include "protocol/protocol.h"

#include <QtCore/qdatastream.h>

QByteArray wrapPacket(const StreamPacket &packet)
{
    QByteArray buffer{packet.size() + static_cast<int>(sizeof(quint32)), '\0'};
    QDataStream dataStream{&buffer, QIODevice::WriteOnly};
    dataStream << packet.buffer();
    return buffer;
}

Stream::Stream(AbstractConnection *connection, StreamId hostId, StreamId deviceId)
    : m_connection{connection},
      m_hostId{hostId},
      m_deviceId{deviceId},
      m_partlyReceived{false},
      m_incomingSize{0},
      m_incomingData{}
{

}

bool Stream::write(const StreamPacket &packet)
{
    Q_ASSERT(packet.size() > 0); // writing nothing to the stream does not make sense
    QByteArray data = wrapPacket(packet);
    while (data.size() > 0) {
        const int splitSize = std::min(data.size(), qdbMaxPayloadSize);
        m_connection->enqueueMessage(QdbMessage{QdbMessage::Write, m_hostId, m_deviceId,
                                                data.left(splitSize)});
        data = data.mid(splitSize);
    }
    return true;
}

StreamId Stream::hostId() const
{
    return m_hostId;
}

StreamId Stream::deviceId() const
{
    return m_deviceId;
}

void Stream::requestClose()
{
    m_connection->enqueueMessage(QdbMessage{QdbMessage::Close, m_hostId, m_deviceId});
}

void Stream::close()
{
    emit closed();
}

void Stream::receiveMessage(const QdbMessage &message)
{
    Q_ASSERT(message.command() == QdbMessage::Write);
    Q_ASSERT(message.hostStream() == m_hostId);
    Q_ASSERT(message.deviceStream() == m_deviceId);

    if (m_partlyReceived) {
        const int missing = m_incomingSize - m_incomingData.size() - message.data().size();
        Q_ASSERT_X(missing >= 0, "Stream::receiveMessage", "One QdbMessage must only contain data from a single Stream packet");

        m_incomingData.append(message.data());
        if (missing > 0)
            return;
    } else {
        int packetSize = 0;
        {
            QDataStream dataStream{message.data()};
            Q_ASSERT(message.data().size() >= static_cast<int>(sizeof(uint32_t)));
            uint32_t size;
            dataStream >> size;
            packetSize = int(size);
        }

        const int dataSize = message.data().size() - static_cast<int>(sizeof(uint32_t));
        Q_ASSERT_X(dataSize <= packetSize, "Stream::receiveMessage", "One QdbMessage must only contain data from a single Stream packet");

        m_incomingData = message.data().mid(sizeof(uint32_t));
        if (dataSize < packetSize) {
            m_partlyReceived = true;
            m_incomingSize = packetSize;
            return;
        }
    }

    StreamPacket packet{m_incomingData};

    m_partlyReceived = false;
    m_incomingSize = 0;
    m_incomingData.clear();

    // Emitted last because handling of the signal may lead to closing of stream
    emit packetAvailable(packet);
}
