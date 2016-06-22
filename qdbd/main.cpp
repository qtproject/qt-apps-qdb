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
#include "usb-gadget/usbgadget.h"
#include "server.h"
#include "protocol/qdbtransport.h"

#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({"debug-transport", "Show each transmitted message"});
    parser.addOption({"debug-connection", "Show enqueued messages"});
    parser.process(app);

    QString filterRules;
    if (!parser.isSet("debug-transport")) {
        filterRules.append("transport=false\n");
    }
    if (!parser.isSet("debug-connection")) {
        filterRules.append("connection=false\n");
    }
    QLoggingCategory::setFilterRules(filterRules);

    Server server{new QdbTransport{new UsbGadget{}}};
    if (server.initialize()) {
        qDebug() << "initialized server";
    } else {
        qDebug() << "could not initialize server";
        return 1;
    }

    return app.exec();
}
