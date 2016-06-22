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
#include "interruptsignalhandler.h"

#include "../utils/make_unique.h"

#include <QtCore/qdebug.h>
#include <QtCore/qsocketnotifier.h>

#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

InterruptSignalHandler::InterruptSignalHandler(QObject *parent)
    : QObject(parent),
      m_socketNotifier{nullptr}
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, s_socketPair))
        qFatal("Could not create socketpair for signal handling in InterruptSignalHandler");

    m_socketNotifier = make_unique<QSocketNotifier>(s_socketPair[1], QSocketNotifier::Read);
    connect(m_socketNotifier.get(), SIGNAL(activated(int)), this, SLOT(handleSigInt()));

    if (!installSigIntHandler())
        qFatal("Could not install signal handler in InterruptSignalHandler");
}

int InterruptSignalHandler::s_socketPair[2] = {0, 0};

InterruptSignalHandler::~InterruptSignalHandler() = default;

void InterruptSignalHandler::sigIntHandler(int signalId)
{
    Q_UNUSED(signalId);

    char a = 1;
    write(s_socketPair[0], &a, sizeof(a));
}

void InterruptSignalHandler::handleSigInt()
{
    char tmp;
    read(s_socketPair[1], &tmp, sizeof(tmp));

    // Reset signal action to allow a second Control-C to interrupt hanging qdb
    sigaction(SIGINT, &m_oldAction, nullptr);

    emit interrupted();
}

bool InterruptSignalHandler::installSigIntHandler()
{
    struct sigaction action;
    action.sa_handler = &InterruptSignalHandler::sigIntHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_flags |= SA_RESTART;

    if (sigaction(SIGINT, &action, &m_oldAction))
       return false;

    return true;
}

