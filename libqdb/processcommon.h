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
#ifndef PROCESSCOMMON_H
#define PROCESSCOMMON_H

#include <QtGlobal>

#include <cstdint>

enum ProcessPacketType : uint32_t
{
    ProcessStart = 1,
    ProcessStarted,
    ProcessRead,
    ProcessWrite,
    ProcessError,
    ProcessFinished,
};

inline
ProcessPacketType toProcessPacketType(uint32_t x)
{
    switch (static_cast<ProcessPacketType>(x))
    {
    case ProcessStart:
        return ProcessStart;
    case ProcessStarted:
        return ProcessStarted;
    case ProcessRead:
        return ProcessRead;
    case ProcessWrite:
        return ProcessWrite;
    case ProcessError:
        return ProcessError;
    case ProcessFinished:
        return ProcessFinished;
    default:
        Q_UNREACHABLE(); // all possible values are covered
        return ProcessError;
    }
}

#endif // PROCESSCOMMON_H
