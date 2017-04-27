/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Debug Bridge.
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
#ifndef SUBNET_H
#define SUBNET_H

#include <QtCore/qmutex.h>
#include <QtNetwork/qhostaddress.h>

#include <memory>

struct Subnet
{
    QHostAddress address;
    int prefixLength;
};
Q_DECLARE_METATYPE(Subnet)

bool operator==(const Subnet &lhs, const Subnet &rhs);

class SubnetReservationImpl
{
public:
    SubnetReservationImpl(const Subnet &subnet);
    ~SubnetReservationImpl();

    Subnet subnet() const;

private:
    Subnet m_subnet;
};

using SubnetReservation = std::shared_ptr<SubnetReservationImpl>;

class SubnetPool
{
public:
    static SubnetPool *instance();

    std::vector<Subnet> candidates();
    SubnetReservation reserve(const Subnet &subnet);
    void free(const Subnet &subnet);

private:
    SubnetPool();
    SubnetPool(const std::vector<Subnet> &subnet);

    QMutex m_lock;
    std::vector<Subnet> m_candidates;
    std::vector<Subnet> m_reserved;
};

SubnetReservation findUnusedSubnet();
std::pair<Subnet, bool> findUnusedSubnet(const std::vector<Subnet> &candidateSubnets,
                                         const std::vector<Subnet> &usedSubnets);

#endif // SUBNET_H
