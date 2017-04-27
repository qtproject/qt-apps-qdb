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
#include "hostmessages.h"

const QString responseField = "response";
const QString requestField = "request";
const QString versionField = "_version";

void setVersionField(QJsonObject &obj)
{
    obj[versionField] = qdbHostMessageVersion;
}

bool checkHostMessageVersion(const QJsonObject &obj)
{
    return obj[versionField].toInt() == qdbHostMessageVersion;
}

QByteArray createRequest(const RequestType &type)
{
    QJsonObject obj;
    setVersionField(obj);
    obj[requestField] = requestTypeString(type);
    return QJsonDocument{obj}.toJson(QJsonDocument::Compact).append('\n');
}

RequestType requestType(const QJsonObject &obj)
{
    const auto fieldValue = obj[requestField];
    if (fieldValue == requestTypeString(RequestType::Devices))
        return RequestType::Devices;
    if (fieldValue == requestTypeString(RequestType::WatchDevices))
        return RequestType::WatchDevices;
    if (fieldValue == requestTypeString(RequestType::StopServer))
        return RequestType::StopServer;

    return RequestType::Unknown;
}

QString requestTypeString(const RequestType &type)
{
    switch (type) {
    case RequestType::Devices:
        return "devices";
    case RequestType::WatchDevices:
        return "watch-devices";
    case RequestType::StopServer:
        return "stop-server";
    case RequestType::Unknown:
        break;
    }
    qFatal("Tried to use unknown request type as a value in requestTypeString");
}

QJsonObject initializeResponse(const ResponseType &type)
{
    QJsonObject obj;
    setVersionField(obj);
    obj[responseField] = responseTypeString(type);
    return obj;
}

ResponseType responseType(const QJsonObject &obj)
{
    const auto fieldValue = obj[responseField];
    if (fieldValue == responseTypeString(ResponseType::Devices))
        return ResponseType::Devices;
    if (fieldValue == responseTypeString(ResponseType::NewDevice))
        return ResponseType::NewDevice;
    if (fieldValue == responseTypeString(ResponseType::DisconnectedDevice))
        return ResponseType::DisconnectedDevice;
    if (fieldValue == responseTypeString(ResponseType::Stopping))
        return ResponseType::Stopping;
    if (fieldValue == responseTypeString(ResponseType::InvalidRequest))
        return ResponseType::InvalidRequest;
    if (fieldValue == responseTypeString(ResponseType::UnsupportedVersion))
        return ResponseType::UnsupportedVersion;

    return ResponseType::Unknown;
}

QString responseTypeString(const ResponseType &type)
{
    switch (type) {
    case ResponseType::Devices:
        return "devices";
    case ResponseType::NewDevice:
        return "new-device";
    case ResponseType::DisconnectedDevice:
        return "disconnected-device";
    case ResponseType::Stopping:
        return "stopping";
    case ResponseType::InvalidRequest:
        return "invalid-request";
    case ResponseType::UnsupportedVersion:
        return "unsupported-version";
    case ResponseType::Unknown:
        break;
    }
    qFatal("Tried to use unknown response type as a value in responseTypeString");
}

QByteArray serialiseResponse(const QJsonObject &obj)
{
    return QJsonDocument{obj}.toJson(QJsonDocument::Compact).append('\n');
}
