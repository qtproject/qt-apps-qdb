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
#include "usbconnection.h"

#include "libqdb/make_unique.h"
#include "libqdb/protocol/protocol.h"
#include "libqdb/scopeguard.h"
#include "usbcommon.h"
#include "usbconnectionreader.h"

#include <QtCore/qdebug.h>
#include <QtCore/qthread.h>

#include <libusb.h>

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
    if (ret) {
        qDebug("could not get config descriptor: %s\n",
               libusb_error_name(ret));
        return false;
    }
    ScopeGuard configGuard = [&]() {
        libusb_free_config_descriptor(config);
    };

    ret = libusb_open(m_device.pointer(), &m_handle);
    if (ret) {
        qDebug("cannot open device: %s\n", libusb_error_name(ret));
        return false;
    }

    if (libusb_kernel_driver_active(m_handle, m_interfaceInfo.number) == 1) {
        qDebug() << "Detached kernel driver";
        m_detachedKernel = true;
        libusb_detach_kernel_driver(m_handle, m_interfaceInfo.number);
    }

    ret = libusb_claim_interface(m_handle, m_interfaceInfo.number);
    if (ret) {
        qDebug("cannot claim interface: %s", libusb_error_name(ret));
        return false;
    }
    qDebug("claimed interface %d", m_interfaceInfo.number);

    startReader(m_handle, m_interfaceInfo.inAddress);

    return true;
}

qint64 UsbConnection::readData(char *data, qint64 maxSize)
{
    if (m_reads.isEmpty()) {
        qDebug() << "UsbConnection read queue empty in readData";
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
    if (ret) {
        qDebug() << "writeData error:" << libusb_error_name(ret);
        return -1;
    }
    Q_ASSERT(transferred == size); // TODO: handle partial transfers of header
    transferred = 0;

    if (size < maxSize) {
        int rest = maxSize - size;
        int ret = libusb_bulk_transfer(m_handle, m_interfaceInfo.outAddress, (unsigned char*)data + size, rest, &transferred, 0);
        if (ret) {
            qDebug() << "writeData error:" << libusb_error_name(ret);
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
