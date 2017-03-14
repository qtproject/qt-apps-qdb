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
#include "usbgadgetcontrol.h"

#include "configuration.h"
#include "libqdb/protocol/protocol.h"
#include "libqdb/protocol/qdbmessage.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qprocess.h>
#include <QtCore/qtimer.h>

#include <linux/usb/functionfs.h>

Q_DECLARE_LOGGING_CATEGORY(usbC);

namespace {

QString eventTypeName(usb_functionfs_event_type eventType)
{
    switch (eventType)
    {
    case FUNCTIONFS_BIND:
        return QLatin1String("FUNCTIONFS_BIND");
    case FUNCTIONFS_UNBIND:
        return QLatin1String("FUNCTIONFS_UNBIND");
    case FUNCTIONFS_ENABLE:
        return QLatin1String("FUNCTIONFS_ENABLE");
    case FUNCTIONFS_DISABLE:
        return QLatin1String("FUNCTIONFS_DISABLE");
    case FUNCTIONFS_SETUP:
        return QLatin1String("FUNCTIONFS_SETUP");
    case FUNCTIONFS_SUSPEND:
        return QLatin1String("FUNCTIONFS_SUSPEND");
    case FUNCTIONFS_RESUME:
        return QLatin1String("FUNCTIONFS_RESUME");
    }
    return QLatin1String("(unknown)");
}

} // anonymous namespace

UsbGadgetControl::UsbGadgetControl(QFile *controlEndpoint)
    : m_controlEndpoint{controlEndpoint}
{

}

void UsbGadgetControl::monitor()
{
    if (!m_controlEndpoint->isOpen()) {
        qCCritical(usbC) << "Tried to read FFS events from a closed endpoint";
        return;
    }

    QTimer::singleShot(0, this, &UsbGadgetControl::monitor);

    const auto eventSize = sizeof(usb_functionfs_event);

    QByteArray buffer{eventSize, '\0'};
    int count = m_controlEndpoint->read(buffer.data(), buffer.size());
    if (count == -1) {
        qCWarning(usbC) << "Could not read FFS event from control endpoint";
        return;
    } else if (count < buffer.size()) {
        qCWarning(usbC) << "Could only read" << count << "out of" << buffer.size()
                        << "byte event from control endpoint";
        return;
    }

    const auto *event = reinterpret_cast<usb_functionfs_event *>(buffer.data());
    const auto eventType = static_cast<usb_functionfs_event_type>(event->type);
    qCDebug(usbC) << "USB FFS event:" << eventTypeName(eventType);
    if (eventType == FUNCTIONFS_DISABLE || eventType == FUNCTIONFS_SUSPEND) {
        QProcess process;
        process.start(Configuration::networkScript(), QStringList{"--reset"});
        process.waitForFinished();
        if (process.exitCode() != 0) {
            qCWarning(usbC) << "Using script" << Configuration::networkScript()
                            << "to reset the network configuration failed";
            return;
        }
        qCDebug(usbC) << "Reset the network configuration";
    }
}
