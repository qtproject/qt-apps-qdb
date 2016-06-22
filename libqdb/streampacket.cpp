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
#include "streampacket.h"

StreamPacket::StreamPacket()
    : m_buffer{},
      m_dataStream{&m_buffer, QIODevice::WriteOnly}
{

}

StreamPacket::StreamPacket(const QByteArray &data)
    : m_buffer{data},
      m_dataStream{m_buffer}
{

}

StreamPacket::StreamPacket(const StreamPacket &other)
    : m_buffer{other.buffer()},
      m_dataStream{m_buffer}
{

}

const QByteArray &StreamPacket::buffer() const
{
    return m_buffer;
}

int StreamPacket::size() const
{
    return m_buffer.size();
}
