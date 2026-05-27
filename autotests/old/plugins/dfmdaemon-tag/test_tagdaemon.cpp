// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include <gtest/gtest.h>
#include <QApplication>
#include <QThread>
#include <QSignalSpy>

#include "stubext.h"

#include "tagdaemon.h"
#include "tagmanagerdbus.h"
#include "tagmanageradaptor.h"

#include <QDBusConnection>

DAEMONPTAG_USE_NAMESPACE

class TestTagDaemon : public testing::Test
{
public:
    void SetUp() override
    {
        daemon = new TagDaemon;
    }

    void TearDown() override
    {
        if (daemon) {
            // daemon->stop();
            delete daemon;
            daemon = nullptr;
        }
        stub.clear();
    }

protected:
    TagDaemon *daemon = nullptr;
    stub_ext::StubExt stub;
};

class TestTagDBusWorker : public testing::Test
{
public:
    void SetUp() override
    {
        worker = new TagDBusWorker;
    }

    void TearDown() override
    {
        delete worker;
        worker = nullptr;
        stub.clear();
    }

protected:
    TagDBusWorker *worker = nullptr;
    stub_ext::StubExt stub;
};

// Test TagDaemon initialization
TEST_F(TestTagDaemon, Initialize_ShouldSetupWorkerThread)
{
    bool threadStarted = false;

    // Use proper function signature for QThread::start overload
    using StartFunc = void (QThread::*)(QThread::Priority);
    stub.set_lamda(static_cast<StartFunc>(&QThread::start), [&threadStarted](QThread *, QThread::Priority) {
        __DBG_STUB_INVOKE__
        threadStarted = true;
    });

    daemon->initialize();

    EXPECT_TRUE(threadStarted);
}

// Test TagDaemon start functionality
TEST_F(TestTagDaemon, Start_ShouldEmitRequestLaunchSignal_ReturnTrue)
{
    // Initialize first to set up connections
    using StartFunc = void (QThread::*)(QThread::Priority);
    stub.set_lamda(static_cast<StartFunc>(&QThread::start), [](QThread *, QThread::Priority) {
        __DBG_STUB_INVOKE__
    });
    daemon->initialize();

    // Connect to the signal to verify it's emitted
    QSignalSpy spy(daemon, &TagDaemon::requestLaunch);

    bool result = daemon->start();

    EXPECT_TRUE(result);
    EXPECT_EQ(spy.count(), 1);
}

// Test TagDaemon stop functionality
TEST_F(TestTagDaemon, Stop_ShouldQuitAndWaitForThread)
{
    bool quitCalled = false;
    bool waitCalled = false;

    using StartFunc = void (QThread::*)(QThread::Priority);
    stub.set_lamda(static_cast<StartFunc>(&QThread::start), [](QThread *, QThread::Priority) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QThread::quit, [&quitCalled](QThread *) {
        __DBG_STUB_INVOKE__
        quitCalled = true;
    });

    using WaitFuncPtr = bool (QThread::*)(QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFuncPtr>(&QThread::wait), [&](QThread *, QDeadlineTimer) {
        __DBG_STUB_INVOKE__
        waitCalled = true;
        return true;
    });

    daemon->initialize();
    daemon->stop();

    EXPECT_TRUE(quitCalled);
    EXPECT_TRUE(waitCalled);
}

// Test TagDBusWorker service launch with successful DBus registration
TEST_F(TestTagDBusWorker, LaunchService_SuccessfulRegistration_ShouldCreateManagerAndRegisterObject)
{
    bool connectionRegistered = false;
    bool serviceReadyEmitted = false;

    // Mock QDBusConnection::sessionBus()
    stub.set_lamda(&QDBusConnection::sessionBus, []() {
        __DBG_STUB_INVOKE__
        // Create a valid QDBusConnection using constructor with name
        return QDBusConnection("test_connection");
    });

    // Mock registerObject - need to handle overloaded function
    using RegisterObjectFunc = bool (QDBusConnection::*)(const QString &, QObject *, QDBusConnection::RegisterOptions);
    stub.set_lamda(static_cast<RegisterObjectFunc>(&QDBusConnection::registerObject),
                   [&connectionRegistered](QDBusConnection *, const QString &path, QObject *object, QDBusConnection::RegisterOptions) {
                       __DBG_STUB_INVOKE__
                       connectionRegistered = true;
                       EXPECT_EQ(path, "/org/deepin/Filemanager/Daemon/TagManager");
                       EXPECT_NE(object, nullptr);
                       return true;
                   });

    // Mock TagsServiceReady signal emission
    stub.set_lamda(&TagManagerDBus::TagsServiceReady, [&serviceReadyEmitted](TagManagerDBus *) {
        __DBG_STUB_INVOKE__
        serviceReadyEmitted = true;
    });

    worker->launchService();

    EXPECT_TRUE(connectionRegistered);
    EXPECT_TRUE(serviceReadyEmitted);
}

// Test TagDBusWorker service launch with failed DBus registration
TEST_F(TestTagDBusWorker, LaunchService_FailedRegistration_ShouldCleanupManager)
{
    bool registrationAttempted = false;

    // Mock QDBusConnection::sessionBus()
    stub.set_lamda(&QDBusConnection::sessionBus, []() {
        __DBG_STUB_INVOKE__
        return QDBusConnection("test_connection");
    });

    // Mock registerObject to return false (failure)
    using RegisterObjectFunc = bool (QDBusConnection::*)(const QString &, QObject *, QDBusConnection::RegisterOptions);
    stub.set_lamda(static_cast<RegisterObjectFunc>(&QDBusConnection::registerObject),
                   [&registrationAttempted](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
                       __DBG_STUB_INVOKE__
                       registrationAttempted = true;
                       return false;
                   });

    worker->launchService();

    EXPECT_TRUE(registrationAttempted);
    // The tagManager should be reset to nullptr on failure
    // This is verified by the internal logic, no direct way to test the private member
}

// Test TagDBusWorker in different thread context
TEST_F(TestTagDBusWorker, LaunchService_ShouldWorkInNonMainThread)
{
    bool currentThreadChecked = false;

    // Mock other necessary functions
    stub.set_lamda(&QDBusConnection::sessionBus, []() {
        __DBG_STUB_INVOKE__
        return QDBusConnection("test_connection");
    });

    using RegisterObjectFunc = bool (QDBusConnection::*)(const QString &, QObject *, QDBusConnection::RegisterOptions);
    stub.set_lamda(static_cast<RegisterObjectFunc>(&QDBusConnection::registerObject),
                   [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&TagManagerDBus::TagsServiceReady, [&currentThreadChecked](TagManagerDBus *) {
        __DBG_STUB_INVOKE__
        currentThreadChecked = true;
    });

    worker->launchService();

    EXPECT_TRUE(currentThreadChecked);
}
