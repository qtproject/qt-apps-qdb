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
#ifndef SERVER_H
#define SERVER_H

#include "libqdb/abstractconnection.h"
class Executor;
class QdbMessage;
class QdbTransport;

#include <memory>
#include <unordered_map>

enum class ServerState
{
    Disconnected,
    Connected,
    Waiting
};

class Server : public AbstractConnection
{
    Q_OBJECT
public:
    /*! Server takes ownership of the passed QdbTransport. */
    explicit Server(QdbTransport *transport, QObject *parent = 0);
    ~Server();

    void enqueueMessage(const QdbMessage &message) override;
signals:

public slots:
    void handleMessage() override;

private:
    void processQueue();
    void handleOpen(StreamId hostId, const QByteArray &tag);
    void resetServer(bool hostConnected);
    void handleWrite(const QdbMessage &message);
    void closeStream(StreamId id);
    void checkVersion(const QdbMessage &message);

    ServerState m_state;
    std::unordered_map<StreamId, std::unique_ptr<Executor>> m_executors;
};

#endif // SERVER_H
