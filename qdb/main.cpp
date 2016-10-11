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
#include "client/client.h"
#include "libqdb/interruptsignalhandler.h"
#include "server/hostserver.h"

#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcoreapplication.h>

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({"debug-transport", "Print each message that is sent. (Only server process)"});
    parser.addOption({"debug-connection", "Show enqueued messages. (Only server process)"});
    parser.addPositionalArgument("command",
                                 "Subcommand of qdb to run. Possible commands are: "
                                    "devices, server");
    parser.process(app);

    const QStringList arguments = parser.positionalArguments();
    if (arguments.size() < 1)
        parser.showHelp(1);
    const QString command = arguments[0];

    if (command == "devices") {
        return askDevices(app);
    } else if (command == "server") {
        return hostServer(app, parser);
    } else {
        std::cerr << "Unrecognized command: " << qUtf8Printable(command) << std::endl;
        return 1;
    }
}
