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
#include "client.h"

#include "hostmessages.h"
#include "libqdb/make_unique.h"
#include "libqdb/qdbconstants.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qprocess.h>
#include <QtCore/qtimer.h>
#include <QtNetwork/qlocalsocket.h>

#include <iostream>

const int startupDelay = 500; // time in ms to wait for host server startup before retrying

void forkHostServer()
{
    QStringList arguments;
    arguments << "server";
    if (!QProcess::startDetached(QCoreApplication::applicationFilePath(), arguments))
        std::cerr << "Could not start QDB host server\n";
}

int execClient(const QCoreApplication &app, const QString &command, const QCommandLineParser &parser)
{
    Client client;
    client.ignoreErrors(parser.isSet("force"));

    if (command == "devices")
        client.askDevices();
    else if (command == "start-server")
        client.startServer();
    else if (command == "stop-server")
        client.stopServer();
    else if (command == "watch-devices")
        client.watchDevices();
    else if (command == "messages")
        client.askMessages();
    else if (command == "messages-and-clear")
        client.askMessagesAndClear();
    else if (command == "watch-messages")
        client.watchMessages();
    else
        qFatal("Unknown command %s in execClient", qUtf8Printable(command));
    return app.exec();
}

Client::Client()
    : m_socket{nullptr},
      m_triedToStart{false},
      m_ignoreErrors{false}
{

}

void Client::ignoreErrors(bool ignoreErrors)
{
    m_ignoreErrors = ignoreErrors;
}

void Client::askDevices()
{
    setupSocketAndConnect(&Client::handleDevicesConnection, std::bind(&Client::handleErrorWithRetry, this, std::placeholders::_1, &Client::askDevices));
}

void Client::startServer()
{
    forkHostServer();
    shutdown(0);
}

void Client::stopServer()
{
    setupSocketAndConnect(&Client::handleStopConnection, std::bind(&Client::handleStopError, this, std::placeholders::_1));
}

void Client::watchDevices()
{
    setupSocketAndConnect(&Client::handleWatchConnection, std::bind(&Client::handleErrorWithRetry, this, std::placeholders::_1, &Client::watchDevices));
}

void Client::askMessages()
{
    setupSocketAndConnect(&Client::handleMessagesConnection, std::bind(&Client::handleErrorWithRetry, this, std::placeholders::_1, &Client::askMessages));
}

void Client::watchMessages()
{
    setupSocketAndConnect(&Client::handleWatchMessagesConnection, std::bind(&Client::handleErrorWithRetry, this, std::placeholders::_1, &Client::watchMessages));
}

void Client::askMessagesAndClear()
{
    setupSocketAndConnect(&Client::handleMessagesAndClearConnection, std::bind(&Client::handleErrorWithRetry, this, std::placeholders::_1, &Client::askMessagesAndClear));
}

void Client::handleMessagesConnection()
{
    m_socket->write(createRequest(RequestType::Messages));
    if (!m_socket->waitForReadyRead()) {
        std::cerr << "Could not read response from QDB host server\n";
        shutdown(1);
        return;
    }

    const auto response = m_socket->readLine();
    const auto document = QJsonDocument::fromJson(response);

    std::cout << document.toJson().data() << std::endl;

    shutdown(0);
}

void Client::handleDevicesConnection()
{
    m_socket->write(createRequest(RequestType::Devices));
    if (!m_socket->waitForReadyRead()) {
        std::cerr << "Could not read response from QDB host server\n";
        shutdown(1);
        return;
    }

    const auto response = m_socket->readLine();
    const auto document = QJsonDocument::fromJson(response);

    std::cout << document.toJson().data() << std::endl;

    shutdown(0);
}

void Client::handleErrorWithRetry(QLocalSocket::LocalSocketError error, ConnectedSlot repeatFunction)
{
    if (error == QLocalSocket::PeerClosedError)
        return;
    if (error != QLocalSocket::ServerNotFoundError &&
            error != QLocalSocket::ConnectionRefusedError) {
        std::cerr << "Unexpected QLocalSocket error:" << qUtf8Printable(m_socket->errorString())
                  << std::endl;
        shutdown(1);
        return;
    }

    if (m_triedToStart) {
        std::cerr << "Could not connect QDB host server even after trying to start it\n";
        shutdown(1);
        return;
    }
    std::cout << "Starting QDB host server\n";
    m_triedToStart = true;
    forkHostServer();
    QTimer::singleShot(startupDelay, this, repeatFunction);
}

void Client::handleStopConnection()
{
    m_socket->write(createRequest(RequestType::StopServer));
    if (!m_socket->waitForReadyRead()) {
        std::cerr << "Could not read response from QDB host server\n";
        shutdown(1);
        return;
    }

    const auto responseBytes = m_socket->readLine();
    const auto response = QJsonDocument::fromJson(responseBytes).object();

    if (responseType(response) == ResponseType::Stopping) {
        std::cout << "Stopped server\n";
        shutdown(0);
    } else {
        std::cerr << "Unexpected response: " << qUtf8Printable(responseBytes) << std::endl;
        shutdown(1);
    }
}

void Client::handleStopError(QLocalSocket::LocalSocketError error)
{
    if (error == QLocalSocket::PeerClosedError)
        return;

    std::cerr << "Could not connect to QDB host server, perhaps no server was running?\n";
    shutdown(1);
}

void Client::handleWatchConnection()
{
    connect(m_socket.get(), &QIODevice::readyRead, this, &Client::handleWatchMessage);
    m_socket->write(createRequest(RequestType::WatchDevices));
}

void Client::handleWatchMessage()
{
    while (m_socket->bytesAvailable() > 0) {
        const auto responseBytes = m_socket->readLine();
        const auto document = QJsonDocument::fromJson(responseBytes);

        std::cout << document.toJson().data() << std::endl;

        const auto type = responseType(document.object());
        if (type != ResponseType::NewDevice && type != ResponseType::DisconnectedDevice) {
            std::cerr << "Shutting down due to unexpected response:" << responseBytes.data();
            shutdown(1);
        }
    }
}

void Client::setupSocketAndConnect(Client::ConnectedSlot handleConnection, Client::ErrorSlot handleError)
{
    m_socket = make_unique<QLocalSocket>();
    connect(m_socket.get(), &QLocalSocket::connected, this, handleConnection);
    connect(m_socket.get(), &QLocalSocket::errorOccurred, this, handleError);
    m_socket->connectToServer(qdbSocketName);
}

void Client::shutdown(int exitCode)
{
    if (m_ignoreErrors)
        exitCode = 0;

    QTimer::singleShot(0, [=]() {
        QCoreApplication::instance()->exit(exitCode);
    });
}

void Client::handleMessagesAndClearConnection()
{
    m_socket->write(createRequest(RequestType::MessagesAndClear));
    if (!m_socket->waitForReadyRead()) {
        std::cerr << "Could not read response from QDB host server\n";
        shutdown(1);
        return;
    }

    const auto response = m_socket->readLine();
    const auto document = QJsonDocument::fromJson(response);

    std::cout << document.toJson().data() << std::endl;

    shutdown(0);
}

void Client::handleWatchMessagesConnection()
{
    connect(m_socket.get(), &QIODevice::readyRead, this, &Client::handleMessagesMessage);
    m_socket->write(createRequest(RequestType::WatchMessages));
}

void Client::handleMessagesMessage()
{
    while (m_socket->bytesAvailable() > 0) {
        const auto responseBytes = m_socket->readLine();
        const auto document = QJsonDocument::fromJson(responseBytes);

        std::cout << document.toJson().data() << std::endl;

        const auto type = responseType(document.object());
        if (type != ResponseType::Messages) {
            std::cerr << "Shutting down due to unexpected response:" << responseBytes.data();
            shutdown(1);
        }
    }
}

