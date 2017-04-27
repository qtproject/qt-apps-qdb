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

#include "../libqdb/protocol/protocol.h"
#include "../libqdb/protocol/qdbmessage.h"

class tst_QdbMessage : public QObject
{
    Q_OBJECT
private slots:
    void construction_data();
    void construction();
    void setters();
    void roundtrip_data();
    void roundtrip();
    void gettingSize_data();
    void gettingSize();
};

void testData()
{
    QTest::addColumn<QdbMessage::CommandType>("command");
    QTest::addColumn<StreamId>("hostStream");
    QTest::addColumn<StreamId>("deviceStream");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("dataSize");

    QTest::newRow("connect") << QdbMessage::Connect << 0u << 0u << QByteArray() << 0;
    QTest::newRow("open") << QdbMessage::Open << 1u << 0u << QByteArray("\x05\x04\x03\x02\x01") << 5;
    QTest::newRow("write") << QdbMessage::Write << 255u << 254u << QByteArray("\x01\x02\x03") << 3;
    QTest::newRow("close") << QdbMessage::Write << 0u << 1u << QByteArray("1234") << 4;
    QTest::newRow("ok") << QdbMessage::Ok << 3u << 5u << QByteArray("\x0A\x0B\x0C\x0D") << 4;
}

void tst_QdbMessage::construction_data()
{
    testData();
}

void tst_QdbMessage::construction()
{
    QFETCH(QdbMessage::CommandType, command);
    QFETCH(StreamId, hostStream);
    QFETCH(StreamId, deviceStream);

    QdbMessage message{command, hostStream, deviceStream};
    QCOMPARE(message.command(), command);
    QCOMPARE(message.hostStream(), hostStream);
    QCOMPARE(message.deviceStream(), deviceStream);
    QCOMPARE(message.data().isEmpty(), true);

    QFETCH(QByteArray, data);

    message = QdbMessage{command, hostStream, deviceStream, data};
    QCOMPARE(message.command(), command);
    QCOMPARE(message.hostStream(), hostStream);
    QCOMPARE(message.deviceStream(), deviceStream);
    QCOMPARE(message.data(), data);
}

void tst_QdbMessage::setters()
{
    QdbMessage message{QdbMessage::Open, 0, 0};

    message.setHostStream(255u);
    QCOMPARE(message.hostStream(), 255u);

    message.setDeviceStream((12u));
    QCOMPARE(message.deviceStream(), 12u);

    message.setData("ABCD", 4);
    QCOMPARE(message.data(), QByteArray("ABCD", 4));
}

void tst_QdbMessage::roundtrip_data()
{
    testData();
}

void tst_QdbMessage::roundtrip()
{
    QFETCH(QdbMessage::CommandType, command);
    QFETCH(StreamId, hostStream);
    QFETCH(StreamId, deviceStream);
    QFETCH(QByteArray, data);

    QByteArray buf{qdbMessageSize, '\0'};
    QDataStream writeStream{&buf, QIODevice::WriteOnly};

    QdbMessage message{command, hostStream, deviceStream, data};
    writeStream << message;

    QDataStream readStream{buf};
    QdbMessage readMessage;
    readStream >> readMessage;

    QCOMPARE(readMessage.command(), message.command());
    QCOMPARE(readMessage.hostStream(), message.hostStream());
    QCOMPARE(readMessage.deviceStream(), message.deviceStream());
    QCOMPARE(readMessage.data(), message.data());
}

void tst_QdbMessage::gettingSize_data()
{
    testData();
}

void tst_QdbMessage::gettingSize()
{
    QFETCH(QdbMessage::CommandType, command);
    QFETCH(StreamId, hostStream);
    QFETCH(StreamId, deviceStream);
    QFETCH(QByteArray, data);

    QByteArray buf{qdbMessageSize, '\0'};
    QDataStream writeStream{&buf, QIODevice::WriteOnly};

    QdbMessage message{command, hostStream, deviceStream, data};
    writeStream << message;

    QFETCH(int, dataSize);

    int size = QdbMessage::GetDataSize(buf);

    QCOMPARE(size, dataSize);
}

QTEST_APPLESS_MAIN(tst_QdbMessage)
#include "tst_qdbmessage.moc"
