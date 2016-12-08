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
};

QJsonObject initializeResponse(const ResponseType &type);
ResponseType responseType(const QJsonObject &obj);
QString responseTypeString(const ResponseType &type);
QByteArray serialiseResponse(const QJsonObject &obj);

#endif // HOSTMESSAGES_H
