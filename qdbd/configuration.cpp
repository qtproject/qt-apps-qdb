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

QString Configuration::rndisFunctionName()
{
    return s_rndisFunctionName;
}

void Configuration::setFunctionFsDir(const QString &path)
{
    s_functionFsDir = QDir::cleanPath(path);
}

void Configuration::setGadgetConfigFsDir(const QString &path)
{
    s_gadgetConfigFsDir = QDir::cleanPath(path);
}

void Configuration::setRndisFunctionName(const QString &name)
{
    s_rndisFunctionName = name;
}

QString Configuration::s_functionFsDir = "/dev/usb-ffs/qdb";
QString Configuration::s_gadgetConfigFsDir = "/sys/kernel/config/usb_gadget/g1";
QString Configuration::s_rndisFunctionName = "rndis.usb0";
