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

#ifdef Q_OS_UNIX
#include "libqdb/make_unique.h"

#include <QtCore/qdebug.h>
#include <QtCore/qsocketnotifier.h>

#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

#elif Q_OS_WINDOWS
#include <windows.h>
#endif

#ifdef Q_OS_UNIX
int InterruptSignalHandler::s_socketPair[2] = {0, 0};
#elif Q_OS_WINDOWS
InterruptSignalHandler* InterruptSignalHandler::m_handler = nullptr;
#endif

InterruptSignalHandler::InterruptSignalHandler(QObject *parent)
    : QObject(parent)
{
#ifdef Q_OS_UNIX
    m_socketNotifier = nullptr;
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, s_socketPair))
        qFatal("Could not create socketpair for signal handling in InterruptSignalHandler");

    m_socketNotifier = make_unique<QSocketNotifier>(s_socketPair[1], QSocketNotifier::Read);
    connect(m_socketNotifier.get(), SIGNAL(activated(int)), this, SLOT(handleSigInt()));
#endif

    if (!installSigIntHandler())
        qFatal("Could not install signal handler in InterruptSignalHandler");
}

InterruptSignalHandler::~InterruptSignalHandler() {
#ifdef Q_OS_WINDOWS
    SetConsoleCtrlHandler(&InterruptSignalHandler::consoleHandler, FALSE);
#endif
}

bool InterruptSignalHandler::installSigIntHandler()
{
#ifdef Q_OS_UNIX
    struct sigaction action;
    action.sa_handler = &InterruptSignalHandler::sigIntHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    action.sa_flags |= SA_RESTART;

    if (sigaction(SIGINT, &action, &m_oldAction))
       return false;
#elif Q_OS_WINDOWS
    m_handler = this;
    if (!SetConsoleCtrlHandler(&InterruptSignalHandler::consoleHandler, TRUE))
        return false;
#endif

    return true;
}

#ifdef Q_OS_UNIX
void InterruptSignalHandler::sigIntHandler(int signalId)
{
    Q_UNUSED(signalId);

    char a = 1;
    write(s_socketPair[0], &a, sizeof(a));
}
#endif

#ifdef Q_OS_UNIX
void InterruptSignalHandler::handleSigInt()
{
    char tmp;
    read(s_socketPair[1], &tmp, sizeof(tmp));

    // Reset signal action to allow a second Control-C to interrupt hanging qdb
    sigaction(SIGINT, &m_oldAction, nullptr);

    emit interrupted();
}
#endif

#ifdef Q_OS_WINDOWS
BOOL WINAPI InterruptSignalHandler::consoleHandler(DWORD signal)
{
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
        emit m_handler->interrupted();
        return true;
    }
    return false;
}
#endif
