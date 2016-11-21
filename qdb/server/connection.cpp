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
#include "connection.h"

#include "libqdb/make_unique.h"
#include "libqdb/protocol/protocol.h"
#include "libqdb/protocol/qdbtransport.h"
#include "service.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

Q_LOGGING_CATEGORY(connectionC, "qdb.connection");

Connection::Connection(QdbTransport *transport, QObject *parent)
    : AbstractConnection{transport, parent},
      m_state{ConnectionState::Disconnected},
      m_streamRequests{},
      m_closing{false}
{

}

void Connection::connect()
{
    Q_ASSERT(m_state == ConnectionState::Disconnected);

    QByteArray versionBuffer{};
    QDataStream dataStream{&versionBuffer, QIODevice::WriteOnly};
    dataStream << qdbProtocolVersion;

    enqueueMessage(QdbMessage{QdbMessage::Connect, 0, 0, versionBuffer});
}

void Connection::close()
{
    if (m_state == ConnectionState::Disconnected)
        return;

    if (m_state == ConnectionState::WaitingForConnection) {
        m_state = ConnectionState::Disconnected;
        return;
    }

    // No need to wait for 'Ok's just to send the final Closes
    if (m_state == ConnectionState::Waiting)
        m_state = ConnectionState::Connected;

    m_outgoingMessages.clear();
    m_streamRequests.clear();

    while (!m_streams.empty()) {
        auto &stream = m_streams.begin()->second;
        enqueueMessage(QdbMessage{QdbMessage::Close, stream->hostId(), stream->deviceId()});
        // Processing of the Close message erased the stream from m_streams
    }
}

ConnectionState Connection::state() const
{
    return m_state;
}

void Connection::createStream(const QByteArray &openTag, StreamCreatedCallback streamCreatedCallback)
{
    StreamId id = m_nextStreamId++;
    m_streamRequests[id] = streamCreatedCallback;
    enqueueMessage(QdbMessage{QdbMessage::Open, id, 0, openTag});
}

Connection::~Connection() = default;

void Connection::enqueueMessage(const QdbMessage &message)
{
    Q_ASSERT(message.command() != QdbMessage::Invalid);
    qCDebug(connectionC) << "Connection enqueue: " << message;
    m_outgoingMessages.enqueue(message);
    processQueue();
}

void Connection::handleMessage()
{
    QdbMessage message = m_transport->receive();

    if (message.command() == QdbMessage::Open)
        qFatal("Connection received QdbMessage::Open, which is not supported!");

    if (message.command() == QdbMessage::Invalid) {
        qCCritical(connectionC) << "Connection received invalid message!";
        resetConnection(false);
        return;
    }

    switch (m_state) {
    case ConnectionState::Disconnected:
        qCWarning(connectionC) << "Connection got a message in Disconnected state";
        resetConnection(true);
        break;
    case ConnectionState::WaitingForConnection:
        if (message.command() != QdbMessage::Connect) {
            qCWarning(connectionC) << "Connection got a non-Connect message in WaitingForConnection state";
            resetConnection(true);
            break;
        }
        if (checkVersion(message))
            m_state = ConnectionState::Connected;
        else
            m_state = ConnectionState::Disconnected;
        break;
    case ConnectionState::Connected:
        switch (message.command()) {
        case QdbMessage::Connect:
            qCWarning(connectionC) << "Connection received QdbMessage::Connect while already connected. Reconnecting.";
            resetConnection(true);
            break;
        case QdbMessage::Write:
            handleWrite(message);
            break;
        case QdbMessage::Close:
            closeStream(message.hostStream());
            break;
        case QdbMessage::Ok:
            qCWarning(connectionC) << "Connection received QdbMessage::Ok in connected state";
            break;
        case QdbMessage::Open:
            //[[fallthrough]]
        case QdbMessage::Invalid:
            Q_UNREACHABLE();
            break;
        }
        break;
    case ConnectionState::Waiting:
        switch (message.command()) {
        case QdbMessage::Connect:
            qCWarning(connectionC) << "Connection received QdbMessage::Connect while already connected and waiting. Reconnecting.";
            resetConnection(true);
            break;
        case QdbMessage::Write:
            handleWrite(message);
            break;
        case QdbMessage::Close:
            closeStream(message.hostStream());
            break;
        case QdbMessage::Ok:
            m_state = ConnectionState::Connected;
            if (m_streamRequests.contains(message.hostStream())) {
                // This message is a response to Open
                finishCreateStream(message.hostStream(), message.deviceStream());
            }
            break;
        case QdbMessage::Open:
            //[[fallthrough]]
        case QdbMessage::Invalid:
            Q_UNREACHABLE();
            break;
        }
        break;
    }
    processQueue();
}

void Connection::acknowledge(StreamId hostId, StreamId deviceId)
{
    // QdbMessage::Ok is sent also in Waiting state to avoid both sides ending up waiting for Ok
    Q_ASSERT(m_state == ConnectionState::Connected || m_state == ConnectionState::Waiting);

    QdbMessage message{QdbMessage::Ok, hostId, deviceId};
    if (!m_transport->send(message)) {
        qCCritical(connectionC) << "Connection could not send" << message;
        resetConnection(false);
        return;
    }
}

void Connection::processQueue()
{
    if (m_outgoingMessages.isEmpty()) {
        return;
    }

    if (m_state == ConnectionState::Waiting) {
        qCDebug(connectionC) << "Connection::processQueue() skipping to wait for QdbMessage::Ok";
        return;
    }

    if (m_state == ConnectionState::WaitingForConnection) {
        qCDebug(connectionC) << "Connection::processQueue() skipping to wait for QdbMessage::Connect";
        return;
    }

    auto message = m_outgoingMessages.dequeue();

    Q_ASSERT_X(message.command() != QdbMessage::Invalid, "Connection::processQueue()",
               "Tried to send invalid message");

    if (!m_transport->send(message)) {
        qCCritical(connectionC) << "Connection could not send" << message;
        resetConnection(false);
        return;
    }

    switch (message.command()) {
    case QdbMessage::Connect:
        Q_ASSERT(m_state == ConnectionState::Disconnected);
        m_state = ConnectionState::WaitingForConnection;
        break;
        // Need to wait for Ok after Open and Write
    case QdbMessage::Open:
        Q_ASSERT(m_state == ConnectionState::Connected);
        m_state = ConnectionState::Waiting;
        break;
    case QdbMessage::Write:
        Q_ASSERT(m_state == ConnectionState::Connected);
        m_state = ConnectionState::Waiting;
        break;
        // Close is not acknowledged so no need to transition to ConnectionState::Waiting
    case QdbMessage::Close:
        Q_ASSERT(m_state == ConnectionState::Connected);
        closeStream(message.hostStream());
        break;
    case QdbMessage::Ok:
        // 'Ok's are sent via acknowledge()
        //[[fallthrough]]
    case QdbMessage::Invalid:
        Q_UNREACHABLE();
        break;
    }
}

void Connection::resetConnection(bool reconnect)
{
    m_outgoingMessages.clear();
    m_state = ConnectionState::Disconnected;
    m_streamRequests.clear();
    m_streams.clear();

    if (reconnect)
        connect();
}

void Connection::closeStream(StreamId id)
{
    if (m_streams.find(id) == m_streams.end())
        return;

    m_streams[id]->close();
    m_streams.erase(id);

    auto messageInStream = [&id](const QdbMessage &message) {
        return message.hostStream() == id;
    };
    m_outgoingMessages.erase(
                std::remove_if(m_outgoingMessages.begin(), m_outgoingMessages.end(), messageInStream),
                m_outgoingMessages.end());
    // Closes are not acknowledged
}

void Connection::finishCreateStream(StreamId hostId, StreamId deviceId)
{
    m_streams[hostId] = make_unique<Stream>(this, hostId, deviceId);
    StreamCreatedCallback callback = m_streamRequests.take(hostId);
    callback(m_streams[hostId].get());
}

void Connection::handleWrite(const QdbMessage &message)
{
    if (m_streams.find(message.hostStream()) == m_streams.end()) {
        qCWarning(connectionC) << "Connection received message to non-existing stream" << message.hostStream();
        enqueueMessage(QdbMessage{QdbMessage::Close, message.hostStream(), message.deviceStream()});
        return;
    }
    acknowledge(message.hostStream(), message.deviceStream());
    m_streams[message.hostStream()]->receiveMessage(message);
}

bool Connection::checkVersion(const QdbMessage &message)
{
    Q_ASSERT(message.command() == QdbMessage::Connect);
    Q_ASSERT(message.data().size() == sizeof(qdbProtocolVersion));

    QDataStream dataStream{message.data()};
    uint32_t protocolVersion;
    dataStream >> protocolVersion;

    if (protocolVersion != qdbProtocolVersion) {
        qCCritical(connectionC) << "Device offered protocol version" << protocolVersion
                                << ", but only version" << qdbProtocolVersion << "is supported";
        return false;
    }
    return true;
}
