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
#ifndef HANDSHAKESERVICE_H
#define HANDSHAKESERVICE_H

#include "service.h"

class Connection;
class Stream;
class StreamPacket;

class HandshakeService : public Service
{
    Q_OBJECT
public:
    explicit HandshakeService(Connection *connection);
    ~HandshakeService();

    void initialize() override;

    bool hasStream() const;
    void ask();
    void close();

signals:
    void response(QString serial, QString macAddress, QString ipAddress);

public slots:
    void receive(StreamPacket packet) override;

protected slots:
    void onStreamClosed() override;

private:
    void handleDisconnected();
    void failedResponse();

    Connection *m_connection;
    bool m_responded;
};

#endif // HANDSHAKESERVICE_H
