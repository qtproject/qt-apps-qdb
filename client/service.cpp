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
#include "service.h"

#include "stream.h"

#include <QtCore/qdebug.h>

Service::Service()
    : m_stream{nullptr}
{

}

void Service::streamCreated(Stream *stream)
{
    if (stream) {
        m_stream = stream;
        connect(m_stream, &Stream::packetAvailable, this, &Service::receive);
        connect(m_stream, &Stream::closed, this, &Service::onStreamClosed);
        emit initialized();
    }
}

void Service::onStreamClosed()
{
    m_stream = nullptr;
}
