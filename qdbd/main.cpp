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

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({"debug-transport", "Show each transmitted message"});
    parser.addOption({"debug-connection", "Show enqueued messages"});
    parser.addOption({"ffs-dir", "Directory to the USB Function File System endpoints to use", "directory"});
    parser.addOption({"gadget-configfs-dir",
                      "Location of the configfs gadget configuration (including the gadget name)",
                      "directory"});
    parser.addOption({"rndis-function-name", "Name of the Function File System function that provides RNDIS", "name"});
    parser.process(app);

    if (parser.isSet("ffs-dir"))
        Configuration::setFunctionFsDir(parser.value("ffs-dir"));
    if (parser.isSet("gadget-configfs-dir"))
        Configuration::setGadgetConfigFsDir(parser.value("gadget-configfs-dir"));
    if (parser.isSet("rndis-function-name"))
        Configuration::setRndisFunctionName(parser.value("rndis-function-name"));

    QString filterRules;
    if (!parser.isSet("debug-transport")) {
        filterRules.append("transport.debug=false\n");
    }
    if (!parser.isSet("debug-connection")) {
        filterRules.append("connection.debug=false\n");
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
