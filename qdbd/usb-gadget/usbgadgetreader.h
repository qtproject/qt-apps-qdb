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
#ifndef USBGADGETREADER_H
#define USBGADGETREADER_H

#include <QtCore/qobject.h>
QT_BEGIN_NAMESPACE
class QFile;
QT_END_NAMESPACE

class UsbGadgetReader : public QObject
{
    Q_OBJECT
public:
    UsbGadgetReader(QFile *readEndpoint);

signals:
    void newRead(QByteArray data);

public slots:
    void executeRead();

private:
    QFile *m_readEndpoint;
};

#endif // USBGADGETREADER_H
