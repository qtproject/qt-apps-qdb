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
#include "usbgadgetwriter.h"

#include "protocol/protocol.h"
#include "protocol/qdbmessage.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qtimer.h>

UsbGadgetWriter::UsbGadgetWriter(QFile *writeEndpoint)
    : m_writeEndpoint{writeEndpoint}
{

}

void UsbGadgetWriter::write(QByteArray data)
{
    if (!m_writeEndpoint->isOpen()) {
        qWarning() << "UsbGadgetWriter: Tried to write to a closed endpoint";
        emit writeDone(false);
        return;
    }

    auto written = m_writeEndpoint->write(data);
    if (written != data.size()) {
        qWarning() << "UsbGadgetWriter: Write to endpoint failed";
        emit writeDone(false);
    }
    emit writeDone(true);
}