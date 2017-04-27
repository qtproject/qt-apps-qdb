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
#include "libqdb/protocol/protocol.h"
#include "libqdb/protocol/qdbmessage.h"
#include "libqdb/protocol/qdbtransport.h"
#include "libqdb/protocol/services.h"
#include "qdb/server/usb-host/usbconnection.h"
#include "qdb/server/usb-host/usbdeviceenumerator.h"

#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>
#include <QtTest/QtTest>

const int testTimeout = 500; // in milliseconds

class TestCase : public QObject
{
    Q_OBJECT
public:
    TestCase()
        : m_transport{nullptr}, m_versionBuffer{}, m_phase{0}
    {
        QDataStream dataStream{&m_versionBuffer, QIODevice::WriteOnly};
        dataStream << qdbProtocolVersion;
    }
public slots:
    void run()
    {
        UsbDeviceEnumerator deviceManager;
        const auto devices = deviceManager.listUsbDevices();
        if (devices.empty())
            QFAIL("Could not find QDB USB device to run the test against");

        m_transport = make_unique<QdbTransport>(new UsbConnection{devices[0]});
        if (m_transport->open()) {
            qDebug() << "opened transport";
            connect(m_transport.get(), &QdbTransport::messageAvailable, this, &TestCase::testPhases);
            testPhases();
        } else {
            qDebug() << "failed to open transport";
        }
    }

    virtual void testPhases() = 0;
signals:
    void passed();
protected:
    std::unique_ptr<QdbTransport> m_transport;
    QByteArray m_versionBuffer;
    int m_phase;
};

class OpenWriteCloseEchoTest : public TestCase
{
    Q_OBJECT
public slots:
    void testPhases() override
    {
        switch (m_phase) {
        case 0: {
                QdbMessage cnxn{QdbMessage::Connect, 0, 0, m_versionBuffer};
                QVERIFY(m_transport->send(cnxn));
                break;
            }
        case 1: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Connect);
                QCOMPARE(response.data(), m_versionBuffer);

                QdbMessage open{QdbMessage::Open, m_hostId, 0, tagBuffer(EchoTag)};
                QVERIFY(m_transport->send(open));
                break;
            }
        case 2: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Ok);
                QCOMPARE(response.hostStream(), m_hostId);
                QVERIFY(response.deviceStream() != 0);
                QCOMPARE(response.data(), QByteArray{});

                m_deviceId = response.deviceStream();

                QdbMessage write{QdbMessage::Write, m_hostId, m_deviceId, QByteArray{"\x00\x00\x00\x04""ABCD", 8}};
                QVERIFY(m_transport->send(write));
                break;
            }
        case 3: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Ok);
                QCOMPARE(response.hostStream(), m_hostId);
                QCOMPARE(response.deviceStream(), m_deviceId);
                QCOMPARE(response.data(), QByteArray{});
                break;
            }
        case 4: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Write);
                QCOMPARE(response.hostStream(), m_hostId);
                QCOMPARE(response.deviceStream(), m_deviceId);
                QByteArray data{"\x00\x00\x00\x04""ABCD", 8};
                QCOMPARE(response.data(), data);

                QdbMessage ok{QdbMessage::Ok, m_hostId, m_deviceId};
                QVERIFY(m_transport->send(ok));

                QdbMessage close{QdbMessage::Close, m_hostId, m_deviceId};
                QVERIFY(m_transport->send(close));

                emit passed();
                break;
            }
        }
        ++m_phase;
    }

private:
    const StreamId m_hostId = 1;
    StreamId m_deviceId = 0;
};

class DoubleConnectTest : public TestCase
{
    Q_OBJECT
public slots:
    void testPhases() override
    {
        switch (m_phase) {
        case 0: {
                QdbMessage connect{QdbMessage::Connect, 0, 0, m_versionBuffer};
                QVERIFY(m_transport->send(connect));
                break;
            }
        case 1: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Connect);
                QCOMPARE(response.hostStream(), 0u);
                QCOMPARE(response.deviceStream(), 0u);
                QCOMPARE(response.data(), m_versionBuffer);

                QdbMessage connect{QdbMessage::Connect, 0, 0, m_versionBuffer};
                QVERIFY(m_transport->send(connect));
                break;
            }
        case 2: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Connect);
                QCOMPARE(response.hostStream(), 0u);
                QCOMPARE(response.deviceStream(), 0u);
                QCOMPARE(response.data(), m_versionBuffer);

                emit passed();
                break;
            }
        }
        ++m_phase;
    }
};

class ConnectWithUnsupportedVersionTest : public TestCase
{
    Q_OBJECT
public slots:
    void testPhases() override
    {
        switch (m_phase) {
        case 0: {
                QByteArray unsupported;
                QDataStream dataStream{&unsupported, QIODevice::WriteOnly};
                dataStream << (qdbProtocolVersion + 1);
                QdbMessage connect{QdbMessage::Connect, 0, 0, unsupported};
                QVERIFY(m_transport->send(connect));
                break;
            }
        case 1: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Refuse);
                QCOMPARE(response.hostStream(), 0u);
                QCOMPARE(response.deviceStream(), 0u);
                QDataStream dataStream{response.data()};
                uint32_t reason;
                dataStream >> reason;
                QCOMPARE(reason, static_cast<uint32_t>(RefuseReason::UnknownVersion));
                uint32_t version;
                dataStream >> version;
                QCOMPARE(version, qdbProtocolVersion);

                emit passed();
                break;
            }
        }
        ++m_phase;
    }
};

// Closing a stream twice caused a segmentation fault at one point. This is
// guarding against that regression.
class DoubleCloseTest : public TestCase
{
    Q_OBJECT
public slots:
    void testPhases() override
    {
        switch (m_phase) {
        case 0: {
                QdbMessage cnxn{QdbMessage::Connect, 0, 0, m_versionBuffer};
                QVERIFY(m_transport->send(cnxn));
                break;
            }
        case 1: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Connect);

                QdbMessage open{QdbMessage::Open, m_hostId, 0, tagBuffer(EchoTag)};
                QVERIFY(m_transport->send(open));
                break;
            }
        case 2: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Ok);
                QCOMPARE(response.hostStream(), m_hostId);
                QVERIFY(response.deviceStream() != 0);
                QCOMPARE(response.data(), QByteArray{});

                m_deviceId = response.deviceStream();

                QdbMessage close{QdbMessage::Close, m_hostId, m_deviceId};
                QVERIFY(m_transport->send(close));
                QVERIFY(m_transport->send(close));

                // do a write to check whether qdbd is still alive

                QdbMessage write{QdbMessage::Write, m_hostId, m_deviceId, QByteArray{"\x00\x00\x00\x04""ABCD", 8}};
                QVERIFY(m_transport->send(write));
                break;
            }
        case 3: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Close);
                QCOMPARE(response.hostStream(), m_hostId);
                QCOMPARE(response.deviceStream(), m_deviceId);
                QCOMPARE(response.data(), QByteArray{});

                emit passed();
                break;
            }
        }
        ++m_phase;
    }

private:
    const StreamId m_hostId = 1;
    StreamId m_deviceId = 0;
};

class WriteToNonExistentStreamTest : public TestCase
{
    Q_OBJECT
public slots:
    void testPhases() override
    {
        switch (m_phase) {
        case 0: {
                QdbMessage connect{QdbMessage::Connect, 0, 0, m_versionBuffer};
                QVERIFY(m_transport->send(connect));
                break;
            }
        case 1: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Connect);
                QCOMPARE(response.hostStream(), 0u);
                QCOMPARE(response.deviceStream(), 0u);
                QCOMPARE(response.data(), m_versionBuffer);

                // Try to directly write to an unopened stream
                QdbMessage write{QdbMessage::Write, m_hostId, m_deviceId};
                QVERIFY(m_transport->send(write));
                break;
            }
        case 2: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Close);
                QCOMPARE(response.hostStream(), m_hostId);
                QCOMPARE(response.deviceStream(), m_deviceId);
                QCOMPARE(response.data(), QByteArray{});

                // Open and close a stream and then try to write to it
                QdbMessage open{QdbMessage::Open, m_hostId, 0, tagBuffer(EchoTag)};
                QVERIFY(m_transport->send(open));
                break;
            }
        case 3: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Ok);
                QCOMPARE(response.hostStream(), m_hostId);
                QVERIFY(response.deviceStream() != 0);
                QCOMPARE(response.data(), QByteArray{});

                m_deviceId2 = response.deviceStream();

                QdbMessage close{QdbMessage::Close, m_hostId, m_deviceId2};
                QVERIFY(m_transport->send(close));

                QdbMessage write{QdbMessage::Write, m_hostId, m_deviceId2, QByteArray{"\x00\x00\x00\x04""ABCD", 8}};
                QVERIFY(m_transport->send(write));
                break;
            }
        case 4: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Close);
                QCOMPARE(response.hostStream(), m_hostId);
                // Device stream ID does not matter, since device already closed it
                QCOMPARE(response.data(), QByteArray{});
                emit passed();
                break;
            }
        }
        ++m_phase;
    }
private:
    const StreamId m_hostId = 435;
    const StreamId m_deviceId = 7542;
    StreamId m_deviceId2 = 0;
};

class TwoEchoStreamsTest : public TestCase
{
    Q_OBJECT
public slots:
    void testPhases() override
    {
        switch (m_phase) {
        case 0: {
                QdbMessage cnxn{QdbMessage::Connect, 0, 0, m_versionBuffer};
                QVERIFY(m_transport->send(cnxn));
                break;
            }
        case 1: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Connect);

                QdbMessage open{QdbMessage::Open, m_hostId1, 0, tagBuffer(EchoTag)};
                QVERIFY(m_transport->send(open));
                break;
            }
        case 2: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Ok);
                QCOMPARE(response.hostStream(), m_hostId1);
                QVERIFY(response.deviceStream() != 0);
                QCOMPARE(response.data(), QByteArray{});

                m_deviceId1 = response.deviceStream();

                qDebug() << "writing ABCD";
                QdbMessage write{QdbMessage::Write, m_hostId1, m_deviceId1, QByteArray{"\x00\x00\x00\x04""ABCD", 8}};
                QVERIFY(m_transport->send(write));
                qDebug() << "wrote";
                break;
            }
        case 3: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Ok);
                QCOMPARE(response.hostStream(), m_hostId1);
                QCOMPARE(response.deviceStream(), m_deviceId1);
                QCOMPARE(response.data(), QByteArray{});
                break;
            }
        case 4: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Write);
                QCOMPARE(response.hostStream(), m_hostId1);
                QCOMPARE(response.deviceStream(), m_deviceId1);
                QByteArray data{"\x00\x00\x00\x04""ABCD", 8};
                QCOMPARE(response.data(), data);

                QdbMessage ok{QdbMessage::Ok, m_hostId1, m_deviceId1};
                QVERIFY(m_transport->send(ok));

                QdbMessage open{QdbMessage::Open, m_hostId2, 0, tagBuffer(EchoTag)};
                QVERIFY(m_transport->send(open));
                break;
            }
        case 5: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Ok);
                QCOMPARE(response.hostStream(), m_hostId2);
                QVERIFY(response.deviceStream() != 0);
                QCOMPARE(response.data(), QByteArray{});

                m_deviceId2 = response.deviceStream();

                QdbMessage write{QdbMessage::Write, m_hostId2, m_deviceId2,
                            QByteArray{"\x00\x00\x00\x05\x00\x01\x02\x03\x04", 9}};
                QVERIFY(m_transport->send(write));
                break;
            }
        case 6: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Ok);
                QCOMPARE(response.hostStream(), m_hostId2);
                QCOMPARE(response.deviceStream(), m_deviceId2);
                QCOMPARE(response.data(), QByteArray{});
                break;
            }
        case 7: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Write);
                QCOMPARE(response.hostStream(), m_hostId2);
                QCOMPARE(response.deviceStream(), m_deviceId2);
                auto data = QByteArray{"\x00\x00\x00\x05\x00\x01\x02\x03\x04", 9};
                QCOMPARE(response.data(), data);

                QdbMessage ok{QdbMessage::Ok, m_hostId2, m_deviceId2};
                QVERIFY(m_transport->send(ok));

                QdbMessage close{QdbMessage::Close, m_hostId2, m_deviceId2};
                QVERIFY(m_transport->send(close));

                QdbMessage write{QdbMessage::Write, m_hostId1, m_deviceId1, QByteArray{"\x00\x00\x00\x03QDB", 7}};
                QVERIFY(m_transport->send(write));
                break;
            }
        case 8: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Ok);
                QCOMPARE(response.hostStream(), m_hostId1);
                QCOMPARE(response.deviceStream(), m_deviceId1);
                QCOMPARE(response.data(), QByteArray{});
                break;
            }
        case 9: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Write);
                QCOMPARE(response.hostStream(), m_hostId1);
                QCOMPARE(response.deviceStream(), m_deviceId1);
                QByteArray data{"\x00\x00\x00\x03QDB", 7};
                QCOMPARE(response.data(), data);

                QdbMessage ok{QdbMessage::Ok, m_hostId1, m_deviceId1};
                QVERIFY(m_transport->send(ok));

                QdbMessage close{QdbMessage::Close, m_hostId1, m_deviceId1};
                QVERIFY(m_transport->send(close));

                emit passed();
                break;
            }
        }
        ++m_phase;
    }
private:
    const StreamId m_hostId1 = 1;
    StreamId m_deviceId1 = 0;
    const StreamId m_hostId2 = 2;
    StreamId m_deviceId2 = 0;
};

class WriteWithoutConnectingTest : public TestCase
{
    Q_OBJECT
public slots:
    void testPhases() override
    {
        switch (m_phase) {
        case 0: {
                // Send Connect with unknown version to ensure not connected state.
                // Otherwise the device will still be connected due to previous
                // tests establishing the connection.
                QByteArray unsupported;
                QDataStream dataStream{&unsupported, QIODevice::WriteOnly};
                dataStream << (qdbProtocolVersion + 1);
                QdbMessage connect{QdbMessage::Connect, 0, 0, unsupported};
                QVERIFY(m_transport->send(connect));
                break;
            }
        case 1: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Refuse);

                QByteArray data{"ABCD"};
                QdbMessage write{QdbMessage::Write, 1, 1, data};
                QVERIFY(m_transport->send(write));
                break;
            }
        case 2: {
                QdbMessage response = m_transport->receive();
                QCOMPARE(response.command(), QdbMessage::Refuse);
                QCOMPARE(response.hostStream(), 0u);
                QCOMPARE(response.deviceStream(), 0u);
                QByteArray expectedReason;
                QDataStream dataStream{&expectedReason, QIODevice::WriteOnly};
                dataStream << static_cast<uint32_t>(RefuseReason::NotConnected);
                QCOMPARE(response.data(), expectedReason);

                emit passed();
                break;
            }
        }
        ++m_phase;
    }
};

void testCase(TestCase *test)
{
    QSignalSpy spy{test, &TestCase::passed};
    QTimer::singleShot(0, test, &TestCase::run);
    spy.wait(testTimeout);
    QCOMPARE(spy.count(), 1);
}

class StreamTest : public QObject
{
    Q_OBJECT
private slots:
    void openWriteCloseEcho()
    {
        OpenWriteCloseEchoTest test;
        testCase(&test);
    }

    void doubleConnect()
    {
        DoubleConnectTest test;
        testCase(&test);
    }

    void doubleClose()
    {
        DoubleCloseTest test;
        testCase(&test);
    }

    void connectWithUnsupportedVersion()
    {
        ConnectWithUnsupportedVersionTest test;
        testCase(&test);
    }

    void writeToNonexistentStream()
    {
        WriteToNonExistentStreamTest test;
        testCase(&test);
    }

    void twoEchoStreamsTest()
    {
        TwoEchoStreamsTest test;
        testCase(&test);
    }

    void writeWithoutConnecting()
    {
        WriteWithoutConnectingTest test;
        testCase(&test);
    }
};

QTEST_GUILESS_MAIN(StreamTest)
#include "streamtest.moc"
