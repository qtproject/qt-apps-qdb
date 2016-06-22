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
#ifndef LIBQDB_GLOBAL_H
#define LIBQDB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBQDB_LIBRARY)
#  define LIBQDBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBQDBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBQDB_GLOBAL_H
