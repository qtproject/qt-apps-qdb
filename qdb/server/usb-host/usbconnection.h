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
#ifndef USBCONNECTION_H
#define USBCONNECTION_H

#include "usbdevice.h"

class UsbConnectionReader;

#include <QtCore/qbytearray.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qqueue.h>
QT_BEGIN_NAMESPACE
class QThread;
QT_END_NAMESPACE

#include <memory>

struct libusb_device;
struct libusb_device_handle;

class UsbConnection : public QIODevice
{
    Q_OBJECT
public:
    UsbConnection(const UsbDevice &device);
    ~UsbConnection();

    bool open(QIODevice::OpenMode mode) override;

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

private slots:
    void dataRead(QByteArray data);

private:
    void startReader(libusb_device_handle *handle, uint8_t inAddress);

    LibUsbDevice m_device;
    libusb_device_handle *m_handle;
    UsbInterfaceInfo m_interfaceInfo;
    bool m_detachedKernel;
    std::unique_ptr<QThread> m_readThread;
    std::unique_ptr<UsbConnectionReader> m_reader;
    QQueue<QByteArray> m_reads;
};

#endif // USBCONNECTION_H
