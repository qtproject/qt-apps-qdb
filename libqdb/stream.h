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
