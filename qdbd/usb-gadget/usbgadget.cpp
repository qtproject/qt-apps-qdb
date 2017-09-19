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
#include "usbgadget.h"

#include "configuration.h"
#include "libqdb/make_unique.h"
#include "libqdb/qdbconstants.h"
#include "usb-gadget/usbgadgetcontrol.h"
#include "usb-gadget/usbgadgetreader.h"
#include "usb-gadget/usbgadgetwriter.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qthread.h>

#include <QDirIterator>

#include <linux/usb/functionfs.h>

Q_LOGGING_CATEGORY(usbC, "qdb.usb");

usb_interface_descriptor makeInterfaceDescriptor()
{
    usb_interface_descriptor interface;
    interface.bLength = sizeof(interface);
    interface.bDescriptorType = USB_DT_INTERFACE;
    interface.bInterfaceNumber = 0;
    interface.bAlternateSetting = 0;
    interface.bNumEndpoints = 2;
    interface.bInterfaceClass = qdbUsbClassId;
    interface.bInterfaceSubClass = qdbUsbSubclassId;
    interface.bInterfaceProtocol = qdbUsbProtocolId;
    interface.iInterface = 1;
    return interface;
}

usb_endpoint_descriptor_no_audio makeEndpointDescriptor(uint8_t endpointAddress, uint16_t maxPacketSize)
{
    usb_endpoint_descriptor_no_audio endpoint;
    endpoint.bLength = sizeof(endpoint);
    endpoint.bDescriptorType = USB_DT_ENDPOINT;
    endpoint.bEndpointAddress = endpointAddress;
    endpoint.bmAttributes = USB_ENDPOINT_XFER_BULK;
    endpoint.wMaxPacketSize = htole16(maxPacketSize);
    endpoint.bInterval = 0;
    return endpoint;
}

const struct {
    struct usb_functionfs_descs_head header;
    struct {
        struct usb_interface_descriptor intf;
        struct usb_endpoint_descriptor_no_audio bulk_source;
        struct usb_endpoint_descriptor_no_audio bulk_sink;
    } __attribute__ ((__packed__)) fs_descs, hs_descs;
} __attribute__ ((__packed__)) descriptors = {
    {
        htole32(FUNCTIONFS_DESCRIPTORS_MAGIC),
        htole32(sizeof(descriptors)), /* length */
        htole32(3), /* full speed descriptor count */
        htole32(3), /* high speed descriptor count */
    },
    {
        makeInterfaceDescriptor(), /* full speed interface descriptor */
        makeEndpointDescriptor(1 | USB_DIR_OUT, 64),
        makeEndpointDescriptor(2 | USB_DIR_IN, 64),
    },
    {
        makeInterfaceDescriptor(), /* high speed interface descriptor */
        makeEndpointDescriptor(1 | USB_DIR_OUT, 512),
        makeEndpointDescriptor(2 | USB_DIR_IN, 512),
    },
};

#define STR_INTERFACE "QDB Interface"

const struct {
    struct usb_functionfs_strings_head header;
    struct {
        __le16 code;
        const char str1[sizeof(STR_INTERFACE)];
    } __attribute__ ((__packed__)) lang0;
} __attribute__ ((__packed__)) strings = {
    {
        htole32(FUNCTIONFS_STRINGS_MAGIC),
        htole32(sizeof(strings)),
        htole32(1),
        htole32(1),
    },
    {
        htole16(0x0409), /* en-us */
        STR_INTERFACE,
    },
};

UsbGadget::UsbGadget()
    : m_controlEndpoint(Configuration::functionFsDir() + "/ep0"),
      m_outEndpoint(Configuration::functionFsDir() + "/ep1"),
      m_inEndpoint(Configuration::functionFsDir() + "/ep2"),
      m_controlThread{nullptr},
      m_readThread{nullptr},
      m_writeThread{nullptr},
      m_control{nullptr},
      m_reader{nullptr},
      m_writer{nullptr},
      m_reads{}
{

}

UsbGadget::~UsbGadget()
{
    if (m_controlThread) {
        m_controlThread->terminate();
        m_controlThread->wait();
    }
    if (m_readThread) {
        m_readThread->terminate();
        m_readThread->wait();
    }
    if (m_writeThread) {
        m_writeThread->terminate();
        m_readThread->wait();
    }
    m_inEndpoint.close();
    m_outEndpoint.close();
    m_controlEndpoint.close();
}

bool UsbGadget::open(QIODevice::OpenMode mode)
{
    Q_ASSERT(mode == (QIODevice::ReadWrite | QIODevice::Unbuffered));
    QIODevice::open(mode);

    if (!openControlEndpoint())
        return false;

    qint64 bytes = m_controlEndpoint.write(reinterpret_cast<const char*>(&descriptors), sizeof(descriptors));
    if (bytes == -1) {
        qCCritical(usbC) << "Failed to write USB descriptors:" << m_controlEndpoint.errorString();
        return false;
    }

    bytes = m_controlEndpoint.write(reinterpret_cast<const char*>(&strings), sizeof(strings));
    if (bytes == -1) {
        qCCritical(usbC) << "Failed to write USB strings:" << m_controlEndpoint.errorString();
        return false;
    }

    if (!m_outEndpoint.open(QIODevice::ReadOnly | QIODevice::Unbuffered)) {
        qCCritical(usbC) << "Failed to open endpoint from host to gadget";
        return false;
    }

    if (!m_inEndpoint.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        qCCritical(usbC) << "Failed to open endpoint from gadget to host";
        return false;
    }
    qCDebug(usbC) << "Initialized function fs";

    startControlThread();
    startReadThread();
    startWriteThread();
    initializeGadgetWithUdc();

    return true;
}

qint64 UsbGadget::readData(char *data, qint64 maxSize)
{
    if (m_reads.isEmpty()) {
        qCWarning(usbC) << "UsbGadget read queue empty while trying to read";
        return -1;
    }
    QByteArray read = m_reads.dequeue();
    Q_ASSERT(read.size() <= maxSize); // TODO: handle too big reads
    std::copy(read.begin(), read.end(), data);

    return read.size();
}

qint64 UsbGadget::writeData(const char *data, qint64 size)
{
    if (m_inEndpoint.isOpen()) {
        emit writeAvailable(QByteArray{data, static_cast<int>(size)});
        return size;
    }

    qCCritical(usbC) << "Tried to send to host through closed endpoint";
    return -1;
}

void UsbGadget::dataRead(QByteArray data)
{
    m_reads.enqueue(data);
    emit readyRead();
}

void UsbGadget::startControlThread()
{
    m_control = make_unique<UsbGadgetControl>(&m_controlEndpoint);
    m_controlThread = make_unique<QThread>();

    connect(m_controlThread.get(), &QThread::started,
            m_control.get(), &UsbGadgetControl::monitor);

    m_control->moveToThread(m_controlThread.get());
    m_controlThread->setObjectName("UsbGadgetControl");
    m_controlThread->start();
}

void UsbGadget::startReadThread()
{
    m_reader = make_unique<UsbGadgetReader>(&m_outEndpoint);
    m_readThread = make_unique<QThread>();

    connect(m_reader.get(), &UsbGadgetReader::newRead, this, &UsbGadget::dataRead);
    connect(m_readThread.get(), &QThread::started, m_reader.get(), &UsbGadgetReader::executeRead);

    m_reader->moveToThread(m_readThread.get());
    m_readThread->setObjectName("UsbGadgetReader");
    m_readThread->start();
}

void UsbGadget::startWriteThread()
{
    m_writer = make_unique<UsbGadgetWriter>(&m_inEndpoint);
    m_writeThread = make_unique<QThread>();

    connect(this, &UsbGadget::writeAvailable, m_writer.get(), &UsbGadgetWriter::write);

    m_writer->moveToThread(m_writeThread.get());
    m_writeThread->setObjectName("UsbGadgetWriter");
    m_writeThread->start();
}

bool UsbGadget::openControlEndpoint()
{
    if (!QFile::exists(m_controlEndpoint.fileName())) {
        qCCritical(usbC) << "USB ffs control endpoint" << m_controlEndpoint.fileName() << "does not exist";
        return false;
    }
    if (!m_controlEndpoint.open(QIODevice::ReadWrite | QIODevice::Unbuffered)) {
        qCCritical(usbC) << "Failed to open control endpoint" << m_controlEndpoint.fileName();
        return false;
    }
    return true;
}

/**
 * Initialize usb gadget with the first UDC driver
 */
void UsbGadget::initializeGadgetWithUdc()
{
    QString driverName = []() {
        QDirIterator it(Configuration::udcDriverDir());
        while (it.hasNext()) {
            it.next();
            const QString candidate = it.fileName();
            if (candidate != QLatin1String(".") && candidate != QLatin1String(".."))
                return candidate;
        }
        return QString{};
    }();
    if (driverName.isEmpty()) {
        qCCritical(usbC) << "Failed to initialize USB gadget, no UDC drivers found in"
                         << Configuration::udcDriverDir();
        return;
    }

    const QString gadgetConfigPath = Configuration::gadgetConfigFsDir() + QLatin1String("/UDC");
    QFile gadgetConfigFile{gadgetConfigPath};
    if (!gadgetConfigFile.exists()) {
        qCCritical(usbC) << "Failed to initialize USB gadget, no gadget file found in"
                         << gadgetConfigPath;
        return;
    }
    if (!gadgetConfigFile.open(QIODevice::ReadWrite | QIODevice::Unbuffered)) {
        qCCritical(usbC) << "Failed to initialize USB gadget, can't open" << gadgetConfigPath;
        return;
    }
    if (gadgetConfigFile.write(driverName.toUtf8()) == -1) {
        qCCritical(usbC) << "Failed to initialize USB gadget, can't write to"
                         << gadgetConfigPath;
        return;
    }
    gadgetConfigFile.close();
    qCDebug(usbC) << "Initialized USB gadget UDC driver";
}
