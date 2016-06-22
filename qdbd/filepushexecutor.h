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
#ifndef FILEPUSHEXECUTOR_H
#define FILEPUSHEXECUTOR_H

#include "executor.h"
class Stream;

class QByteArray;
class QFile;

#include <memory>

class FilePushExecutor : public Executor
{
public:
    explicit FilePushExecutor(Stream *stream);

public slots:
    void receive(StreamPacket packet) override;

private:
    void openSink(const QString &path);
    void writeToSink(const QByteArray &data);
    void closeSink();

    Stream* m_stream;
    std::unique_ptr<QFile> m_sink;
};

#endif // FILEPUSHEXECUTOR_H
