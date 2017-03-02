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
#include "subnet.h"

#include <QtNetwork/qnetworkinterface.h>

namespace {

std::vector<Subnet> fetchUsedSubnets()
{
    std::vector<Subnet> subnets;
    for (const QNetworkInterface &interface : QNetworkInterface::allInterfaces())
        for (const QNetworkAddressEntry &entry : interface.addressEntries())
            subnets.push_back(Subnet{entry.ip(), entry.prefixLength()});

    return subnets;
}

} // anonymous namespace

std::pair<Subnet, bool> findUnusedSubnet()
{
    const std::vector<Subnet> usedSubnets = fetchUsedSubnets();
    const std::vector<Subnet> candidateSubnets = {{QHostAddress{"172.16.58.1"}, 30},
                                                  {QHostAddress{"172.17.58.1"}, 30},
                                                  {QHostAddress{"172.18.58.1"}, 30},
                                                  {QHostAddress{"172.19.58.1"}, 30},
                                                  {QHostAddress{"172.20.58.1"}, 30},
                                                  {QHostAddress{"172.21.58.1"}, 30},
                                                  {QHostAddress{"172.22.58.1"}, 30},
                                                  {QHostAddress{"172.23.58.1"}, 30},
                                                  {QHostAddress{"172.24.58.1"}, 30},
                                                  {QHostAddress{"172.25.58.1"}, 30},
                                                  {QHostAddress{"172.26.58.1"}, 30},
                                                  {QHostAddress{"172.27.58.1"}, 30},
                                                  {QHostAddress{"172.28.58.1"}, 30},
                                                  {QHostAddress{"172.29.58.1"}, 30},
                                                  {QHostAddress{"172.30.58.1"}, 30},
                                                  {QHostAddress{"172.31.58.1"}, 30},
                                                  {QHostAddress{"192.168.58.1"}, 30},
                                                  {QHostAddress{"10.17.20.1"}, 30}};
    return findUnusedSubnet(candidateSubnets, usedSubnets);
}

std::pair<Subnet, bool> findUnusedSubnet(const std::vector<Subnet> &candidateSubnets,
                                         const std::vector<Subnet> &usedSubnets)
{
    for (const Subnet &candidate : candidateSubnets) {
        auto overlaps = [&candidate](const Subnet &subnet) {
            // Subnets are defined by prefix masks, so they can't overlap partially.
            // If there is overlap, one is a subset of the other.
            return candidate.address.isInSubnet(subnet.address, subnet.prefixLength)
                    || subnet.address.isInSubnet(candidate.address, candidate.prefixLength);
        };
        if (std::none_of(usedSubnets.begin(), usedSubnets.end(), overlaps))
            return std::make_pair(candidate, true);
    }
    return std::make_pair(Subnet{}, false);
}
