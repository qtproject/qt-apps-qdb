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
#ifndef HOSTSERVER_H
#define HOSTSERVER_H

#include "deviceinformationfetcher.h"
#include "devicemanager.h"

#include <QtCore/qobject.h>
#include <QtNetwork/qlocalserver.h>
QT_BEGIN_NAMESPACE
class QCoreApplication;
class QCommandLineParser;
QT_END_NAMESPACE

int execHostServer(const QCoreApplication &app, const QCommandLineParser &parser);

class HostServer : public QObject
{
    Q_OBJECT
public:
    explicit HostServer(QObject *parent = nullptr);
    void listen();

signals:
    void closed();

public slots:
    void close();

private slots:
    void handleConnection();
    void handleDisconnection();
    void handleNewDeviceInfo(DeviceInformation info);
    void handleDisconnectedDevice(QString serial);
    void handleRequest();

private:
    void replyDeviceInformation();
    void stopServer();

    QLocalServer m_localServer;
    QLocalSocket *m_client; // owned by this class, deleted in handleDisconnection()
    DeviceManager m_deviceManager;
};

#endif // HOSTSERVER_H
