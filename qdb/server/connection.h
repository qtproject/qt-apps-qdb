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
#ifndef CONNECTION_H
#define CONNECTION_H

#include "libqdb/abstractconnection.h"
class Service;
class QdbMessage;
class QdbTransport;

#include <QtCore/qhash.h>

#include <functional>
#include <memory>
#include <unordered_map>

using StreamCreatedCallback = std::function<void(Stream *)>;

enum class ConnectionState
{
    Disconnected,
    WaitingForConnection,
    Connected,
    Waiting
};

class Connection : public AbstractConnection
{
    Q_OBJECT
public:
    /*! Server takes ownership of the passed QdbTransport. */
    explicit Connection(QdbTransport *transport, QObject *parent = 0);
    ~Connection();

    void connect();
    ConnectionState state() const;
    void createStream(const QByteArray &openTag, StreamCreatedCallback streamCreatedCallback);

    void enqueueMessage(const QdbMessage &message) override;

public slots:
    void close();
    void handleMessage() override;

private:
    void acknowledge(StreamId hostId, StreamId deviceId);
    void processQueue();
    void resetConnection(bool reconnect);
    void closeStream(StreamId id);
    void finishCreateStream(StreamId hostId, StreamId deviceId);
    void handleWrite(const QdbMessage &message);
    bool checkVersion(const QdbMessage &message);

    ConnectionState m_state;
    QHash<StreamId, StreamCreatedCallback> m_streamRequests;
    bool m_closing;
};

#endif // CONNECTION_H
