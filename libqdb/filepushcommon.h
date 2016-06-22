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
#ifndef FILEPUSHCOMMON_H
#define FILEPUSHCOMMON_H

#include <QtGlobal>

#include <cstdint>

enum FilePushPacketType : uint32_t
{
    FilePushOpen = 1,
    FilePushOpened,
    FilePushWrite,
    FilePushWritten,
    FilePushEnd,
    FilePushError,
};

inline
FilePushPacketType toFilePushPacketType(uint32_t x)
{
    switch (static_cast<FilePushPacketType>(x))
    {
    case FilePushOpen:
        return FilePushOpen;
        case FilePushOpened:
        return FilePushOpened;
    case FilePushWrite:
        return FilePushWrite;
    case FilePushWritten:
        return FilePushWritten;
    case FilePushEnd:
        return FilePushEnd;
    case FilePushError:
        return FilePushError;
    default:
        Q_UNREACHABLE();
        return FilePushError;
    }
}

#endif // FILEPUSHCOMMON_H
