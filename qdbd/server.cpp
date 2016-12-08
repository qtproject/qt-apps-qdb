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
#include "server.h"

#include "createexecutor.h"
#include "echoexecutor.h"
#include "libqdb/make_unique.h"
#include "libqdb/protocol/qdbmessage.h"
#include "libqdb/protocol/qdbtransport.h"
#include "libqdb/stream.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

#include <algorithm>

Q_LOGGING_CATEGORY(connectionC, "qdb.connection");

Server::Server(QdbTransport *transport, QObject *parent)
    : AbstractConnection{transport, parent},
      m_state{ServerState::Disconnected},
      m_executors{}
{

}

Server::~Server() = default;

void Server::handleMessage()
{
    QdbMessage message = m_transport->receive();

    if (message.command() == QdbMessage::Invalid)
        qFatal("Server received Invalid message, which is not supported");

    if (message.command() == QdbMessage::Refuse)
        qFatal("Server received Refuse message, which is not supported");

    switch (m_state) {
    case ServerState::Disconnected:
        if (message.command() != QdbMessage::Connect) {
            qCWarning(connectionC) << "Server got non-Connect message in Disconnected state. Refusing.";
            refuse(RefuseReason::NotConnected);
            break;
        }
        handleConnect(message.data());
        break;
    case ServerState::Connected:
        switch (message.command()) {
        case QdbMessage::Connect:
            qCWarning(connectionC) << "Server received QdbMessage::Connect while already connected. Resetting.";
            handleConnect(message.data());
            break;
        case QdbMessage::Open:
            handleOpen(message.hostStream(), message.data());
            break;
        case QdbMessage::Write:
            handleWrite(message);
            break;
        case QdbMessage::Close:
            closeStream(message.deviceStream());
            break;
        case QdbMessage::Ok:
            qCWarning(connectionC) << "Server received QdbMessage::Ok in connected state";
            break;
        case QdbMessage::Refuse:
            //[[fallthrough]]
        case QdbMessage::Invalid:
            Q_UNREACHABLE();
            break;
        }
        break;
    case ServerState::Waiting:
        switch (message.command()) {
        case QdbMessage::Connect:
            qCWarning(connectionC) << "Server received QdbMessage::Connect while already connected and waiting. Resetting.";
            handleConnect(message.data());
            break;
        case QdbMessage::Open:
            handleOpen(message.hostStream(), message.data());
            break;
        case QdbMessage::Write:
            handleWrite(message);
            break;
        case QdbMessage::Close:
            m_state = ServerState::Connected;
            closeStream(message.deviceStream());
            break;
        case QdbMessage::Ok:
            m_state = ServerState::Connected;
            break;
        case QdbMessage::Refuse:
            //[[fallthrough]]
        case QdbMessage::Invalid:
            Q_UNREACHABLE();
            break;
        }
        break;
    }
    processQueue();
}

void Server::enqueueMessage(const QdbMessage &message)
{
    Q_ASSERT(message.command() != QdbMessage::Invalid);
    qCDebug(connectionC) << "Server enqueue: " << message;
    m_outgoingMessages.enqueue(message);
    processQueue();
}

void Server::processQueue()
{
    if (m_outgoingMessages.isEmpty())
        return;

    if (m_state == ServerState::Waiting) {
        qCDebug(connectionC) << "Server::processQueue() skipping to wait for QdbMessage::Ok";
        return;
    }

    auto message = m_outgoingMessages.dequeue();

    Q_ASSERT_X(message.command() != QdbMessage::Invalid, "Server::processQueue()",
               "Tried to send invalid message");

    if (!m_transport->send(message)) {
        qCCritical(connectionC) << "Server could not send" << message;
        m_state = ServerState::Disconnected;
        return;
    }

    switch (message.command()) {
    case QdbMessage::Refuse:
        m_state = ServerState::Disconnected;
        break;
    case QdbMessage::Open:
        qFatal("Server sending QdbMessage::Open is not supported");
        break;
    case QdbMessage::Write:
        Q_ASSERT(m_state == ServerState::Connected);
        m_state = ServerState::Waiting;
        break;
        // Connect, Close and Ok are not acknowledged when sent by server,
        // so no need to transition to ServerState::Waiting.
        // Since there is no acknowledgment incoming, we also need to move
        // onto the next message in the queue, otherwise it would only be sent
        // after host sends us something.
    case QdbMessage::Connect:
        //[[fallthrough]]
    case QdbMessage::Close:
        //[[fallthrough]]
    case QdbMessage::Ok:
        if (!m_outgoingMessages.isEmpty()) {
            processQueue();
        }
        break;
    case QdbMessage::Invalid:
        Q_UNREACHABLE();
        break;
    }
}

void Server::handleConnect(const QByteArray &payload)
{
    if (!checkVersion(payload)) {
        refuse(RefuseReason::UnknownVersion);
        return;
    }

    resetServer();
    m_state = ServerState::Connected;

    QByteArray buffer{};
    QDataStream dataStream{&buffer, QIODevice::WriteOnly};
    dataStream << qdbProtocolVersion;

    enqueueMessage(QdbMessage{QdbMessage::Connect, 0, 0, buffer});
}

void Server::handleOpen(StreamId hostId, const QByteArray &tag)
{
    StreamId deviceId = m_nextStreamId++;
    enqueueMessage(QdbMessage{QdbMessage::Ok, hostId, deviceId});
    m_streams[deviceId] = make_unique<Stream>(this, hostId, deviceId);
    m_executors[deviceId] = createExecutor(m_streams[deviceId].get(), tag);
}

void Server::refuse(RefuseReason reason)
{
    resetServer();
    QByteArray buffer{};
    QDataStream stream{&buffer, QIODevice::WriteOnly};
    stream << static_cast<uint32_t>(reason);

    if (reason == RefuseReason::UnknownVersion)
        stream << qdbProtocolVersion;

    m_state = ServerState::Disconnected;
    enqueueMessage(QdbMessage{QdbMessage::Refuse, 0, 0, buffer});
}

void Server::resetServer()
{
    m_outgoingMessages.clear();
    m_executors.clear();
    m_streams.clear();
}

void Server::handleWrite(const QdbMessage &message)
{
    if (m_streams.find(message.deviceStream()) == m_streams.end()) {
        qCWarning(connectionC) << "Server received message to non-existing stream" << message.deviceStream();
        enqueueMessage(QdbMessage{QdbMessage::Close, message.hostStream(), message.deviceStream()});
        return;
    }
    enqueueMessage(QdbMessage{QdbMessage::Ok, message.hostStream(), message.deviceStream()});
    m_streams[message.deviceStream()]->receiveMessage(message);
}

void Server::closeStream(StreamId id)
{
    if (m_streams.find(id) == m_streams.end()) {
        qCWarning(connectionC) << "Server received Close to a non-existing stream" << id;
        return;
    }

    m_streams[id]->close();
    m_executors.erase(id);
    m_streams.erase(id);

    auto messageInStream = [&id](const QdbMessage &message) {
        return message.deviceStream() == id;
    };
    m_outgoingMessages.erase(
                std::remove_if(m_outgoingMessages.begin(), m_outgoingMessages.end(), messageInStream),
                m_outgoingMessages.end());
    // Closes are not acknowledged
}

bool Server::checkVersion(const QByteArray &payload)
{
    if (static_cast<size_t>(payload.size()) < sizeof(qdbProtocolVersion)) {
       qCCritical(connectionC) << "Connection request did not contain a protocol version";
       return false;
    };

    QDataStream dataStream{payload};
    uint32_t protocolVersion;
    dataStream >> protocolVersion;

    if (protocolVersion != qdbProtocolVersion) {
        qCWarning(connectionC) << "Protocol version" << protocolVersion << "requested, but only version"
                               << qdbProtocolVersion << "is known";
        return false;
    }
    return true;
}

