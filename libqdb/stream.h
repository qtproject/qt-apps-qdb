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
#ifndef STREAM_H
#define STREAM_H

class AbstractConnection;
#include "protocol/qdbmessage.h"
#include "streampacket.h"

#include <QtCore/qobject.h>

class Stream : public QObject
{
    Q_OBJECT
public:
    Stream(AbstractConnection *connection, StreamId hostId, StreamId deviceId);

    bool write(const StreamPacket &packet);

    StreamId hostId() const;
    StreamId deviceId() const;

    void requestClose();
    // Should only be called by AbstractConnection, use requestClose() instead elsewhere
    void close();
signals:
    void packetAvailable(StreamPacket data);
    void closed();

public slots:
    void receiveMessage(const QdbMessage &message);

private:
    AbstractConnection *m_connection;
    StreamId m_hostId;
    StreamId m_deviceId;
    bool m_partlyReceived;
    int m_incomingSize;
    QByteArray m_incomingData;
};

#endif // STREAM_H
