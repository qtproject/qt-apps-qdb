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
#include "usbconnectionreader.h"

#include "libqdb/protocol/protocol.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qthread.h>
#include <QtCore/qtimer.h>

#include <libusb.h>

Q_DECLARE_LOGGING_CATEGORY(usbC);

// Amount of milliseconds between yielding control to the event loop of the reading thread
static const int quitCheckingTimeout = 500;

UsbConnectionReader::UsbConnectionReader(libusb_device_handle *handle, uint8_t inAddress)
    : m_handle{handle},
      m_inAddress{inAddress},
      m_errorCount{0}
{

}

void UsbConnectionReader::executeRead()
{
    QByteArray buffer{qdbMessageSize, '\0'};
    int transferred = 0;
    int ret = libusb_bulk_transfer(m_handle, m_inAddress, reinterpret_cast<unsigned char *>(buffer.data()),
                                   buffer.size(), &transferred, quitCheckingTimeout);
    if (ret != LIBUSB_SUCCESS) {
        if (ret != LIBUSB_ERROR_TIMEOUT) {
            qCWarning(usbC) << "Could not read from USB connection:" << libusb_error_name(ret);
            ++m_errorCount;
            if (m_errorCount == 5) {
                emit newRead(QByteArray{});
                return; // Quit reading, this connection has failed.
            }
        }
    } else {
        m_errorCount = 0;
        buffer.resize(transferred);
        emit newRead(buffer);
    }
    QTimer::singleShot(0, this, &UsbConnectionReader::executeRead);
}
