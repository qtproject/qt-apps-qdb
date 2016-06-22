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
#ifndef ABSTRACTCONNECTION_H
#define ABSTRACTCONNECTION_H

#include "protocol/qdbmessage.h"
#include "stream.h"
class QdbTransport;

#include <QtCore/qobject.h>
#include <QtCore/qqueue.h>

#include <memory>
#include <unordered_map>

class AbstractConnection : public QObject
{
    Q_OBJECT
public:
    AbstractConnection(QdbTransport *transport, QObject *parent = 0);
    virtual ~AbstractConnection();

    virtual bool initialize();
    virtual void enqueueMessage(const QdbMessage &message) = 0;

public slots:
    virtual void handleMessage() = 0;

protected:
    std::unique_ptr<QdbTransport> m_transport;
    QQueue<QdbMessage> m_outgoingMessages;
    std::unordered_map<StreamId, std::unique_ptr<Stream>> m_streams;
    StreamId m_nextStreamId;
};

#endif // ABSTRACTCONNECTION_H
