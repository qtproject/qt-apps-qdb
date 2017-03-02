/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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
#ifndef NETWORKCONFIGURATIONSERVICE_H
#define NETWORKCONFIGURATIONSERVICE_H

#include "service.h"
class Connection;

class NetworkConfigurationService : public Service
{
    Q_OBJECT
public:
    explicit NetworkConfigurationService(Connection *connection);
    ~NetworkConfigurationService();

    void initialize() override;

    void configure(QString subnet);

signals:
    void response(bool success);

public slots:
    void receive(StreamPacket packet) override;

protected slots:
    void onStreamClosed() override;

private:
    void handleDisconnected();
    void failedResponse();

    Connection *m_connection;
    bool m_responded;
};

#endif // NETWORKCONFIGURATIONSERVICE_H
