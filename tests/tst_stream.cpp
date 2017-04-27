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
#include <QtTest/QtTest>

#include "libqdb/abstractconnection.h"
#include "libqdb/stream.h"

class ConnectionStub : public AbstractConnection
{
public:
    ConnectionStub() : AbstractConnection{nullptr} {}

    bool initialize() override
    {
        return true;
    }

    void enqueueMessage(const QdbMessage &message) override
    {
        enqueued.append(message);
    }

    void handleMessage() override
    {

    }

    void reset()
    {
        enqueued.clear();
    }

    QList<QdbMessage> enqueued;
};

class tst_Stream : public QObject
{
    Q_OBJECT
public:
    tst_Stream()
        : m_connection{},
          m_stream{&m_connection, 13, 26},
          m_hostId{13},
          m_deviceId{26}
    { }

private slots:
    void singleWrite();
    void doubleWrite();
    void singleMessagePacket();
    void splitPacket();
    void closedIsEmitted();

private:
    ConnectionStub m_connection;
    Stream m_stream;
    StreamId m_hostId;
    StreamId m_deviceId;
};

void tst_Stream::singleWrite()
{
    m_connection.reset();

    StreamPacket packet{QByteArray{"ABCD"}};
    m_stream.write(packet);

    QCOMPARE(m_connection.enqueued.size(), 1);
    QByteArray expected{"\x00\x00\x00\x04""ABCD", 8};
    QCOMPARE(m_connection.enqueued[0].data(), expected);
}

void tst_Stream::doubleWrite()
{
    m_connection.reset();

    StreamPacket packet{QByteArray{"ABCD"}};
    m_stream.write(packet);
    StreamPacket packet2{QByteArray{"QDB"}};
    m_stream.write(packet2);

    QCOMPARE(m_connection.enqueued.size(), 2);
    QByteArray expected{"\x00\x00\x00\x04""ABCD", 8};
    QCOMPARE(m_connection.enqueued[0].data(), expected);
    expected = QByteArray{"\x00\x00\x00\x03QDB", 7};
    QCOMPARE(m_connection.enqueued[1].data(), expected);
}

void tst_Stream::singleMessagePacket()
{
    QSignalSpy spy{&m_stream, &Stream::packetAvailable};
    m_stream.receiveMessage(QdbMessage{QdbMessage::Write, m_hostId, m_deviceId,
                                       QByteArray{"\x00\x00\x00\x02OK", 6}});

    QCOMPARE(spy.count(), 1);
    QCOMPARE(static_cast<int>(spy[0][0].type()), QMetaType::type("StreamPacket"));
    auto packet = spy[0][0].value<StreamPacket>();
    QCOMPARE(packet.buffer(), QByteArray{"OK"});
}

void tst_Stream::splitPacket()
{
    QSignalSpy spy{&m_stream, &Stream::packetAvailable};
    m_stream.receiveMessage(QdbMessage{QdbMessage::Write, m_hostId, m_deviceId,
                                       QByteArray{"\x00\x00\x00\x05""AB", 6}});
    m_stream.receiveMessage(QdbMessage{QdbMessage::Write, m_hostId, m_deviceId,
                                       QByteArray{"C"}});
    m_stream.receiveMessage(QdbMessage{QdbMessage::Write, m_hostId, m_deviceId,
                                       QByteArray{"DE"}});

    QCOMPARE(spy.count(), 1);
    QCOMPARE(static_cast<int>(spy[0][0].type()), QMetaType::type("StreamPacket"));
    auto packet = spy[0][0].value<StreamPacket>();
    QCOMPARE(packet.buffer(), QByteArray{"ABCDE"});
}

void tst_Stream::closedIsEmitted()
{
    QSignalSpy spy{&m_stream, &Stream::closed};
    m_stream.close();
    QCOMPARE(spy.count(), 1);
}

QTEST_APPLESS_MAIN(tst_Stream)
#include "tst_stream.moc"
