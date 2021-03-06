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
#include "../qdb/server/subnet.h"

#include <QtCore/qstring.h>
#include <QtTest>

using Subnets = std::vector<Subnet>;

class tst_Subnet : public QObject
{
    Q_OBJECT

private slots:
    void freeSubnets();
    void freeSubnets_data();
    void usedSubnets();
    void usedSubnets_data();
    void reserveOne();
    void reserveOne_data();
    void reserveSome();
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

void tst_Subnet::reserveOne()
{
    QFETCH(Subnet, subnet);

    SubnetPool *pool = SubnetPool::instance();
    const auto amountOfCandidates = pool->candidates().size();
    QVERIFY(amountOfCandidates >= 1);
    {
        const auto reservation = pool->reserve(subnet);
        const auto candidates = pool->candidates();
        QCOMPARE(candidates.size(), amountOfCandidates - 1);
        QVERIFY(std::none_of(candidates.begin(), candidates.end(),
                             [&](const Subnet &other) {
                                 return subnet == other;
                             }));
    }
    const auto candidates = pool->candidates();
    QCOMPARE(candidates.size(), amountOfCandidates);
    QVERIFY(std::find(candidates.begin(), candidates.end(), subnet) != candidates.end());
}

void tst_Subnet::reserveOne_data()
{
    QTest::addColumn<Subnet>("subnet");
    QTest::newRow("first") << Subnet{QHostAddress{"172.16.58.1"}, 30};
    QTest::newRow("middle") << Subnet{QHostAddress{"172.25.58.1"}, 30};
    QTest::newRow("last") << Subnet{QHostAddress{"10.17.20.1"}, 30};
}

void tst_Subnet::reserveSome()
{
    SubnetPool *pool = SubnetPool::instance();
    const Subnets subnetsToReserve {{QHostAddress{"172.18.58.1"}, 30},
                                    {QHostAddress{"172.19.58.1"}, 30},
                                    {QHostAddress{"172.20.58.1"}, 30}};
    const auto amountOfCandidates = pool->candidates().size();

    std::vector<SubnetReservation> reservations;
    for (const Subnet &subnet : subnetsToReserve)
    {
        reservations.push_back(pool->reserve(subnet));
        const auto candidates = pool->candidates();
        QCOMPARE(candidates.size(), amountOfCandidates - reservations.size());
        QVERIFY(std::find(candidates.begin(), candidates.end(), subnet) == candidates.end());
    }
    const auto candidates = pool->candidates();
    QCOMPARE(candidates.size(), amountOfCandidates - reservations.size());
    QVERIFY(std::none_of(candidates.begin(), candidates.end(),
                         [&](const Subnet &subnet) {
                             return std::find(subnetsToReserve.begin(),
                                              subnetsToReserve.end(),
                                              subnet)
                                     != subnetsToReserve.end();
                         }));

    reservations.clear();

    QCOMPARE(pool->candidates().size(), amountOfCandidates);
}

QTEST_APPLESS_MAIN(tst_Subnet)

#include "tst_subnet.moc"
