/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
#include "../qdb/server/subnet.h"

#include <QtCore/qstring.h>
#include <QtTest>

using Subnets = std::vector<Subnet>;

bool operator==(const Subnet &lhs, const Subnet &rhs)
{
    return lhs.address == rhs.address
            && lhs.prefixLength == rhs.prefixLength;
}

class tst_Subnet : public QObject
{
    Q_OBJECT

private slots:
    void freeSubnets();
    void freeSubnets_data();
    void usedSubnets();
    void usedSubnets_data();
};

void tst_Subnet::freeSubnets()
{
    QFETCH(Subnets, candidates);
    QFETCH(Subnets, useds);

    const std::pair<Subnet, bool> result = findUnusedSubnet(candidates, useds);
    QCOMPARE(result.second, true);
    QCOMPARE(result.first, candidates[0]);
}

void tst_Subnet::freeSubnets_data()
{
    QTest::addColumn<Subnets>("candidates");
    QTest::addColumn<Subnets>("useds");

    Subnets candidates = {{QHostAddress{"172.31.0.1"}, 30}};
    Subnets subnets = {{QHostAddress{"10.9.7.70"}, 22}};
    QTest::newRow("1") << candidates << subnets;
    subnets = {{QHostAddress{"10.9.7.70"}, 22},
               {QHostAddress{"10.30.7.0"}, 22}};
    QTest::newRow("2") << candidates << subnets;
    candidates = {{QHostAddress{"10.9.7.254"}, 22}};
    subnets = {{QHostAddress{"10.9.8.1"}, 22}};
    QTest::newRow("3") << candidates << subnets;
}

void tst_Subnet::usedSubnets()
{
    QFETCH(Subnets, candidates);
    QFETCH(Subnets, useds);

    const std::pair<Subnet, bool> result = findUnusedSubnet(candidates, useds);
    QCOMPARE(result.second, false);
}

void tst_Subnet::usedSubnets_data()
{
    QTest::addColumn<Subnets>("candidates");
    QTest::addColumn<Subnets>("useds");

    Subnets candidates = {{QHostAddress{"10.9.7.0"}, 30}};
    Subnets subnets = {{QHostAddress{"10.9.4.70"}, 22}};
    QTest::newRow("1") << candidates << subnets;
    candidates = {{QHostAddress{"10.0.0.0"}, 8}};
    subnets = {{QHostAddress{"10.200.100.1"}, 24}};
    QTest::newRow("2") << candidates << subnets;
    candidates = {{QHostAddress{"10.9.4.70"}, 22}};
    subnets = {{QHostAddress{"192.168.12.1"}, 24},
               {QHostAddress{"172.16.45.1"}, 24},
               {QHostAddress{"10.9.0.1"}, 16}};
    QTest::newRow("3") << candidates << subnets;
}

QTEST_APPLESS_MAIN(tst_Subnet)

#include "tst_subnet.moc"
