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
#include "libqdb/make_unique.h"
#include "libqdb/protocol/qdbtransport.h"
#include "libqdb/protocol/services.h"
#include "qdb/server/connection.h"
#include "qdb/server/echoservice.h"
#include "qdb/server/usb-host/usbconnection.h"
#include "qdb/server/usb-host/usbdeviceenumerator.h"

#include <QtCore/qdebug.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qtimer.h>
#include <QtTest/QtTest>

const int testTimeout = 500; // in milliseconds

// Helper to initialize Connection in testcases
struct ConnectionContext
{
    ConnectionContext()
        : deviceEnumerator{},
          connection{new QdbTransport{new UsbConnection{deviceEnumerator.listUsbDevices()[0]}}}
    {
        QVERIFY(connection.initialize());

        connection.connect();
    }
    UsbDeviceEnumerator deviceEnumerator;
    Connection connection;
};

class ServiceTest : public QObject
{
    Q_OBJECT
private slots:
    void echo();
};

void ServiceTest::echo()
{
    ConnectionContext ctx;

    EchoService echo{&ctx.connection};
    connect(&echo, &EchoService::initialized, [&]() {
        echo.send("ABCD");
    });
    QSignalSpy spy{&echo, &EchoService::echo};

    echo.initialize();

    spy.wait(testTimeout);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy[0][0].toByteArray(), QByteArray{"ABCD"});
}

QTEST_GUILESS_MAIN(ServiceTest)
#include "servicetest.moc"
