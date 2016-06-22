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
#ifndef STREAMPACKET_H
#define STREAMPACKET_H

#include <QtCore/qbytearray.h>
#include <QtCore/qdatastream.h>

class StreamPacket
{
public:
    //! Create a writable StreamPacket
    StreamPacket();
    StreamPacket(const StreamPacket &other);
    StreamPacket& operator=(const StreamPacket &rhs) = delete;
    //! Create a readable StreamPacket
    explicit StreamPacket(const QByteArray &data);


    const QByteArray &buffer() const;
    int size() const;

    template<typename T>
    StreamPacket &operator<<(const T &value)
    {
        m_dataStream << value;
        return *this;
    }

    template<typename T>
    StreamPacket &operator>>(T &target)
    {
        m_dataStream >> target;
        return *this;
    }

private:
    QByteArray m_buffer;
    QDataStream m_dataStream;
};
Q_DECLARE_METATYPE(StreamPacket);

#endif // STREAMPACKET_H
