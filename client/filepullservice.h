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
#ifndef FILEPULLSERVICE_H
#define FILEPULLSERVICE_H

class Connection;
#include "service.h"

#include <QtCore/qstring.h>
QT_BEGIN_NAMESPACE
class QByteArray;
class QFile;
QT_END_NAMESPACE

#include <memory>

class FilePullService : public Service
{
    Q_OBJECT
public:
    explicit FilePullService(Connection *connection);
    ~FilePullService();

    void initialize() override;

    bool pull(const QString &devicePath, const QString &hostPath);

signals:
    void pulled();
    void error(QString error);

public slots:
    void receive(StreamPacket packet) override;

private:
    void openSink();
    void writeToSink(const QByteArray &data);
    void closeSink();

    Connection *m_connection;
    QString m_hostPath;
    QString m_devicePath;
    std::unique_ptr<QFile> m_sink;
};

#endif // FILEPULLSERVICE_H
