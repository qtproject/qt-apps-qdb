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
#ifndef FILEPULLCOMMON_H
#define FILEPULLCOMMON_H

#include <QtGlobal>

#include <cstdint>

enum FilePullPacketType : uint32_t
{
    FilePullOpen = 1,
    FilePullOpened,
    FilePullRead,
    FilePullWasRead,
    FilePullEnd,
    FilePullError,
};

inline
FilePullPacketType toFilePullPacketType(uint32_t x)
{
    switch (static_cast<FilePullPacketType>(x))
    {
    case FilePullOpen:
        return FilePullOpen;
        case FilePullOpened:
        return FilePullOpened;
    case FilePullRead:
        return FilePullRead;
    case FilePullWasRead:
        return FilePullWasRead;
    case FilePullEnd:
        return FilePullEnd;
    case FilePullError:
        return FilePullError;
    default:
        Q_UNREACHABLE();
        return FilePullError;
    }
}

#endif // FILEPULLCOMMON_H
