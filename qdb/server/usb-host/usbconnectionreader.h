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
#ifndef USBCONNECTIONREADER_H
#define USBCONNECTIONREADER_H

#include <QtCore/qobject.h>

#include <stdint.h>

struct libusb_device_handle;

class UsbConnectionReader : public QObject
{
    Q_OBJECT
public:
    UsbConnectionReader(libusb_device_handle *handle, uint8_t inAddress);

signals:
    void newRead(QByteArray data);

public slots:
    void executeRead();

private:
    libusb_device_handle *m_handle;
    uint8_t m_inAddress;
    int m_errorCount;
};

#endif // USBCONNECTIONREADER_H
