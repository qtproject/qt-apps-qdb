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
#ifndef SERVER_H
#define SERVER_H

#include "libqdb/abstractconnection.h"
#include "libqdb/protocol/protocol.h"
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
    void handleConnect(const QByteArray &payload);
    void handleOpen(StreamId hostId, const QByteArray &tag);
    void refuse(RefuseReason reason);
    void resetServer();
    void handleWrite(const QdbMessage &message);
    void closeStream(StreamId id);
    bool checkVersion(const QByteArray &payload);

    ServerState m_state;
    std::unordered_map<StreamId, std::unique_ptr<Executor>> m_executors;
};

#endif // SERVER_H
