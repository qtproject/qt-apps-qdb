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
#include "qdbmessage.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtEndian>

int QdbMessage::GetDataSize(const QByteArray &header)
{
    const uint32_t sizeFieldBE = *(reinterpret_cast<const uint32_t *>(header.data()) + 3);
    const uint32_t sizeField = qFromBigEndian(sizeFieldBE);

    if (sizeField == 0xFFFFFFFF) {
        // empty QByteArray
        return 0;
    }
    Q_ASSERT(sizeField <= std::numeric_limits<int>::max());
    return sizeField;
}

QdbMessage::QdbMessage()
    : QdbMessage{Invalid, 0, 0, QByteArray{}}
{

}

QdbMessage::QdbMessage(QdbMessage::CommandType command, StreamId hostStream, StreamId deviceStream)
    : QdbMessage{command, hostStream, deviceStream, QByteArray{}}
{

}

QdbMessage::QdbMessage(QdbMessage::CommandType command, StreamId hostStream, StreamId deviceStream, QByteArray data)
    : m_command{command},
      m_hostStream{hostStream},
      m_deviceStream{deviceStream},
      m_data{data}
{

}

QdbMessage::QdbMessage(QdbMessage::CommandType command, StreamId hostStream, StreamId deviceStream, const char *data, int length)
    : QdbMessage{command, hostStream, deviceStream, QByteArray{data, length}}
{

}

QdbMessage::CommandType QdbMessage::command() const
{
    return m_command;
}

void QdbMessage::setCommand(QdbMessage::CommandType command)
{
    m_command = command;
}

StreamId QdbMessage::hostStream() const
{
    return m_hostStream;
}

void QdbMessage::setHostStream(StreamId hostStream)
{
    m_hostStream = hostStream;
}

StreamId QdbMessage::deviceStream() const
{
    return m_deviceStream;
}

void QdbMessage::setDeviceStream(StreamId deviceStream)
{
    m_deviceStream = deviceStream;
}

const QByteArray &QdbMessage::data() const
{
    return m_data;
}

void QdbMessage::setData(const QByteArray &data)
{
    m_data = data;
}

void QdbMessage::setData(const char *data, int length)
{
    m_data = QByteArray{data, length};
}

QdbMessage::CommandType toCommandType(uint32_t command)
{
    switch (command) {
    case static_cast<uint32_t>(QdbMessage::Connect):
        return QdbMessage::Connect;
    case static_cast<uint32_t>(QdbMessage::Open):
        return QdbMessage::Open;
    case static_cast<uint32_t>(QdbMessage::Write):
        return QdbMessage::Write;
    case static_cast<uint32_t>(QdbMessage::Close):
        return QdbMessage::Close;
    case static_cast<uint32_t>(QdbMessage::Ok):
        return QdbMessage::Ok;
    }
    return QdbMessage::Invalid;
}

QT_BEGIN_NAMESPACE

QDataStream &operator<<(QDataStream &stream, const QdbMessage &message)
{
    stream << message.command();
    stream << message.hostStream() << message.deviceStream();
    stream << message.data();

    return stream;
}

QDataStream &operator>>(QDataStream &stream, QdbMessage &message)
{
    uint32_t command;
    stream >> command;
    message.setCommand(toCommandType(command));

    StreamId hostStream;
    stream >> hostStream;
    message.setHostStream(hostStream);

    StreamId deviceStream;
    stream >> deviceStream;
    message.setDeviceStream(deviceStream);

    QByteArray data;
    stream >> data;
    message.setData(data);

    return stream;
}

QDebug &operator<<(QDebug &stream, ::QdbMessage::CommandType command)
{
    switch (command) {
    case QdbMessage::Invalid:
        stream << "Invalid";
        break;
    case QdbMessage::Connect:
        stream << "Connect";
        break;
    case QdbMessage::Open:
        stream << "Open";
        break;
    case QdbMessage::Write:
        stream << "Write";
        break;
    case QdbMessage::Close:
        stream << "Close";
        break;
    case QdbMessage::Ok:
        stream << "Ok";
        break;
    }
    return stream;
}

QDebug &operator<<(QDebug &stream, const QdbMessage &message)
{
    stream << message.command() << message.hostStream() << message.deviceStream() <<
              message.data();

    return stream;
}

QT_END_NAMESPACE
