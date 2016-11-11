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
#include "client.h"

#include "libqdb/make_unique.h"
#include "libqdb/qdbconstants.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qprocess.h>
#include <QtCore/qtimer.h>
#include <QtNetwork/qlocalsocket.h>

#include <iostream>

void forkHostServer()
{
    QStringList arguments;
    arguments << "server";
    if (!QProcess::startDetached(QCoreApplication::applicationFilePath(), arguments))
        std::cerr << "Could not start QDB host server\n";
}

int execClient(const QCoreApplication &app, const QString &command)
{
    Client client;
    if (command == "devices")
        client.askDevices();
    else if (command == "stop-server")
        client.stopServer();
    else
        qFatal("Unknown command %s in execClient", qUtf8Printable(command));
    return app.exec();
}

Client::Client()
    : m_socket{nullptr},
      m_triedToStart{false}
{

}

void Client::askDevices()
{
    m_socket = make_unique<QLocalSocket>();
    connect(m_socket.get(), &QLocalSocket::connected, this, &Client::handleDevicesConnection);
    connect(m_socket.get(), QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
            this, &Client::handleDevicesError);
    m_socket->connectToServer(qdbSocketName);
}

void Client::stopServer()
{
    m_socket = make_unique<QLocalSocket>();
    connect(m_socket.get(), &QLocalSocket::connected, this, &Client::handleStopConnection);
    connect(m_socket.get(), QOverload<QLocalSocket::LocalSocketError>::of(&QLocalSocket::error),
            this, &Client::handleStopError);
    m_socket->connectToServer(qdbSocketName);
}

void Client::handleDevicesConnection()
{
    m_socket->write("{\"request\":\"devices\"}");
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

void Client::handleDevicesError(QLocalSocket::LocalSocketError error)
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
    QTimer::singleShot(500, this, &Client::askDevices);
}

void Client::handleStopConnection()
{
    m_socket->write("{\"request\":\"stop-server\"}");
    if (!m_socket->waitForReadyRead()) {
        std::cerr << "Could not read response from QDB host server\n";
        shutdown(1);
        return;
    }

    const auto response = m_socket->readLine();
    const auto document = QJsonDocument::fromJson(response);

    if (document.object()["response"] == "stopping") {
        std::cout << "Stopped server\n";
        shutdown(0);
    } else {
        std::cerr << "Unexpected response: " << document.toJson().data() << std::endl;
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

void Client::shutdown(int exitCode)
{
    QTimer::singleShot(0, [=]() {
        QCoreApplication::instance()->exit(exitCode);
    });
}
