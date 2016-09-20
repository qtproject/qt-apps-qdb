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
#ifndef USBMANAGER_H
#define USBMANAGER_H

#include "libqdb_global.h"

class UsbConnectionReader;

#include <QtCore/qbytearray.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qqueue.h>
QT_BEGIN_NAMESPACE
class QThread;
QT_END_NAMESPACE

#include <memory>

struct libusb_context;
struct libusb_device_handle;

class LIBQDBSHARED_EXPORT UsbConnection : public QIODevice
{
    Q_OBJECT
public:
    UsbConnection();
    ~UsbConnection();

    bool open(QIODevice::OpenMode mode) override;

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

private slots:
    void dataRead(QByteArray data);

private:
    void startReader(libusb_device_handle *handle, uint8_t inAddress);

    libusb_context *m_context;
    libusb_device_handle *m_handle;
    uint8_t m_interfaceNumber;
    uint8_t m_inAddress;
    uint8_t m_outAddress;
    std::unique_ptr<QThread> m_readThread;
    std::unique_ptr<UsbConnectionReader> m_reader;
    QQueue<QByteArray> m_reads;
};

#endif // USBMANAGER_H
