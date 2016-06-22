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

#include "../utils/make_unique.h"
#include "../utils/scopeguard.h"
#include "protocol/protocol.h"
#include "usbconnectionreader.h"

#include <QtCore/qdebug.h>
#include <QtCore/qthread.h>

#include <libusb.h>

static const uint16_t vendorID = 0x18d1; // Google Inc.
static const uint16_t productID = 0x0d02; // Celkon A88
static const int inEndpointIndex = 1;
static const int outEndpointIndex = 0;

bool isQdbInterface(const libusb_interface &interface)
{
    const libusb_interface_descriptor *descriptor = &interface.altsetting[0];
    return descriptor->bInterfaceClass == qdbUsbClassId && descriptor->bInterfaceSubClass == qdbUsbSubclassId;
}

UsbConnection::UsbConnection()
    : m_context(nullptr),
      m_handle(nullptr),
      m_interfaceNumber(0),
      m_inAddress(0),
      m_outAddress(0),
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
        libusb_release_interface(m_handle, m_interfaceNumber);
        libusb_close(m_handle);
    }
    if (m_context)
        libusb_exit(m_context);
}

bool UsbConnection::open(OpenMode mode)
{
    Q_ASSERT(mode == (QIODevice::ReadWrite | QIODevice::Unbuffered));
    QIODevice::open(mode);

    libusb_device *found = NULL;

    int ret = libusb_init(&m_context);
    if (ret) {
        qDebug("cannot init libusb: %s\n", libusb_error_name(ret));
        return false;
    }

    libusb_device **devices;
    ssize_t deviceCount = libusb_get_device_list(m_context, &devices);
    ScopeGuard deviceListGuard = [&]() {
        libusb_free_device_list(devices, 1);
    };

    if (deviceCount <= 0) {
        qDebug("no devices found\n");
        return false;
    }

    for (int i = 0; i < deviceCount; ++i) {
        libusb_device *device = devices[i];
        struct libusb_device_descriptor desc;
        ret = libusb_get_device_descriptor(device, &desc);
        if (ret) {
            qDebug("unable to get device descriptor: %s\n",
                   libusb_error_name(ret));
            return false;
        }
        if (desc.idVendor == vendorID && desc.idProduct == productID) {
            qDebug("found QDB device");
            found = device;
            break;
        }
    }

    if (!found) {
        qDebug("no QDB devices found\n");
        return false;
    }

    libusb_config_descriptor *config;
    ret = libusb_get_active_config_descriptor(found, &config);
    if (ret) {
        qDebug("could not get config descriptor: %s\n",
               libusb_error_name(ret));
        return false;
    }
    ScopeGuard configGuard = [&]() {
        libusb_free_config_descriptor(config);
    };

    auto last = config->interface + config->bNumInterfaces;
    auto qdbInterface = std::find_if(config->interface, last, isQdbInterface);
    if (qdbInterface == last) {
        qDebug("no QDB interface found in device");
        return false;
    }

    const libusb_interface_descriptor *interface = &qdbInterface->altsetting[0];
    m_interfaceNumber = interface->bInterfaceNumber;
    m_inAddress = interface->endpoint[inEndpointIndex].bEndpointAddress;
    m_outAddress = interface->endpoint[outEndpointIndex].bEndpointAddress;

    qDebug("interface %d is a qdb interface", m_interfaceNumber);

    ret = libusb_open(found, &m_handle);
    if (ret) {
        qDebug("cannot open device: %s\n", libusb_error_name(ret));
        return false;
    }

    libusb_set_auto_detach_kernel_driver(m_handle, 1);

    ret = libusb_claim_interface(m_handle, m_interfaceNumber);
    if (ret) {
        qDebug("cannot claim interface: %s", libusb_error_name(ret));
        return false;
    }
    qDebug("claimed interface %d", m_interfaceNumber);

    startReader(m_handle, m_inAddress);

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
    int ret = libusb_bulk_transfer(m_handle, m_outAddress, (unsigned char*)data, size, &transferred, 0);
    if (ret) {
        qDebug() << "writeData error:" << libusb_error_name(ret);
        return -1;
    }
    Q_ASSERT(transferred == size); // TODO: handle partial transfers of header
    transferred = 0;

    if (size < maxSize) {
        int rest = maxSize - size;
        int ret = libusb_bulk_transfer(m_handle, m_outAddress, (unsigned char*)data + size, rest, &transferred, 0);
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
