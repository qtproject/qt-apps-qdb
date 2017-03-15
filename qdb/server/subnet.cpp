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

SubnetReservation findUnusedSubnet()
{
    SubnetPool *pool = SubnetPool::instance();
    const std::vector<Subnet> usedSubnets = fetchUsedSubnets();
    const std::pair<Subnet, bool> result = findUnusedSubnet(pool->candidates(),
                                                            usedSubnets);
    if (!result.second)
        return SubnetReservation{};

    return pool->reserve(result.first);
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

SubnetPool *SubnetPool::instance()
{
    static SubnetPool pool;
    return &pool;
}

std::vector<Subnet> SubnetPool::candidates()
{
    QMutexLocker locker{&m_lock};
    std::vector<Subnet> freeCandidates;
    freeCandidates.reserve(m_candidates.size() - m_reserved.size());
    std::copy_if(m_candidates.begin(), m_candidates.end(),
                 std::back_inserter(freeCandidates),
                 [&](const Subnet &subnet) {
                     return std::find(m_reserved.begin(), m_reserved.end(), subnet)
                             == m_reserved.end();
                 });
    return freeCandidates;
}

SubnetReservation SubnetPool::reserve(const Subnet &subnet)
{
    QMutexLocker locker{&m_lock};

    const auto iter = std::find(m_reserved.begin(), m_reserved.end(), subnet);
    if (iter != m_reserved.end()) // Already reserved
        return SubnetReservation{};

    m_reserved.push_back(subnet);
    return std::make_shared<SubnetReservationImpl>(subnet);
}

void SubnetPool::free(const Subnet &subnet)
{
    QMutexLocker locker{&m_lock};

    const auto iter = std::find(m_reserved.begin(), m_reserved.end(), subnet);
    if (iter != m_reserved.end())
        m_reserved.erase(iter);
}

SubnetPool::SubnetPool()
    : m_lock{},
      m_candidates(),
      m_reserved{}
{
    // MSVC 2013 gave error C2707 when trying to use initializer list for m_candidates
    m_candidates.push_back({QHostAddress{"172.16.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.17.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.18.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.19.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.20.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.21.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.22.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.23.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.24.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.25.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.26.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.27.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.28.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.29.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.30.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"172.31.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"192.168.58.1"}, 30});
    m_candidates.push_back({QHostAddress{"10.17.20.1"}, 30});
}

bool operator==(const Subnet &lhs, const Subnet &rhs)
{
    return lhs.address == rhs.address
            && lhs.prefixLength == rhs.prefixLength;
}

SubnetReservationImpl::SubnetReservationImpl(const Subnet &subnet)
    : m_subnet(subnet) // uniform initialization with {} fails in MSVC 2013 with error C2797
{

}

SubnetReservationImpl::~SubnetReservationImpl()
{
    SubnetPool::instance()->free(m_subnet);
}

Subnet SubnetReservationImpl::subnet() const
{
    return m_subnet;
}
