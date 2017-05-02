/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
#ifndef NETWORKCONFIGURATION_H
#define NETWORKCONFIGURATION_H

#include "libqdb/networkconfigurationcommon.h"

#include <QtCore/qmutex.h>
#include <QtCore/qstring.h>

class NetworkConfiguration
{
public:
    static NetworkConfiguration *instance();

    ConfigurationResult set(QString subnetString);
    bool reset();

    bool isSet() const;

    QString subnet() const;

private:
    NetworkConfiguration();

    mutable QMutex m_lock;
    QString m_subnetString;
};

#endif // NETWORKCONFIGURATION_H
