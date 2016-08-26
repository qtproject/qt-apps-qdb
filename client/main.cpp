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
#include "../libqdb/usb/usbconnection.h"
#include "../libqdb/protocol/qdbtransport.h"
#include "connection.h"
#include "filepullservice.h"
#include "filepushservice.h"
#include "handshakeservice.h"
#include "interruptsignalhandler.h"
#include "networkmanagercontrol.h"
#include "processservice.h"

#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qtimer.h>
#include <QtDBus/QDBusObjectPath>

void setupFilePullService(Connection *connection, const QString &sourcePath, const QString &sinkPath)
{
    auto *service = new FilePullService{connection};

    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                     service, &QObject::deleteLater);
    QObject::connect(service, &FilePullService::pulled,
                     []() {
                         qDebug() << "File pull finished.";
                         QCoreApplication::quit();
                     });
    QObject::connect(service, &FilePullService::error, []() {
        qDebug() << "Error while pulling file.";
        QCoreApplication::exit(1);
    });
    QObject::connect(service, &Service::initialized, [=]() {
        service->pull(sourcePath, sinkPath);
    });

    service->initialize();
}

void setupFilePushService(Connection *connection, const QString &sourcePath, const QString &sinkPath)
{
    auto *service = new FilePushService{connection};

    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                     service, &QObject::deleteLater);
    QObject::connect(service, &FilePushService::pushed,
                     []() {
                         qDebug() << "File transfer finished.";
                         QCoreApplication::quit();
                     });
    QObject::connect(service, &FilePushService::error, []() {
        qDebug() << "Error while pushing file.";
        QCoreApplication::exit(1);
    });
    QObject::connect(service, &Service::initialized, [=]() {
        service->push(sourcePath, sinkPath);
    });

    service->initialize();
}

void setupProcessService(Connection *connection, const QString &processName, const QStringList &arguments)
{
    auto *service = new ProcessService{connection};

    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                     service, &QObject::deleteLater);
    QObject::connect(service, &ProcessService::executed,
                     [](int exitCode, QProcess::ExitStatus exitStatus, QString output) {
                         qDebug() << "Process run, exit code:" << exitCode << exitStatus << output;
                         QTimer::singleShot(1, []() { QCoreApplication::quit(); });
                     });
    QObject::connect(service, &ProcessService::executionError, [](QProcess::ProcessError error) {
        qDebug() << "Process not run, error:" << error;
        QTimer::singleShot(1, []() {QCoreApplication::exit(1); });
    });
    QObject::connect(service, &ProcessService::started, []() {
        qDebug() << "Process started";
    });
    QObject::connect(service, &ProcessService::readyRead, [=]() {
        qDebug() << "Process output:" << service->read();
    });
    QObject::connect(service, &Service::initialized, [=]() {
        service->execute(processName, arguments);
    });

    service->initialize();
}

void configureUsbNetwork(const QString &serial, const QString &macAddress)
{
    qDebug() << "Configuring network for" << serial << "at" << macAddress;
    NetworkManagerControl networkManager;
    auto deviceResult = networkManager.findNetworkDeviceByMac(macAddress);
    if (!deviceResult.isValid()) {
        qWarning() << "Could not find network device" << macAddress;
        return;
    } else {
        const auto networkCard = deviceResult.toString();
        if (networkManager.isActivated(networkCard)) {
            qDebug() << networkCard << "is activated";
            if (networkManager.isDeviceUsingLinkLocal(networkCard)) {
                qInfo() << networkCard << "is already using a link-local IP";
                return;
            }
        }
        if (!networkManager.activateOrCreateConnection(QDBusObjectPath{networkCard}, serial, macAddress))
            qWarning() << "Could not setup network settings for the USB Ethernet interface";
    }
}

void setupNetworkConfiguration(Connection *connection)
{
    auto *service = new HandshakeService{connection};

    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                     service, &QObject::deleteLater);
    QObject::connect(service, &HandshakeService::response,
                     [](QString serial, QString mac) {
                         qDebug() << "Device serial:" << serial;
                         qDebug() << "Host-side MAC address:" << mac;
                         configureUsbNetwork(serial, mac);
                         QCoreApplication::quit();
                     });
    QObject::connect(service, &Service::initialized, [=]() {
        service->ask();
    });

    service->initialize();
}

void setupHandshakeService(Connection *connection)
{
    auto *service = new HandshakeService{connection};

    QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                     service, &QObject::deleteLater);
    QObject::connect(service, &HandshakeService::response,
                     [](QString serial, QString mac) {
                         qDebug() << "Device serial:" << serial;
                         qDebug() << "Host-side MAC address:" << mac;
                         QCoreApplication::quit();
                     });
    QObject::connect(service, &Service::initialized, [=]() {
        service->ask();
    });

    service->initialize();
}

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};

    InterruptSignalHandler signalHandler;

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({{"d","debug-transport"}, "Print each message that is sent."});
    parser.addOption({"debug-connection", "Show enqueued messages"});
    parser.addPositionalArgument("command",
                                 "Subcommand of qdb to run. Possible commands are: "
                                    "run, push, pull");
    parser.process(app);

    QString filterRules;
    if (!parser.isSet("debug-transport"))
        filterRules.append("transport=false\n");
    if (!parser.isSet("debug-connection"))
        filterRules.append("connection=false\n");
    QLoggingCategory::setFilterRules(filterRules);

    Connection connection{new QdbTransport{new UsbConnection{}}};
    if (!connection.initialize()) {
        qDebug() << "could not initialize Connection";
        return 1;
    }

    QObject::connect(&signalHandler, &InterruptSignalHandler::interrupted, [&]() {
        connection.close();
        QCoreApplication::exit(130);
    });

    connection.connect();
    qDebug() << "initialized connection";

    QStringList arguments = parser.positionalArguments();
    if (arguments.size() < 1)
        return 0;

    QString command = arguments[0];
    if (command == "run") {
        Q_ASSERT(arguments.size() >= 2);
        setupProcessService(&connection, arguments[1], arguments.mid(2));
    } else if (command == "push") {
        Q_ASSERT(arguments.size() == 3);
        setupFilePushService(&connection, arguments[1], arguments[2]);
    } else if (command == "pull") {
        Q_ASSERT(arguments.size() == 3);
        setupFilePullService(&connection, arguments[1], arguments[2]);
    } else if (command == "network") {
        if (arguments.size() == 1) {
            setupNetworkConfiguration(&connection);
        } else {
            Q_ASSERT(arguments.size() == 2);
            const auto macAddress = arguments[1];
            configureUsbNetwork(QString{"B2Qt device at %1"}.arg(macAddress), macAddress);
            QTimer::singleShot(1, []() { QCoreApplication::quit(); });
        }
    } else if (command == "handshake") {
        setupHandshakeService(&connection);
    } else {
        qDebug() << "Unrecognized command:" << command;
        return 1;
    }

    return app.exec();
}
