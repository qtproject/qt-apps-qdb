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
#ifndef USBGADGET_H
#define USBGADGET_H

class UsbGadgetControl;
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
    virtual ~UsbGadget() override;

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
    void startControlThread();
    void startReadThread();
    void startWriteThread();

    QFile m_controlEndpoint;
    // Endpoints are named in line with USB terminology. Out means from host to
    // gadget and in means from gadget to host.
    QFile m_outEndpoint;
    QFile m_inEndpoint;
    std::unique_ptr<QThread> m_controlThread;
    std::unique_ptr<QThread> m_readThread;
    std::unique_ptr<QThread> m_writeThread;
    std::unique_ptr<UsbGadgetControl> m_control;
    std::unique_ptr<UsbGadgetReader> m_reader;
    std::unique_ptr<UsbGadgetWriter> m_writer;
    QQueue<QByteArray> m_reads;
};

#endif // USBGADGET_H
