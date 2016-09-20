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
#ifndef USBGADGET_H
#define USBGADGET_H

class UsbGadgetReader;
class UsbGadgetWriter;

#include <QtCore/qbytearray.h>
#include <QtCore/qfile.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qqueue.h>
QT_BEGIN_NAMESPACE
class QThread;
QT_END_NAMESPACE

#include <memory>

class UsbGadget : public QIODevice
{
    Q_OBJECT

public:
    UsbGadget();
    virtual ~UsbGadget();

    bool open(OpenMode mode) override;

signals:
    void writeAvailable(QByteArray data);

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

private slots:
    void dataRead(QByteArray data);

private:
    bool openControlEndpoint();
    void startReadThread();
    void startWriteThread();

    QFile m_controlEndpoint;
    // Endpoints are named in line with USB terminology. Out means from host to
    // gadget and in means from gadget to host.
    QFile m_outEndpoint;
    QFile m_inEndpoint;
    std::unique_ptr<QThread> m_readThread;
    std::unique_ptr<QThread> m_writeThread;
    std::unique_ptr<UsbGadgetReader> m_reader;
    std::unique_ptr<UsbGadgetWriter> m_writer;
    QQueue<QByteArray> m_reads;
};

#endif // USBGADGET_H
