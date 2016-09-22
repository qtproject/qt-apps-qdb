/******************************************************************************
 **
 ** Copyright (C) 2016 The Qt Company Ltd.
 ** Contact: http://www.qt.io/licensing/
 **
 ** This file is part of config tests for the Qt Debug Bridge.
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
#include <libusb.h>

int main()
{
    int ret = libusb_init(nullptr);
    libusb_exit(nullptr);
    return ret;
}
