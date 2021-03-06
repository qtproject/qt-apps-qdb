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
#include "usbconnection.h"

#include "libqdb/make_unique.h"
#include "libqdb/protocol/protocol.h"
#include "libqdb/scopeguard.h"
#include "usbcommon.h"
#include "usbconnectionreader.h"

#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qthread.h>

#include <libusb.h>

Q_LOGGING_CATEGORY(usbC, "qdb.usb");

UsbConnection::UsbConnection(const UsbDevice &device)
    : m_device{device.usbDevice},
      m_handle{nullptr},
      m_interfaceInfo(device.interfaceInfo), // uniform initialization with {} fails with GCC 4.9
      m_detachedKernel{false},
      m_readThread{nullptr},
      m_reader{nullptr},
      m_reads{}
{
}

UsbConnection::~UsbConnection()
{
    if (m_readThread) {
        m_readThread->quit();
        m_readThread->wait();
    }
    if (m_handle) {
        libusb_release_interface(m_handle, m_interfaceInfo.number);
        if (m_detachedKernel)
            libusb_attach_kernel_driver(m_handle, m_interfaceInfo.number);
        libusb_close(m_handle);
    }
}

bool UsbConnection::open(OpenMode mode)
{
    Q_ASSERT(mode == (QIODevice::ReadWrite | QIODevice::Unbuffered));
    QIODevice::open(mode);

    libusb_config_descriptor *config;
    int ret = libusb_get_active_config_descriptor(m_device.pointer(), &config);
    if (ret != LIBUSB_SUCCESS) {
        qCWarning(usbC) << "Could not get config descriptor:" << libusb_error_name(ret);
        return false;
    }
    ScopeGuard configGuard = [&]() {
        libusb_free_config_descriptor(config);
    };

    ret = libusb_open(m_device.pointer(), &m_handle);
    if (ret != LIBUSB_SUCCESS) {
        if (ret == LIBUSB_ERROR_ACCESS) {
            qCWarning(usbC) << "Access to USB device was denied."
                            << "Check the manual for setting up access to USB devices.";
        } else {
            qCWarning(usbC) << "Could not open device:" << libusb_error_name(ret);
        }
        return false;
    }

    if (libusb_kernel_driver_active(m_handle, m_interfaceInfo.number) == 1) {
        qCDebug(usbC) << "Detached kernel driver";
        m_detachedKernel = true;
        libusb_detach_kernel_driver(m_handle, m_interfaceInfo.number);
    }

    ret = libusb_claim_interface(m_handle, m_interfaceInfo.number);
    if (ret != LIBUSB_SUCCESS) {
        qCDebug(usbC) << "Could not claim interface:" << libusb_error_name(ret);
        return false;
    }
    qCDebug(usbC) << "Claimed interface" << m_interfaceInfo.number;

    startReader(m_handle, m_interfaceInfo.inAddress);

    return true;
}

qint64 UsbConnection::readData(char *data, qint64 maxSize)
{
    if (m_reads.isEmpty()) {
        qCWarning(usbC) << "UsbConnection read queue empty while trying to read";
        return -1;
    }
    QByteArray read = m_reads.dequeue();
    Q_ASSERT(read.size() <= maxSize); // TODO: handle too big reads
    std::copy(read.begin(), read.end(), data);

    return read.size();
}

qint64 UsbConnection::writeData(const char *data, qint64 maxSize)
{
    // Send header as a separate transfer to allow separate read on device side
    int size = maxSize > qdbHeaderSize ? qdbHeaderSize : maxSize;

    int transferred = 0;
    int ret = libusb_bulk_transfer(m_handle, m_interfaceInfo.outAddress, (unsigned char*)data, size, &transferred, 0);
    if (ret != LIBUSB_SUCCESS) {
        qCCritical(usbC) << "Could not write message header:" << libusb_error_name(ret);
        return -1;
    }
    Q_ASSERT(transferred == size); // TODO: handle partial transfers of header
    transferred = 0;

    if (size < maxSize) {
        int rest = maxSize - size;
        int ret = libusb_bulk_transfer(m_handle, m_interfaceInfo.outAddress, (unsigned char*)data + size, rest, &transferred, 0);
        if (ret != LIBUSB_SUCCESS) {
            qCCritical(usbC) << "Could not write message payload:" << libusb_error_name(ret);
            return -1;
        }
    }
    return size + transferred;
}

void UsbConnection::dataRead(QByteArray data)
{
    m_reads.enqueue(data);
    emit readyRead();
}

void UsbConnection::startReader(libusb_device_handle *handle, uint8_t inAddress)
{
    m_readThread = make_unique<QThread>();
    m_reader = make_unique<UsbConnectionReader>(handle, inAddress);

    connect(m_reader.get(), &UsbConnectionReader::newRead, this, &UsbConnection::dataRead);
    connect(m_readThread.get(), &QThread::started, m_reader.get(), &UsbConnectionReader::executeRead);
    m_reader->moveToThread(m_readThread.get());

    m_readThread->setObjectName("UsbConnectionReader");
    m_readThread->start();
}
