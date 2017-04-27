/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of Qt Debug Bridge.
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
class ConnectionContext
{
public:
    ConnectionContext()
        : m_connection{nullptr},
          m_valid{false}
    {
        UsbDeviceEnumerator deviceEnumerator;
        const auto devices = deviceEnumerator.listUsbDevices();
        if (devices.empty())
            QFAIL("Could not find QDB USB device to run the test against");

        m_connection = make_unique<Connection>(new QdbTransport{new UsbConnection{devices[0]}});

        QVERIFY(m_connection->initialize());
        m_valid = true;

        m_connection->connect();
    }

    Connection *connection()
    {
        return m_connection.get();
    }

    bool isValid()
    {
        return m_valid;
    }

private:
    std::unique_ptr<Connection> m_connection;
    bool m_valid;
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
    QVERIFY(ctx.isValid());

    EchoService echo{ctx.connection()};
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
