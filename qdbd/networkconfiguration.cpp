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
#include "networkconfiguration.h"

#include "configuration.h"

#include <QtCore/QMutexLocker>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qprocess.h>

Q_LOGGING_CATEGORY(configurationC, "qdb.networkconfiguration")

NetworkConfiguration *NetworkConfiguration::instance()
{
    static NetworkConfiguration instance;
    return &instance;
}

ConfigurationResult NetworkConfiguration::set(QString subnetString)
{
    QMutexLocker m_locker{&m_lock};
    if (!m_subnetString.isEmpty()) {
        qCWarning(configurationC) << "Can't set network configuration since it is already set";
        return ConfigurationResult::AlreadySet;
    }
    m_subnetString = subnetString;

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(&process, &QProcess::readyReadStandardOutput, &process, [&]() {
        qCDebug(configurationC) << "Script:" << process.readAllStandardOutput();
    });

    const QStringList args = QStringList{"--set", subnetString};
    qCDebug(configurationC) << "Running network configuration script" << Configuration::networkScript() << args;
    process.start(Configuration::networkScript(), args);

    process.waitForFinished();
    if (process.exitCode() != 0) {
        qCWarning(configurationC) << "Using script" << Configuration::networkScript() << "to configure the network failed";
        m_subnetString.clear();
        return ConfigurationResult::Failure;
    }

    qCDebug(configurationC) << "Configured network device to" << subnetString;
    return ConfigurationResult::Success;
}

bool NetworkConfiguration::reset()
{
    QMutexLocker m_locker{&m_lock};
    m_subnetString.clear();

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(&process, &QProcess::readyReadStandardOutput, &process, [&]() {
        qCDebug(configurationC) << "Script:" << process.readAllStandardOutput();
    });

    const QStringList args = QStringList{"--reset"};
    qCDebug(configurationC) << "Running network configuration script" << Configuration::networkScript() << args;
    process.start(Configuration::networkScript(), args);

    process.waitForFinished();
    if (process.exitCode() != 0) {
        qCWarning(configurationC) << "Using script" << Configuration::networkScript()
                                  << "to reset the network configuration failed";
        return false;
    }
    qCDebug(configurationC) << "Reset the network configuration";
    return true;
}

bool NetworkConfiguration::isSet() const
{
    QMutexLocker m_locker{&m_lock};
    return !m_subnetString.isEmpty();
}

QString NetworkConfiguration::subnet() const
{
    QMutexLocker m_locker{&m_lock};
    return m_subnetString;
}

NetworkConfiguration::NetworkConfiguration()
    : m_subnetString{}
{

}
