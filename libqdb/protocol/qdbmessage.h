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
#ifndef QDBMESSAGE_H
#define QDBMESSAGE_H

#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>

#include <stdint.h>

QT_BEGIN_NAMESPACE
class QDataStream;
QT_END_NAMESPACE

using StreamId = uint32_t;

class QdbMessage
{
public:
    /*! Get the amount of bytes of data in the message payload from the message header. */
    static int GetDataSize(const QByteArray &header);

    enum CommandType : uint32_t
    {
        Invalid = 0, // never sent
        Connect = 0x434e584e, // CNXN
        Refuse =  0x52465345, // RFSE
        Open = 0x4f50454e, // OPEN
        Write = 0x57525445, // WRTE
        Close = 0x434c5345, // CLSE
        Ok = 0x4f4b4159, // OKAY
    };

    QdbMessage();
    QdbMessage(CommandType command, StreamId hostStream, StreamId deviceStream);
    QdbMessage(CommandType command, StreamId hostStream, StreamId deviceStream, QByteArray data);
    QdbMessage(CommandType command, StreamId hostStream, StreamId deviceStream, const char *data, int length);

    CommandType command() const;
    void setCommand(CommandType command);

    StreamId hostStream() const;
    void setHostStream(StreamId hostStream);

    StreamId deviceStream() const;
    void setDeviceStream(StreamId deviceStream);

    const QByteArray &data() const;
    void setData(const QByteArray &data);
    void setData(const char *data, int length);

private:
    CommandType m_command;
    StreamId m_hostStream;
    StreamId m_deviceStream;
    QByteArray m_data;
};
Q_DECLARE_METATYPE(QdbMessage::CommandType)

QT_BEGIN_NAMESPACE
QDebug &operator<<(QDebug &stream, ::QdbMessage::CommandType command);
QDebug &operator<<(QDebug &stream, const ::QdbMessage &message);

QDataStream &operator<<(QDataStream &stream, const ::QdbMessage &message);
QDataStream &operator>>(QDataStream &stream, ::QdbMessage &message);
QT_END_NAMESPACE

#endif // QDBMESSAGE_H
