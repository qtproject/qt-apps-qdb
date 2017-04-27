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
#include "deviceinformationfetcher.h"

#include "connection.h"
#include "handshakeservice.h"
#include "libqdb/protocol/qdbtransport.h"
#include "usb-host/usbconnection.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

Q_LOGGING_CATEGORY(deviceInfoC, "qdb.devices.info");

DeviceInformationFetcher::DeviceInformationFetcher(std::shared_ptr<Connection> connection,
                                                   UsbDevice device)
    : m_connection{connection},
      m_device(device) // uniform initialization with {} fails in MSVC 2013 with error C2797
{

}

void DeviceInformationFetcher::fetch()
{
    if (!m_connection || m_connection->state() == ConnectionState::Disconnected) {
        qCWarning(deviceInfoC) << "Not fetching device information due to no connection";
        emit fetched(m_device, Info{"", "", ""});
        return;
    }

    auto *service = new HandshakeService{m_connection.get()};

    connect(this, &DeviceInformationFetcher::fetched,
            service, &QObject::deleteLater);
    connect(service, &HandshakeService::response,
            this, &DeviceInformationFetcher::handshakeResponse);
    connect(service, &Service::initialized, [=]() {
        service->ask();
    });

    service->initialize();
}

void DeviceInformationFetcher::handshakeResponse(QString serial, QString hostMac, QString ipAddress)
{
    qCDebug(deviceInfoC) << "Fetched device information:";
    qCDebug(deviceInfoC) << "    Device serial:" << serial;
    qCDebug(deviceInfoC) << "    Host-side MAC address:" << hostMac;
    qCDebug(deviceInfoC) << "    Device IP address:" << ipAddress;
    emit fetched(m_device, Info{serial, hostMac, ipAddress});
}
