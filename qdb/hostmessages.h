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
#ifndef HOSTMESSAGES_H
#define HOSTMESSAGES_H

#include <QtCore/qbytearray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>

const int qdbHostMessageVersion = 1;
bool checkHostMessageVersion(const QJsonObject &obj);

enum class RequestType
{
    Unknown = 0,
    Devices,
    WatchDevices,
    StopServer,
    WatchMessages,
    Messages,
    MessagesAndClear,
};

QByteArray createRequest(const RequestType &type);
RequestType requestType(const QJsonObject &obj);
QString requestTypeString(const RequestType &type);

enum class ResponseType
{
    Unknown = 0,
    Devices,
    NewDevice,
    DisconnectedDevice,
    Stopping,
    InvalidRequest,
    UnsupportedVersion,
    Messages,
};

QJsonObject initializeResponse(const ResponseType &type);
ResponseType responseType(const QJsonObject &obj);
QString responseTypeString(const ResponseType &type);
QByteArray serialiseResponse(const QJsonObject &obj);

#endif // HOSTMESSAGES_H
