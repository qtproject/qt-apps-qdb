/******************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
