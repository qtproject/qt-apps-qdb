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
#ifndef FILEPUSHSERVICE_H
#define FILEPUSHSERVICE_H

class Connection;
#include "service.h"

class QByteArray;
class QFile;
#include <QtCore/qstring.h>

#include <memory>

class FilePushService : public Service
{
    Q_OBJECT
public:
    explicit FilePushService(Connection *connection);
    ~FilePushService();

    void initialize() override;

    bool push(const QString &hostPath, const QString &devicePath);

signals:
    void pushed();
    void error(QString error);

public slots:
    void receive(StreamPacket packet) override;

private:
    bool openSource();
    void transferBlock();
    void endTransfer();

    Connection *m_connection;
    QString m_hostPath;
    QString m_devicePath;
    std::unique_ptr<QFile> m_source;
    bool m_transferring;
};

#endif // FILEPUSHSERVICE_H
