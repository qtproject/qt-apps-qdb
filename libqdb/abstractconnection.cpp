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
#include "abstractconnection.h"
#include "protocol/qdbtransport.h"


AbstractConnection::AbstractConnection(QdbTransport *transport, QObject *parent)
    : QObject{parent},
      m_transport{transport},
      m_outgoingMessages{},
      m_streams{},
      m_nextStreamId{1} // Start from 1 since stream IDs of 0 have special meaning
{

}

AbstractConnection::~AbstractConnection()
{

}

bool AbstractConnection::initialize()
{
    connect(m_transport.get(), &QdbTransport::messageAvailable, this, &AbstractConnection::handleMessage);
    return m_transport->open();
}
