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
#include "libqdb/protocol/protocol.h"
#include "libqdb/protocol/qdbmessage.h"
#include "libqdb/protocol/qdbtransport.h"
#include "libqdb/stream.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

#include <algorithm>

Q_LOGGING_CATEGORY(connectionC, "connection");

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

    Q_ASSERT_X(message.command() != QdbMessage::Invalid, "Server::handleMessage()", "Received invalid message");

    switch (m_state) {
    case ServerState::Disconnected:
        if (message.command() != QdbMessage::Connect) {
            qWarning() << "Server got non-Connect message in Disconnected state. Resetting.";
            resetServer(false);
            break;
        }
        checkVersion(message);
        resetServer(true);
        break;
    case ServerState::Connected:
        switch (message.command()) {
        case QdbMessage::Connect:
            qWarning() << "Server received QdbMessage::Connect while already connected. Resetting.";
            checkVersion(message);
            resetServer(true);
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
            qWarning() << "Server received QdbMessage::Ok in connected state";
            break;
        case QdbMessage::Invalid:
            Q_UNREACHABLE();
            break;
        }
        break;
    case ServerState::Waiting:
        switch (message.command()) {
        case QdbMessage::Connect:
            qWarning() << "Server received QdbMessage::Connect while already connected and waiting. Resetting.";
            resetServer(true);
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
    if (m_outgoingMessages.isEmpty()) {
        return;
    }

    if (m_state == ServerState::Waiting) {
        qCDebug(connectionC) << "Server::processQueue() skipping to wait for QdbMessage::Ok";
        return;
    }

    auto message = m_outgoingMessages.dequeue();

    Q_ASSERT_X(message.command() != QdbMessage::Invalid, "Server::processQueue()",
               "Tried to send invalid message");

    if (!m_transport->send(message)) {
        qCritical() << "Server could not send" << message;
        m_state = ServerState::Disconnected;
        return;
    }

    switch (message.command()) {
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
        [[fallthrough]]
    case QdbMessage::Close:
        [[fallthrough]]
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

void Server::handleOpen(StreamId hostId, const QByteArray &tag)
{
    StreamId deviceId = m_nextStreamId++;
    enqueueMessage(QdbMessage{QdbMessage::Ok, hostId, deviceId});
    m_streams[deviceId] = make_unique<Stream>(this, hostId, deviceId);
    m_executors[deviceId] = createExecutor(m_streams[deviceId].get(), tag);
}

void Server::resetServer(bool hostConnected)
{
    m_outgoingMessages.clear();
    m_executors.clear();
    m_streams.clear();
    m_state = hostConnected ? ServerState::Connected : ServerState::Disconnected;

    QByteArray buffer{};
    QDataStream dataStream{&buffer, QIODevice::WriteOnly};
    dataStream << qdbProtocolVersion;

    enqueueMessage(QdbMessage{QdbMessage::Connect, 0, 0, buffer});
}

void Server::handleWrite(const QdbMessage &message)
{
    if (m_streams.find(message.deviceStream()) == m_streams.end()) {
        qWarning() << "Server received message to non-existing stream" << message.deviceStream();
        enqueueMessage(QdbMessage{QdbMessage::Close, message.hostStream(), message.deviceStream()});
        return;
    }
    enqueueMessage(QdbMessage{QdbMessage::Ok, message.hostStream(), message.deviceStream()});
    m_streams[message.deviceStream()]->receiveMessage(message);
}

void Server::closeStream(StreamId id)
{
    if (m_streams.find(id) == m_streams.end()) {
        qWarning() << "Server received Close to a non-existing stream";
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

void Server::checkVersion(const QdbMessage &message)
{
    Q_ASSERT(message.command() == QdbMessage::Connect);
    Q_ASSERT(message.data().size() == sizeof(qdbProtocolVersion));

    QDataStream dataStream{message.data()};
    uint32_t protocolVersion;
    dataStream >> protocolVersion;

    if (protocolVersion != qdbProtocolVersion) {
        qWarning() << "Protocol version" << protocolVersion << "requested, but only version"
                   << qdbProtocolVersion << "is known";
    }
}

