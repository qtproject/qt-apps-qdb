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
#include "interruptsignalhandler.h"

#include "libqdb/make_unique.h"

#include <QtCore/qdebug.h>
#include <QtCore/qsocketnotifier.h>

#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

class InterruptSignalHandlerPrivate : public QObject
{
    Q_OBJECT
public:
    InterruptSignalHandlerPrivate(InterruptSignalHandler *owner);

    static void sigIntHandler(int signalId);

public slots:
    void handleSigInt();

public:
    static int s_socketPair[2];
    struct sigaction oldAction;
    std::unique_ptr<QSocketNotifier> socketNotifier = nullptr;

private:
    InterruptSignalHandler *q;
};

int InterruptSignalHandlerPrivate::s_socketPair[2] = {0, 0};

InterruptSignalHandler::InterruptSignalHandler(QObject *parent)
    : QObject{parent}, d{new InterruptSignalHandlerPrivate{this}}
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, d->s_socketPair))
        qFatal("Could not create socketpair for signal handling in InterruptSignalHandler");

    d->socketNotifier = make_unique<QSocketNotifier>(d->s_socketPair[1], QSocketNotifier::Read);
    connect(d->socketNotifier.get(), &QSocketNotifier::activated, d, &InterruptSignalHandlerPrivate::handleSigInt);

    if (!installSigIntHandler())
        qFatal("Could not install signal handler in InterruptSignalHandler");
}

InterruptSignalHandler::~InterruptSignalHandler()
{
    delete d;
}

bool InterruptSignalHandler::installSigIntHandler()
{
    struct sigaction action;
    action.sa_handler = &InterruptSignalHandlerPrivate::sigIntHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_flags |= SA_RESTART;

    if (sigaction(SIGINT, &action, &d->oldAction))
        return false;

    return true;
}

InterruptSignalHandlerPrivate::InterruptSignalHandlerPrivate(InterruptSignalHandler *owner)
    : q{owner}
{

}

void InterruptSignalHandlerPrivate::sigIntHandler(int signalId)
{
    Q_UNUSED(signalId);

    char a = 1;
    write(s_socketPair[0], &a, sizeof(a));
}

void InterruptSignalHandlerPrivate::handleSigInt()
{
    char tmp;
    read(s_socketPair[1], &tmp, sizeof(tmp));

    // Reset signal action to allow a second Control-C to interrupt hanging qdb
    sigaction(SIGINT, &oldAction, nullptr);

    emit q->interrupted();
}

#include "interruptsignalhandler_unix.moc"
