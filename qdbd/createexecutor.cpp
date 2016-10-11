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
#include "createexecutor.h"

#include "echoexecutor.h"
#include "handshakeexecutor.h"
#include "libqdb/make_unique.h"
#include "libqdb/protocol/services.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qfile.h>

std::unique_ptr<Executor> createExecutor(Stream *stream, const QByteArray &tagBuffer)
{
    QDataStream tagStream{tagBuffer};
    uint32_t tag;
    tagStream >> tag;

    switch (static_cast<ServiceTag>(tag)) {
    case EchoTag:
        return make_unique<EchoExecutor>(stream);
    case HandshakeTag:
        return make_unique<HandshakeExecutor>(stream);
    default:
        qCritical("Unknown ServiceTag %d in createExecutor", tag);
        return std::unique_ptr<Executor>{};
    }
}
