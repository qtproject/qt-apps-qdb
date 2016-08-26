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
#include "networkmanagercontrol.h"

#include <QtDBus>

#include <algorithm>

using SettingsMap = QMap<QString, QMap<QString, QDBusVariant>>;

const QString networkManagerServiceName = "org.freedesktop.NetworkManager";
const QString networkManagerObjectPath = "/org/freedesktop/NetworkManager";
const QString networkManagerInterfaceName = "org.freedesktop.NetworkManager";

QVariant connectionSettings(QDBusConnection &bus, const QDBusObjectPath &connectionPath);
SettingsMap demarshallSettings(const QDBusArgument &settingsArgument);

QVariant connectionMac(QDBusConnection &bus, const QDBusObjectPath &connectionPath)
{
    const auto result = connectionSettings(bus, connectionPath);

    if (!result.isValid()) {
        qWarning() << "Could not get MAC address for" << connectionPath.path();
        return QVariant{};
    }
    const auto settings = result.value<SettingsMap>();

    return settings["802-3-ethernet"]["mac-address"].variant();
}

QVariant connectionSettings(QDBusConnection &bus, const QDBusObjectPath &connectionPath)
{
    if (!bus.isConnected()) {
        qWarning() << "Could not connect to D-Bus system bus: " << bus.lastError();
        return QVariant{};
    }
    QDBusInterface connectionInterface{networkManagerServiceName,
                                   connectionPath.path(),
                                   networkManagerInterfaceName + ".Settings.Connection",
                                   bus};
    if (!connectionInterface.isValid()) {
        qWarning() << "Could not find NetworkManager Connection:" << connectionPath.path() << connectionInterface.lastError();
        return QVariant{};
    }

    QDBusMessage result = connectionInterface.call("GetSettings");
    if (result.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "Could not get connection settings for" << connectionPath.path() << ":" << result.errorMessage();
        return QVariant{};
    }
    Q_ASSERT(result.arguments().size() == 1);
    auto settingsArgument = result.arguments()[0].value<QDBusArgument>();
    return QVariant::fromValue(demarshallSettings(settingsArgument));
}

QVariant connectionSettingsFromDevice(QDBusConnection &bus, const QString &devicePath)
{
    if (!bus.isConnected()) {
        qWarning() << "Could not connect to D-Bus system bus: " << bus.lastError();
        return QVariant{};
    }
    QDBusInterface deviceInterface{networkManagerServiceName,
                                   devicePath,
                                   networkManagerInterfaceName + ".Device",
                                   bus};
    if (!deviceInterface.isValid()) {
        qWarning() << "Could not find NetworkManager Device:" << devicePath << deviceInterface.lastError();
        return QVariant{};
    }

    QDBusMessage result = deviceInterface.call("GetAppliedConnection", 0u);
    if (result.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "Could not get connection settings for" << devicePath << ":" << result.errorMessage();
        return QVariant{};
    }
    Q_ASSERT(result.arguments().size() == 2);
    const auto settingsArgument = result.arguments()[0].value<QDBusArgument>();
    return QVariant::fromValue(demarshallSettings(settingsArgument));
}

SettingsMap demarshallSettings(const QDBusArgument &settingsArgument)
{
    SettingsMap settings;
    settingsArgument >> settings;
    return settings;
}

QByteArray macAddressToByteArray(const QString &macAddress)
{
    QByteArray addressBytes;
    const auto hexs = macAddress.split(":");
    for (auto hex : hexs) {
        bool ok = false;
        addressBytes.append(static_cast<char>(hex.toInt(&ok, 16)));
        Q_ASSERT_X(ok, "macAddressToByteArray", "Invalid MAC address given");
    }
    return addressBytes;
}

bool settingsUseLinkLocal(const SettingsMap &settings)
{
    const auto method = settings["ipv4"]["method"].variant().toString();
    return method == "link-local";
}

NetworkManagerControl::NetworkManagerControl()
    : m_bus{QDBusConnection::systemBus()}
{
    qDBusRegisterMetaType<QMap<QString, QDBusVariant>>();
    qDBusRegisterMetaType<SettingsMap>();
}

bool NetworkManagerControl::activateOrCreateConnection(const QDBusObjectPath &devicePath, const QString &serial, const QString &macAddress)
{
    qDebug() << "Activating or creating a connection";
    const auto connectionsResult = findConnectionsByMac(macAddress);
    if (!connectionsResult.isValid()) {
        qWarning() << "Could not list NetworkManager connections";
        return false;
    }
    const auto connections = connectionsResult.value<QList<QDBusObjectPath>>();

    const auto it = std::find_if(connections.begin(), connections.end(), [this](const QDBusObjectPath path) {
        return this->isConnectionUsingLinkLocal(path.path());
    });

    QDBusObjectPath connectionPath;
    if (it != connections.end()) {
        qDebug() << "Found existing connection:" << it->path();
        connectionPath = *it;
    } else {
        qDebug() << "Creating new connection";
        const auto result = createConnection(serial, macAddress);
        if (!result.isValid()) {
            qWarning() << "Could not create a NetworkManager connection for" << macAddress;
            return false;
        }
        connectionPath = result.value<QDBusObjectPath>();
        qDebug() << "New connection:" << connectionPath.path();
    }

    QDBusInterface networkManagerInterface{networkManagerServiceName,
                                           networkManagerObjectPath,
                                           networkManagerInterfaceName,
                                           m_bus};

    if (!networkManagerInterface.isValid()) {
        qWarning() << "Could not find NetworkManager D-Bus interface:"
                   << networkManagerInterface.lastError();
        return false;
    }

    const auto response = networkManagerInterface.call("ActivateConnection",
                                                       QVariant::fromValue(connectionPath),
                                                       QVariant::fromValue(devicePath),
                                                       QVariant::fromValue(QDBusObjectPath{"/"}));

    if (response.type() != QDBusMessage::ReplyMessage)
        return false;
    qDebug() << "Successfully activated" << connectionPath.path();
    return true;
}

QVariant NetworkManagerControl::createConnection(const QString &serial, const QString &macAddress)
{
    /*
     * Connection settings have the D-Bus signature a{sa{sv}}.
     * Example of the desired map structure for a new connection:
     *  {
     *     "connection": {
     *        "id": "B2Qt test connection",
     *        "type": "802-3-ethernet"
     *     },
     *     "802-3-ethernet": {
     *        "mac-address": [106, 157, 79, 239, 108, 104]
     *     },
     *     "ipv4": {
     *        "method": "link-local"
     *     }
     *  }
     */

    QMap<QString, QDBusVariant> connectionMap;
    connectionMap["id"] = QDBusVariant{QString{"%1 via USB"}.arg(serial)};
    connectionMap["type"] = QDBusVariant{QStringLiteral("802-3-ethernet")};

    QMap<QString, QDBusVariant> ethernetMap;
    ethernetMap["mac-address"] = QDBusVariant{macAddressToByteArray(macAddress)};

    QMap<QString, QDBusVariant> ipv4Map;
    ipv4Map["method"] = QDBusVariant{"link-local"};

    SettingsMap settings;
    settings["connection"] = connectionMap;
    settings["802-3-ethernet"] = ethernetMap;
    settings["ipv4"] = ipv4Map;

    if (!m_bus.isConnected()) {
        qWarning() << "Could not connect to D-Bus system bus: " << m_bus.lastError();
        return QVariant{};
    }

    QDBusInterface connectionSettingsInterface{networkManagerServiceName,
                                               networkManagerObjectPath + "/Settings",
                                               networkManagerInterfaceName + ".Settings",
                                               m_bus};

    if (!connectionSettingsInterface.isValid()) {
        qWarning() << "Could not find NetworkManager Settings D-Bus interface:"
                   << connectionSettingsInterface.lastError();
        return QVariant{};
    }

    const QDBusMessage result = connectionSettingsInterface.call("AddConnection",
                                                                 QVariant::fromValue(settings));

    if (result.type() != QDBusMessage::ReplyMessage)
        return QVariant{};
    Q_ASSERT(result.arguments().size() == 1);
    return result.arguments()[0];
}

QVariant NetworkManagerControl::findConnectionsByMac(const QString &macAddress)
{
    if (!m_bus.isConnected()) {
        qWarning() << "Could not connect to D-Bus system bus: " << m_bus.lastError();
        return QVariant{};
    }

    QDBusInterface settingsInterface{networkManagerServiceName,
                                     networkManagerObjectPath + "/Settings",
                                     networkManagerInterfaceName + ".Settings",
                                     m_bus};

    if (!settingsInterface.isValid()) {
        qWarning() << "Could not find NetworkManager D-Bus interface:"
                   << settingsInterface.lastError();
        return QVariant{};
    }

    QVariant result = settingsInterface.property("Connections");
    if (!result.isValid()) {
        qWarning() << "Could not fetch the NetworkManager connections via D-Bus" << settingsInterface.lastError();
        return QVariant{};
    }

    const auto connections = result.value<QList<QDBusObjectPath>>();
    QList<QDBusObjectPath> matchingConnections;
    std::copy_if(connections.begin(), connections.end(), std::back_inserter(matchingConnections),
                 [&](const QDBusObjectPath &path) {
                     const auto result = connectionMac(m_bus, path);
                     if (!result.isValid())
                         return false;
                     const auto mac = result.toByteArray();
                     return macAddressToByteArray(macAddress) == mac;
                 });
    qDebug() << "Existing connections for" << macAddress << ":";
    for (const auto &connection : matchingConnections)
        qDebug() << "    " << connection.path();
    return QVariant::fromValue(matchingConnections);
}

QVariant NetworkManagerControl::findNetworkDeviceByMac(const QString &macAddress)
{
    QVariant result = listNetworkDevices();

    const auto devices = result.value<QList<QDBusObjectPath>>();
    const auto normalizedMac = macAddress.toUpper();
    for (const auto &device : devices) {
        QDBusInterface wiredDeviceInterface{networkManagerServiceName,
                                            device.path(),
                                            networkManagerInterfaceName + ".Device.Wired",
                                            m_bus};
        if (!wiredDeviceInterface.isValid()) {
            qWarning() << "Could not find NetworkManager Device:" << device.path() << wiredDeviceInterface.lastError();
            continue;
        }
        QVariant macResult = wiredDeviceInterface.property("HwAddress");
        if (!macResult.isValid()) {
            const auto error = wiredDeviceInterface.lastError();
            // Non-wired devices result in InvalidArgs due to no MAC address and need not be warned about
            if (error.type() != QDBusError::InvalidArgs)
                qWarning() << "Could not fetch hw address for" << device.path() << error;
            continue;
        }

        if (macResult.toString().toUpper() == normalizedMac) {
            qDebug() << macAddress << "is" << device.path();
            return device.path();
        }
    }
    return QVariant{};
}

bool NetworkManagerControl::isActivated(const QString &devicePath)
{
    if (!m_bus.isConnected()) {
        qWarning() << "Could not connect to D-Bus system bus: " << m_bus.lastError();
        // TODO: separate error value?
        return false;
    }

    QDBusInterface deviceInterface{networkManagerServiceName,
                                   devicePath,
                                   networkManagerInterfaceName + ".Device",
                                   m_bus};
    if (!deviceInterface.isValid()) {
        qWarning() << "Could not find NetworkManager Device:" << devicePath << deviceInterface.lastError();
        // TODO: separate error value?
        return false;
    }

    const QVariant result = deviceInterface.property("State");
    if (!result.isValid()) {
        qWarning() << "Could not check activation status of " << devicePath
                   << "via D-Bus:" << deviceInterface.lastError();
    }
    const auto state = result.toUInt();

    // Should match https://developer.gnome.org/NetworkManager/unstable/nm-dbus-types.html#NMDeviceState
    enum NetworkManagerState {
        NM_DEVICE_STATE_UNKNOWN = 0,
        NM_DEVICE_STATE_UNMANAGED = 10,
        NM_DEVICE_STATE_UNAVAILABLE = 20,
        NM_DEVICE_STATE_DISCONNECTED = 30,
        NM_DEVICE_STATE_PREPARE = 40,
        NM_DEVICE_STATE_CONFIG = 50,
        NM_DEVICE_STATE_NEED_AUTH = 60,
        NM_DEVICE_STATE_IP_CONFIG = 70,
        NM_DEVICE_STATE_IP_CHECK = 80,
        NM_DEVICE_STATE_SECONDARIES = 90,
        NM_DEVICE_STATE_ACTIVATED = 100,
        NM_DEVICE_STATE_DEACTIVATING = 110,
        NM_DEVICE_STATE_FAILED = 120,
    };

    switch (state) {
    case NM_DEVICE_STATE_UNKNOWN:
        return false;
    case NM_DEVICE_STATE_UNMANAGED:
        return false;
    case NM_DEVICE_STATE_UNAVAILABLE:
        return false;
    case NM_DEVICE_STATE_DISCONNECTED:
        return false;
    case NM_DEVICE_STATE_PREPARE:
        return true;
    case NM_DEVICE_STATE_CONFIG:
        return true;
    case NM_DEVICE_STATE_NEED_AUTH:
        // Authentication is not needed for our configuration
        return false;
    case NM_DEVICE_STATE_IP_CONFIG:
        return true;
    case NM_DEVICE_STATE_IP_CHECK:
        return true;
    case NM_DEVICE_STATE_SECONDARIES:
        // Secondary devices are not needed for our configuration
        return false;
    case NM_DEVICE_STATE_ACTIVATED:
        return true;
    case NM_DEVICE_STATE_DEACTIVATING:
        return false;
    case NM_DEVICE_STATE_FAILED:
        return false;
    default:
        qCritical() << "Unrecognized NetworkManager device state" << state;
        return false;
    }
}

bool NetworkManagerControl::isConnectionUsingLinkLocal(const QString &connectionPath)
{
    const QVariant result = connectionSettings(m_bus, QDBusObjectPath{connectionPath});

    if (!result.isValid()) {
        // TODO: correct way to handle failure?
        return false;
    }
    const auto settings = result.value<SettingsMap>();
    return settingsUseLinkLocal(settings);
}

bool NetworkManagerControl::isDeviceUsingLinkLocal(const QString &devicePath)
{
    const QVariant settingsResult = connectionSettingsFromDevice(m_bus, devicePath);

    if (!settingsResult.isValid()) {
        // TODO: correct way to handle failure?
        return false;
    }

    const auto settings = settingsResult.value<SettingsMap>();
    return settingsUseLinkLocal(settings);
}

QVariant NetworkManagerControl::listNetworkDevices()
{
    if (!m_bus.isConnected()) {
        qWarning() << "Could not connect to D-Bus system bus: " << m_bus.lastError();
        return QVariant{};
    }

    QDBusInterface networkManagerInterface{networkManagerServiceName,
                                           networkManagerObjectPath,
                                           networkManagerInterfaceName,
                                           m_bus};

    if (!networkManagerInterface.isValid()) {
        qWarning() << "Could not find NetworkManager D-Bus interface:"
                   << networkManagerInterface.lastError();
        return QVariant{};
    }

    QVariant result = networkManagerInterface.property("Devices");
    if (!result.isValid()) {
        qWarning() << "Could not fetch the NetworkManager devices via D-Bus" << networkManagerInterface.lastError();
    }
    return result;
}
