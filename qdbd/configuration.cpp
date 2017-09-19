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
#include "configuration.h"

#include <QtCore/qdir.h>

QString Configuration::functionFsDir()
{
    return s_functionFsDir;
}

QString Configuration::gadgetConfigFsDir()
{
    return s_gadgetConfigFsDir;
}

QString Configuration::networkScript()
{
    return s_networkScript;
}

QString Configuration::rndisFunctionName()
{
    return s_rndisFunctionName;
}

QString Configuration::udcDriverDir()
{
    return s_udcDriverDir;
}

void Configuration::setFunctionFsDir(const QString &path)
{
    s_functionFsDir = QDir::cleanPath(path);
}

void Configuration::setGadgetConfigFsDir(const QString &path)
{
    s_gadgetConfigFsDir = QDir::cleanPath(path);
}

void Configuration::setNetworkScript(const QString &script)
{
    s_networkScript = script;
}

void Configuration::setRndisFunctionName(const QString &name)
{
    s_rndisFunctionName = name;
}

QString Configuration::s_functionFsDir = "/dev/usb-ffs/qdb";
QString Configuration::s_gadgetConfigFsDir = "/sys/kernel/config/usb_gadget/g1";
QString Configuration::s_rndisFunctionName = "rndis.usb0";
QString Configuration::s_networkScript = "b2qt-gadget-network.sh";
QString Configuration::s_udcDriverDir = "/sys/class/udc/";
