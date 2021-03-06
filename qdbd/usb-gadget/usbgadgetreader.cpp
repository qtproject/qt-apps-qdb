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
#include "usbgadgetreader.h"

#include "libqdb/protocol/protocol.h"
#include "libqdb/protocol/qdbmessage.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qtimer.h>

Q_DECLARE_LOGGING_CATEGORY(usbC);

UsbGadgetReader::UsbGadgetReader(QFile *readEndpoint)
    : m_readEndpoint{readEndpoint}
{

}

void UsbGadgetReader::executeRead()
{
    if (!m_readEndpoint->isOpen()) {
        qCCritical(usbC) << "Tried to receive from host through closed endpoint";
        return;
    }

    QTimer::singleShot(0, this, &UsbGadgetReader::executeRead);

    QByteArray headerBuffer{qdbHeaderSize, '\0'};
    int count = m_readEndpoint->read(headerBuffer.data(), headerBuffer.size());
    if (count == -1) {
        qCWarning(usbC) << "Could not read message header from endpoint";
        return;
    } else if (count < headerBuffer.size()) {
        qCWarning(usbC) << "Could only read" << count << "out of" << headerBuffer.size() << "byte header from endpoint";
        return;
    }

    int dataSize = QdbMessage::GetDataSize(headerBuffer);
    Q_ASSERT(dataSize >= 0);
    if (dataSize == 0) {
        emit newRead(headerBuffer);
        return;
    }

    QByteArray dataBuffer{dataSize, '\0'};
    count = m_readEndpoint->read(dataBuffer.data(), dataBuffer.size());
    if (count == -1) {
        qCWarning(usbC) << "Could not read message payload from endpoint";
        return;
    } else if (count < dataBuffer.size()) {
        qCWarning(usbC) << "Could only read" << count << "out of" << headerBuffer.size() << "byte payload from endpoint";
        return;
    }

    emit newRead(headerBuffer + dataBuffer);
}
