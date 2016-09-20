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
#ifndef QDBTRANSPORT_H
#define QDBTRANSPORT_H

#include "protocol/qdbmessage.h"

#include <QtCore/qobject.h>
QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE

#include <memory>


class QdbTransport : public QObject
{
    Q_OBJECT
public:
    /*! QdbTransport takes ownership of the passed QIODevice. */
    QdbTransport(QIODevice *io);
    ~QdbTransport();

    bool open();

    bool send(const QdbMessage &message);
    QdbMessage receive();

signals:
    void messageAvailable();

private:
    std::unique_ptr<QIODevice> m_io;
};

#endif // QDBTRANSPORT_H
