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
#ifndef CLIENT_H
#define CLIENT_H

#include <QtNetwork/qlocalsocket.h>
QT_BEGIN_NAMESPACE
class QCommandLineParser;
class QCoreApplication;
QT_END_NAMESPACE

#include <memory>
#include <functional>

int execClient(const QCoreApplication &app, const QString &command, const QCommandLineParser &parser);

class Client : public QObject
{
    Q_OBJECT
public:
    Client();

    void ignoreErrors(bool ignoreErrors);

public slots:
    void askDevices();
    void startServer();
    void stopServer();
    void watchDevices();
    void askMessages();
    void askMessagesAndClear();
    void watchMessages();

private:
    using ConnectedSlot = void (Client::*)();
    using ErrorSlot = std::function<void(QLocalSocket::LocalSocketError)>;

    void handleDevicesConnection();
    void handleErrorWithRetry(QLocalSocket::LocalSocketError error, ConnectedSlot repeatFunction);
    void handleStopConnection();
    void handleStopError(QLocalSocket::LocalSocketError error);
    void handleWatchConnection();
    void handleWatchMessage();
    void handleMessagesConnection();
    void handleMessagesAndClearConnection();
    void handleWatchMessagesConnection();
    void handleMessagesMessage();
    void setupSocketAndConnect(ConnectedSlot handleConnection, ErrorSlot handleError);
    void shutdown(int exitCode);

    std::unique_ptr<QLocalSocket> m_socket;
    bool m_triedToStart;
    bool m_ignoreErrors;
};

#endif // CLIENT_H
