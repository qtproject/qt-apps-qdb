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
#ifndef USBGADGETWRITER_H
#define USBGADGETWRITER_H

#include <QtCore/qobject.h>
QT_BEGIN_NAMESPACE
class QFile;
QT_END_NAMESPACE

class UsbGadgetWriter : public QObject
{
    Q_OBJECT
public:
    UsbGadgetWriter(QFile *writeEndpoint);

signals:
    void writeDone(bool success);

public slots:
    void write(QByteArray data);

private:
    QFile *m_writeEndpoint;
};

#endif // USBGADGETWRITER_H
