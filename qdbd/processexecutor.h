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
#ifndef PROCESSEXECUTOR_H
#define PROCESSEXECUTOR_H

#include "executor.h"
class Stream;

#include "QtCore/qprocess.h"
QT_BEGIN_NAMESPACE
class QByteArray;
class QDataStream;
QT_END_NAMESPACE

#include <memory>

class ProcessExecutor : public Executor
{
    Q_OBJECT
public:
    explicit ProcessExecutor(Stream *stream);

public slots:
    void receive(StreamPacket packet) override;

private slots:
    void onStarted();
    void onReadyRead();
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onErrorOccurred(QProcess::ProcessError error);
    void onStreamClosed();

private:
    void startProcess(const QString &command, const QStringList &arguments);
    void writeToProcess(const QByteArray &data);

    Stream *m_stream;
    std::unique_ptr<QProcess> m_process;
};

#endif // PROCESSEXECUTOR_H
