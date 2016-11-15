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

enum class RequestType
{
    Devices,
    WatchDevices,
    StopServer,
    Unknown
};

enum class ResponseType
{
    Devices,
    NewDevice,
    DisconnectedDevice,
    Stopping,
    InvalidRequest,
    Unknown
};

inline
QString responseTypeString(const ResponseType &type)
{
    const QString devicesValue = "devices";
    const QString newDeviceValue = "new-device";
    const QString disconnectedDeviceValue = "disconnected-device";
    const QString stoppingValue = "stopping";
    const QString invalidRequestValue = "invalid-request";

    switch (type) {
    case ResponseType::Devices:
        return devicesValue;
    case ResponseType::NewDevice:
        return newDeviceValue;
    case ResponseType::DisconnectedDevice:
        return disconnectedDeviceValue;
    case ResponseType::Stopping:
        return stoppingValue;
    case ResponseType::InvalidRequest:
        return invalidRequestValue;
    case ResponseType::Unknown:
        break;
    }
    qFatal("Tried to use unknown response type as a value in responseTypeString");
}

inline
QString requestTypeString(const RequestType &type)
{
    const QString devicesValue = "devices";
    const QString watchDevicesValue = "watch-devices";
    const QString stopServerValue = "stop-server";

    switch (type) {
    case RequestType::Devices:
        return devicesValue;
    case RequestType::WatchDevices:
        return watchDevicesValue;
    case RequestType::StopServer:
        return stopServerValue;
    case RequestType::Unknown:
        break;
    }
    qFatal("Tried to use unknown request type as a value in requestTypeString");
}

inline
ResponseType responseType(const QJsonObject &obj)
{
    const auto fieldValue = obj["response"];
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

    return ResponseType::Unknown;
}

inline
RequestType requestType(const QJsonObject &obj)
{
    const auto fieldValue = obj["request"];
    if (fieldValue == requestTypeString(RequestType::Devices))
        return RequestType::Devices;
    if (fieldValue == requestTypeString(RequestType::WatchDevices))
        return RequestType::WatchDevices;
    if (fieldValue == requestTypeString(RequestType::StopServer))
        return RequestType::StopServer;

    return RequestType::Unknown;
}

inline
QByteArray createRequest(const RequestType &type)
{
    QJsonObject obj;
    obj["request"] = requestTypeString(type);
    return QJsonDocument{obj}.toJson(QJsonDocument::Compact).append('\n');
}

inline
QJsonObject initializeResponse(const ResponseType &type)
{
    QJsonObject obj;
    obj["response"] = responseTypeString(type);
    return obj;
}

inline
QByteArray serialiseResponse(const QJsonObject &obj)
{
    return QJsonDocument{obj}.toJson(QJsonDocument::Compact).append('\n');
}

#endif // HOSTMESSAGES_H
