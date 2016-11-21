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
#include "usbcommon.h"

#include <QtCore/qloggingcategory.h>
#include <QtGlobal>

#include <libusb.h>

Q_DECLARE_LOGGING_CATEGORY(usbC);

struct LibUsbContext
{
    LibUsbContext()
        : context{nullptr}
    {
        int ret = libusb_init(&context);
        if (ret) {
            qCCritical(usbC) << "Could not initialize libusb";
        }
    }

    ~LibUsbContext()
    {
        if (context)
            libusb_exit(context);
    }

    libusb_context* context;
};

libusb_context *libUsbContext()
{
    static LibUsbContext context;
    return context.context;
}
