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
#ifndef SERVICE_H
#define SERVICE_H

class Stream;
#include "libqdb/streampacket.h"

#include <QtCore/qobject.h>

class Service : public QObject
{
    Q_OBJECT
public:
    Service();
    virtual ~Service() { }

    virtual void initialize() = 0;
    virtual void streamCreated(Stream *stream);

signals:
    void initialized();

public slots:
    virtual void receive(StreamPacket) = 0;

protected slots:
    virtual void onStreamClosed();

protected:
    Stream *m_stream;
};

#endif // SERVICE_H
