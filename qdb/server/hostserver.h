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

#include "libqdb/usb/devicemanagement.h"
#include "deviceinformationfetcher.h"

#include <QtCore/qobject.h>
#include <QtNetwork/qlocalserver.h>
class QCoreApplication;
class QCommandLineParser;

int hostServer(QCoreApplication &app, const QCommandLineParser &parser);

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
    void handleDeviceInformation(DeviceInformation deviceInfo);
    void handleRequest();

private:
    void startFetching();
    void finishFetching();
    void fetchDeviceInformation();

    QLocalServer m_localServer;
    QLocalSocket *m_client; // owned by this class, deleted in handleDisconnection()
    std::vector<UsbDevice> m_devices;
    std::vector<DeviceInformation> m_deviceInfos;
    int m_fetchIndex;
};

#endif // HOSTSERVER_H
