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
#ifndef INTERRUPTSIGNALHANDLER_H
#define INTERRUPTSIGNALHANDLER_H

#include <QObject>
class QSocketNotifier;

#include <memory>

#ifndef Q_OS_UNIX
#   error "InterruptSignalHandler only supports UNIX signals"
#endif

#include <signal.h>

class InterruptSignalHandler : public QObject
{
    Q_OBJECT
public:
    explicit InterruptSignalHandler(QObject *parent = 0);
    ~InterruptSignalHandler();

    static void sigIntHandler(int signalId);

signals:
    void interrupted();

public slots:
    void handleSigInt();

private:
    bool installSigIntHandler();

    static int s_socketPair[2];
    struct sigaction m_oldAction;
    std::unique_ptr<QSocketNotifier> m_socketNotifier;
};

#endif // INTERRUPTSIGNALHANDLER_H
