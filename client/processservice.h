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
#ifndef PROCESSSERVICE_H
#define PROCESSSERVICE_H

class Connection;
class Stream;
#include "service.h"

#include <QtCore/qbytearray.h>
#include <QtCore/qprocess.h>
#include <QtCore/qqueue.h>

class ProcessService : public Service
{
    Q_OBJECT
public:
    explicit ProcessService(Connection *connection);

    void initialize() override;

    bool execute(const QString &process, const QStringList &arguments);
    QByteArray read();
    qint64 write(const QByteArray &data);
signals:
    void executed(int exitCode, QProcess::ExitStatus exitStatus, QByteArray output);
    void executionError(QProcess::ProcessError error);
    void readyRead();
    void started();

public slots:
    void receive(StreamPacket packet) override;

private:
    Connection *m_connection;
    QQueue<QByteArray> m_reads;
};

#endif // PROCESSSERVICE_H
