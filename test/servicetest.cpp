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
#include "../client/connection.h"
#include "../client/filepullservice.h"
#include "../client/filepushservice.h"
#include "../client/processservice.h"
#include "../client/echoservice.h"
#include "../utils/make_unique.h"
#include "usb/usbconnection.h"
#include "protocol/qdbtransport.h"
#include "protocol/services.h"

#include <QtCore/qdebug.h>
#include <QtCore/qregularexpression.h>
#include <QtCore/qtimer.h>
#include <QtTest/QtTest>

const int testTimeout = 500; // in milliseconds

// Helper to initialize Connection in testcases
struct ConnectionContext
{
    ConnectionContext()
        : connection{new QdbTransport{new UsbConnection{}}}
    {
        QVERIFY(connection.initialize());

        connection.connect();
    }
    Connection connection;
};

class ServiceTest : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void echo();
    void processOutput();
    void processMultipleOutput();
    void processErrorCode();
    void processNonExistent();
    void processCrash();
    void processInput();
    void processMultipleInput();
    void filePush();
    void filePushNonexistent();
    void filePull();
    void filePullNonexistent();
    void filePullToUnopenable();
};

const QString pushPullFileName = "qdbtestfile1";
static QByteArray pushPullFileContents = "abcd\nefgh\n";
const QString nonexistentFileName{"qdbtestfile2"};

void ServiceTest::initTestCase()
{
    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    qRegisterMetaType<QProcess::ExitStatus>("QProcess::ExitStatus");
}

void ServiceTest::echo()
{
    ConnectionContext ctx;

    EchoService echo{&ctx.connection};
    connect(&echo, &EchoService::initialized, [&]() {
        echo.send("ABCD");
    });
    QSignalSpy spy{&echo, &EchoService::echo};

    echo.initialize();

    spy.wait(testTimeout);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy[0][0].toByteArray(), QByteArray{"ABCD"});
}

void ServiceTest::processOutput()
{
    ConnectionContext ctx;

    QByteArray output;

    ProcessService processService{&ctx.connection};
    connect(&processService, &ProcessService::executionError, [](QProcess::ProcessError error) {
        qDebug() << "Command not run, error:" << error;
        QFAIL("Command was not run successfully");
    });
    connect(&processService, &Service::initialized, [&]() {
        processService.execute("echo", {"ABCD"});
    });
    connect(&processService, &ProcessService::readyRead, [&]() {
        output.append(processService.read());
    });
    QSignalSpy spy{&processService, &ProcessService::executed};

    processService.initialize();

    spy.wait(testTimeout);
    QCOMPARE(spy.count(), 1);
    auto exitCode = spy[0][0].toInt();
    QProcess::ExitStatus exitStatus = spy[0][1].value<QProcess::ExitStatus>();
    auto finalOutput = spy[0][2].toByteArray();
    output.append(finalOutput);
    QCOMPARE(exitCode, 0);
    QCOMPARE(exitStatus, QProcess::NormalExit);
    QCOMPARE(output, QByteArray{"ABCD\n"});
}

void ServiceTest::processMultipleOutput()
{
    ConnectionContext ctx;

    QByteArray output;

    ProcessService processService{&ctx.connection};
    connect(&processService, &ProcessService::executionError, [](QProcess::ProcessError error) {
        qDebug() << "Command not run, error:" << error;
        QFAIL("Command was not run successfully");
    });

    connect(&processService, &Service::initialized, [&]() {
        processService.execute("sh", {"-c", "echo abcd && sleep 1 && echo defg"});
    });
    connect(&processService, &ProcessService::readyRead, [&]() {
        output.append(processService.read());
    });
    QSignalSpy readyReadSpy{&processService, &ProcessService::readyRead};
    QSignalSpy executedSpy{&processService, &ProcessService::executed};

    processService.initialize();

    executedSpy.wait(1000 + testTimeout);
    QCOMPARE(executedSpy.count(), 1);
    // In principle there could be only one (or more than two) readyRead, but in
    // practice the above command seems split into two outputs and otherwise
    // this test is not fulfilling its purpose.
    QCOMPARE(readyReadSpy.count(), 2);
    auto exitCode = executedSpy[0][0].toInt();
    QProcess::ExitStatus exitStatus = executedSpy[0][1].value<QProcess::ExitStatus>();
    auto finalOutput = executedSpy[0][2].toByteArray();
    output.append(finalOutput);
    QCOMPARE(exitCode, 0);
    QCOMPARE(exitStatus, QProcess::NormalExit);
    QCOMPARE(output, QByteArray{"abcd\ndefg\n"});
}

void ServiceTest::processErrorCode()
{
    ConnectionContext ctx;

    ProcessService processService{&ctx.connection};
    connect(&processService, &ProcessService::executionError, [](QProcess::ProcessError error) {
        qDebug() << "Command not run, error:" << error;
        QFAIL("Command was not run successfully");
    });
    connect(&processService, &Service::initialized, [&]() {
        processService.execute("test", {"-z", "ABCD"});
    });
    QSignalSpy spy{&processService, &ProcessService::executed};

    processService.initialize();

    spy.wait(testTimeout);
    QCOMPARE(spy.count(), 1);
    auto exitCode = spy[0][0].toInt();
    QProcess::ExitStatus exitStatus = spy[0][1].value<QProcess::ExitStatus>();
    auto output = spy[0][2].toString();
    QCOMPARE(exitCode, 1);
    QCOMPARE(exitStatus, QProcess::NormalExit);
    QCOMPARE(output, QString{""});
}

void ServiceTest::processNonExistent()
{
    ConnectionContext ctx;

    ProcessService processService{&ctx.connection};
    connect(&processService, &ProcessService::executed, [](int, QProcess::ExitStatus, QString) {
        QFAIL("Command  was unexpectedly run successfully");
    });
    connect(&processService, &Service::initialized, [&]() {
        processService.execute("lsfdajlvaie", {});
    });
    QSignalSpy spy{&processService, &ProcessService::executionError};

    processService.initialize();

    spy.wait(testTimeout);
    QCOMPARE(spy.count(), 1);
    auto error = spy[0][0].value<QProcess::ProcessError>();
    QCOMPARE(error, QProcess::FailedToStart);
}

void ServiceTest::processCrash()
{
    ConnectionContext ctx;

    ProcessService processService{&ctx.connection};
    connect(&processService, &Service::initialized, [&]() {
        // Crash the process by having it send SIGSEGV to itself
        processService.execute("sh", {"-c", "kill -SEGV $$"});
    });
    QSignalSpy errorSpy{&processService, &ProcessService::executionError};
    QSignalSpy executedSpy{&processService, &ProcessService::executed};

    processService.initialize();

    errorSpy.wait(testTimeout);
    QCOMPARE(errorSpy.count(), 1);
    auto error = errorSpy[0][0].value<QProcess::ProcessError>();
    QCOMPARE(error, QProcess::Crashed);

    executedSpy.wait(testTimeout);
    QCOMPARE(executedSpy.count(), 1);
    auto exitCode = executedSpy[0][0].toInt();
    auto exitStatus = executedSpy[0][1].value<QProcess::ExitStatus>();
    auto output = executedSpy[0][2].toString();
    QCOMPARE(exitCode, 11); // 11 for segfault
    QCOMPARE(exitStatus, QProcess::CrashExit);
    QCOMPARE(output, QString{""});
}

void ServiceTest::processInput()
{
    ConnectionContext ctx;

    QByteArray output;

    ProcessService processService{&ctx.connection};
    connect(&processService, &ProcessService::executionError, [](QProcess::ProcessError error) {
        qDebug() << "Command not run, error:" << error;
        QFAIL("Command was not run successfully");
    });
    connect(&processService, &Service::initialized, [&]() {
        processService.execute("sh", {"-c", "read input; echo $input"});
    });
    connect(&processService, &ProcessService::readyRead, [&]() {
        output.append(processService.read());
    });
    connect(&processService, &ProcessService::started, [&]() {
       processService.write("abcd\n");
    });
    QSignalSpy spy{&processService, &ProcessService::executed};

    processService.initialize();

    spy.wait(testTimeout);
    QCOMPARE(spy.count(), 1);
    auto exitCode = spy[0][0].toInt();
    QProcess::ExitStatus exitStatus = spy[0][1].value<QProcess::ExitStatus>();
    auto finalOutput = spy[0][2].toByteArray();
    output.append(finalOutput);
    QCOMPARE(exitCode, 0);
    QCOMPARE(exitStatus, QProcess::NormalExit);
    QCOMPARE(output, QByteArray{"abcd\n"});
}

void ServiceTest::processMultipleInput()
{
    ConnectionContext ctx;

    QByteArray output;

    ProcessService processService{&ctx.connection};
    connect(&processService, &ProcessService::executionError, [](QProcess::ProcessError error) {
        qDebug() << "Command not run, error:" << error;
        QFAIL("Command was not run successfully");
    });
    connect(&processService, &Service::initialized, [&]() {
        processService.execute("sh", {"-c", "for i in {1..2}; do read input; echo $input; done"});
    });
    connect(&processService, &ProcessService::readyRead, [&]() {
        output.append(processService.read());
    });
    connect(&processService, &ProcessService::started, [&]() {
       processService.write("abcd\n");
       processService.write("efgh\n");
    });
    QSignalSpy spy{&processService, &ProcessService::executed};

    processService.initialize();

    spy.wait(testTimeout);
    QCOMPARE(spy.count(), 1);
    auto exitCode = spy[0][0].toInt();
    QProcess::ExitStatus exitStatus = spy[0][1].value<QProcess::ExitStatus>();
    auto finalOutput = spy[0][2].toByteArray();
    output.append(finalOutput);
    QCOMPARE(exitCode, 0);
    QCOMPARE(exitStatus, QProcess::NormalExit);
    QCOMPARE(output, QByteArray{"abcd\nefgh\n"});
}

void ServiceTest::filePush()
{
    ConnectionContext ctx;

    // Write source file
    QFile source{pushPullFileName};
    QVERIFY(source.open(QIODevice::WriteOnly));
    source.write(pushPullFileContents);
    source.close();

    // Push source file to device (it's cleaned up in filePullToUnopenable())
    FilePushService filePushService{&ctx.connection};
    connect(&filePushService, &FilePushService::error, [](QString error) {
        qCritical() << error;
        QFAIL("Error while pushing file.");
    });
    connect(&filePushService, &Service::initialized, [&]() {
        filePushService.push(pushPullFileName, pushPullFileName);
    });
    QSignalSpy pushSpy{&filePushService, &FilePushService::pushed};

    filePushService.initialize();

    pushSpy.wait(testTimeout);
    QCOMPARE(pushSpy.count(), 1);

    // Remove source file
    source.remove();

    // Check contents on device
    QByteArray output;

    ProcessService processService{&ctx.connection};
    connect(&processService, &ProcessService::executionError, [](QProcess::ProcessError error) {
        qDebug() << "Command not run, error:" << error;
        QFAIL("Command was not run successfully");
    });
    connect(&processService, &Service::initialized, [&]() {
        processService.execute("cat", {pushPullFileName});
    });
    connect(&processService, &ProcessService::readyRead, [&]() {
        output.append(processService.read());
    });
    QSignalSpy processSpy{&processService, &ProcessService::executed};

    processService.initialize();

    processSpy.wait(testTimeout);
    QCOMPARE(processSpy.count(), 1);
    auto exitCode = processSpy[0][0].toInt();
    QProcess::ExitStatus exitStatus = processSpy[0][1].value<QProcess::ExitStatus>();
    auto finalOutput = processSpy[0][2].toByteArray();
    output.append(finalOutput);
    QCOMPARE(exitCode, 0);
    QCOMPARE(exitStatus, QProcess::NormalExit);
    QCOMPARE(output, pushPullFileContents);
}

void ServiceTest::filePushNonexistent()
{
    QVERIFY(!QFile::exists(nonexistentFileName));

    ConnectionContext ctx;

    FilePushService filePushService{&ctx.connection};
    connect(&filePushService, &FilePushService::pushed, []() {
        QFAIL("Unexpectedly succeeded pushing nonexistent file.");
    });
    connect(&filePushService, &Service::initialized, [&]() {
        filePushService.push(nonexistentFileName, nonexistentFileName);
    });
    QSignalSpy spy{&filePushService, &FilePushService::error};

    filePushService.initialize();

    spy.wait(testTimeout);
    QCOMPARE(spy.count(), 1);
    QRegularExpression regexp{"^Could not open.+host$"};
    auto errorMessage = spy[0][0].toString();
    QVERIFY(regexp.match(errorMessage).hasMatch());
}

// This test relies on the file pushed in filePush()
void ServiceTest::filePull()
{
    ConnectionContext ctx;

    // Pull source file from device
    FilePullService filePullService{&ctx.connection};
    connect(&filePullService, &FilePullService::error, [](QString error) {
        qCritical() << error;
        QFAIL("Error while pulling file");
    });
    connect(&filePullService, &Service::initialized, [&]() {
        filePullService.pull(pushPullFileName, pushPullFileName);
    });
    QSignalSpy pullSpy{&filePullService, &FilePullService::pulled};

    filePullService.initialize();

    pullSpy.wait(testTimeout);
    QCOMPARE(pullSpy.count(), 1);

    // Check contents
    QFile sink{pushPullFileName};
    QVERIFY(sink.open(QIODevice::ReadOnly));
    auto contents = sink.readAll();
    QCOMPARE(contents, pushPullFileContents);

    sink.close();
    sink.remove();
}

void ServiceTest::filePullNonexistent()
{
    const QString nonexistentFileName{"qdbtestfile2"};

    ConnectionContext ctx;

    // Pull source file from device
    FilePullService filePullService{&ctx.connection};
    connect(&filePullService, &FilePullService::pulled, []() {
        QFAIL("Unexpectedly succeeded pulling nonexistent file");
    });
    connect(&filePullService, &Service::initialized, [&]() {
        filePullService.pull(nonexistentFileName, nonexistentFileName);
    });
    QSignalSpy spy{&filePullService, &FilePullService::error};

    filePullService.initialize();

    spy.wait(testTimeout);
    QCOMPARE(spy.count(), 1);
    QRegularExpression regexp{"^Could not open.+device$"};
    auto errorMessage = spy[0][0].toString();
    QVERIFY(regexp.match(errorMessage).hasMatch());
}

// This test relies on the file pushed in filePush() and removes it in the end
void ServiceTest::filePullToUnopenable()
{
    const QString fileName{"qdbtestfile2"};

    QFile blocker{fileName};
    blocker.open(QIODevice::WriteOnly);
    blocker.setPermissions(QFileDevice::ReadUser);
    blocker.close();

    ConnectionContext ctx;

    // Pull source file from device
    FilePullService filePullService{&ctx.connection};
    connect(&filePullService, &FilePullService::pulled, []() {
        QFAIL("Unexpectedly succeeded pulling into file that can't be written to");
    });
    connect(&filePullService, &Service::initialized, [&]() {
        filePullService.pull(pushPullFileName, fileName);
    });
    QSignalSpy spy{&filePullService, &FilePullService::error};

    filePullService.initialize();

    spy.wait(testTimeout);

    blocker.remove();

    QCOMPARE(spy.count(), 1);
    QRegularExpression regexp{"^Could not open.+host$"};
    auto errorMessage = spy[0][0].toString();
    QVERIFY(regexp.match(errorMessage).hasMatch());

    // Remove file from device
    ProcessService processService{&ctx.connection};
    connect(&processService, &ProcessService::executionError, [](QProcess::ProcessError error) {
        qDebug() << "Command not run, error:" << error;
        QFAIL("Command was not run successfully");
    });
    connect(&processService, &Service::initialized, [&]() {
        processService.execute("rm", {pushPullFileName});
    });
    QSignalSpy processSpy{&processService, &ProcessService::executed};

    processService.initialize();

    processSpy.wait(testTimeout);
    QCOMPARE(processSpy.count(), 1);
    auto exitCode = processSpy[0][0].toInt();
    QProcess::ExitStatus exitStatus = processSpy[0][1].value<QProcess::ExitStatus>();
    QCOMPARE(exitCode, 0);
    QCOMPARE(exitStatus, QProcess::NormalExit);
}

QTEST_GUILESS_MAIN(ServiceTest)
#include "servicetest.moc"
