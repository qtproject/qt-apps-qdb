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
#include "client.h"

#include "libqdb/qdbconstants.h"

#include <QtCore/qcoreapplication.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qtimer.h>
#include <QtNetwork/qlocalsocket.h>

#include <iostream>

int askDevices(QCoreApplication &app)
{
    QLocalSocket socket;
    socket.connectToServer(qdbSocketName);
    if (!socket.waitForConnected()) {
        std::cerr << "Could not connect to QDB host server\n";
        return 1;
    }

    socket.write("{\"request\":\"devices\"}");
    if (!socket.waitForReadyRead()) {
        std::cerr << "Could not read response from QDB host server\n";
        return 1;
    }
    const auto response = socket.readLine();
    const auto document = QJsonDocument::fromJson(response);

    std::cout << "Response: " << document.toJson().data() << std::endl;

    QTimer::singleShot(0, &app, &QCoreApplication::quit);

    return app.exec();
}
