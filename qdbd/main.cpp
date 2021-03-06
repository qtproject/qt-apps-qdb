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
#include "configuration.h"
#include "libqdb/protocol/qdbtransport.h"
#include "usb-gadget/usbgadget.h"
#include "server.h"

#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(QString{"%1, based on Qt %2"}.arg(QDB_VERSION).arg(QT_VERSION_STR));

    const QString ffsKey{"ffs-dir"};
    const QString gadgetKey{"gadget-configfs-dir"};
    const QString networkKey{"network-script"};
    const QString usbEthernetKey{"usb-ethernet-function-name"};

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({"debug-transport", "Show each transmitted message"});
    parser.addOption({"debug-connection", "Show enqueued messages"});
    parser.addOption({ffsKey,
                      "Directory to the USB Function File System endpoints to use",
                      "directory"});
    parser.addOption({gadgetKey,
                      "Location of the configfs gadget configuration (including the gadget name)",
                      "directory"});
    parser.addOption({networkKey,
                      "Script to run for controlling the network between host and device",
                      "script"});
    parser.addOption({usbEthernetKey,
                      "Name of the Function File System function that provides USB Ethernet",
                      "name"});
    parser.process(app);

    if (parser.isSet(ffsKey))
        Configuration::setFunctionFsDir(parser.value(ffsKey));
    if (parser.isSet(gadgetKey))
        Configuration::setGadgetConfigFsDir(parser.value(gadgetKey));
    if (parser.isSet(networkKey))
        Configuration::setNetworkScript(parser.value(networkKey));
    if (parser.isSet(usbEthernetKey))
        Configuration::setUsbEthernetFunctionName(parser.value(usbEthernetKey));

    QString filterRules;
    if (!parser.isSet("debug-transport")) {
        filterRules.append("qdb.transport.debug=false\n");
    }
    if (!parser.isSet("debug-connection")) {
        filterRules.append("qdb.connection.debug=false\n");
    }
    QLoggingCategory::setFilterRules(filterRules);

    Server server{new QdbTransport{new UsbGadget{}}};
    if (server.initialize()) {
        qDebug() << "Initialized device server";
    } else {
        qCritical() << "Could not initialize device server";
        return 1;
    }

    return app.exec();
}
