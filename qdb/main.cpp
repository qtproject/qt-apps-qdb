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
#include "client/client.h"
#include "libqdb/interruptsignalhandler.h"
#include "server/hostserver.h"

#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcoreapplication.h>

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app{argc, argv};
    QCoreApplication::setApplicationVersion(QString{"%1, based on Qt %2"}.arg(QDB_VERSION).arg(QT_VERSION_STR));

    QStringList clientCommands = {"devices", "start-server", "stop-server", "watch-devices"};

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({"debug-transport", "Print each message that is sent. (Only server process)"});
    parser.addOption({"debug-connection", "Show enqueued messages. (Only server process)"});
    auto commandList = clientCommands;
    commandList << "server";
    std::sort(commandList.begin(), commandList.end());
    parser.addPositionalArgument("command",
                                 "Subcommand of qdb to run. Possible commands are: "
                                  + commandList.join(", "));
    parser.process(app);

    const QStringList arguments = parser.positionalArguments();
    if (arguments.size() < 1)
        parser.showHelp(1);
    const QString command = arguments[0];

    if (command == "server") {
        return execHostServer(app, parser);
    } else if (clientCommands.contains(command)) {
        return execClient(app, command);
    } else {
        std::cerr << "Unrecognized command: " << qUtf8Printable(command) << std::endl;
        return 1;
    }
}
