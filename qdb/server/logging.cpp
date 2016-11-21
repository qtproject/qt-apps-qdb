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
#include "logging.h"

#include <QtCore/qdatetime.h>
#include <QtCore/qdebug.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qstandardpaths.h>
#include <QtGlobal>

static QFile logFile;

void hostServerMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (!logFile.isWritable()) {
        if (!logFile.open(QFile::WriteOnly | QIODevice::Unbuffered)) {
            // Fall back to default handler
            qInstallMessageHandler(nullptr);
            qCritical() << "Could not open log file" << logFile.fileName();
            return;
        } else {
            QString startMessage{"-- Starting QDB host server log on %1 --\n"};
            startMessage = startMessage.arg(QDateTime::currentDateTime().toString(Qt::ISODate));
            logFile.write(startMessage.toUtf8());
        }
    }

    QString prefix;
    switch (type) {
    case QtDebugMsg:
        prefix = "D:";
        break;
    case QtInfoMsg:
        prefix = "I:";
        break;
    case QtWarningMsg:
        prefix = "W:";
        break;
    case QtCriticalMsg:
        prefix = "C:";
        break;
    case QtFatalMsg:
        prefix = "F:";
        break;
    }

    const auto message = qFormatLogMessage(type, context, msg);
    const auto fullMsg = QString{"%1 %2\n"}.arg(prefix).arg(message).toUtf8();
    auto written = logFile.write(fullMsg);
    if (written != fullMsg.size()) {
        qInstallMessageHandler(nullptr);
        qCritical() << "Could not write into log file" << logFile.fileName()
                    << ":" << logFile.errorString();
    }

    if (type == QtFatalMsg)
        abort();
}

void setupLogging()
{
    if (qgetenv("QDB_LOGGING_TO_CONSOLE") != "1") {
        auto dataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (!dataLocation.isEmpty()) {
            QDir dataDir{dataLocation};
            bool dirAvailable;
            if (dataDir.exists())
                dirAvailable = true;
            else
                dirAvailable = dataDir.mkpath(".");

            if (dirAvailable) {
                logFile.setFileName(dataLocation + "/qdb.log");
                qInstallMessageHandler(hostServerMessageHandler);
            } else {
                qWarning() << "Application data location" << dataLocation
                           << "was not possible to log in, logging to console";
            }
        } else {
            qWarning() << "Could not find writable application data location, logging to console";
        }
    }
}
