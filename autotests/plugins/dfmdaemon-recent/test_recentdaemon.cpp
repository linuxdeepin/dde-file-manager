// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "recentdaemon.h"
#include "recentmanagerdbus.h"
#include "recentmanageradaptor.h"
#include "recentmanager.h"
#include "serverplugin_recentmanager_global.h"

#include <QDBusConnection>
#include <QSignalSpy>

SERVERRECENTMANAGER_USE_NAMESPACE
using RegisterObjectFuncPtr = bool (QDBusConnection::*)(const QString &, QObject *, QDBusConnection::RegisterOptions);

class UT_RecentDaemon : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        daemon = new RecentDaemon();
    }

    virtual void TearDown() override
    {
        delete daemon;
        daemon = nullptr;
        stub.clear();
    }

protected:
    RecentDaemon *daemon { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_RecentDaemon, constructor_InitializesCorrectly)
{
    EXPECT_NE(daemon, nullptr);
}

TEST_F(UT_RecentDaemon, initialize_RegistersMetaTypeAndInitializesManager)
{
    bool managerInitialized = false;

    // Mock RecentManager::instance and initialize
    stub.set_lamda(&RecentManager::instance, [&]() -> RecentManager & {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::initialize, [&](RecentManager *) {
        __DBG_STUB_INVOKE__
        managerInitialized = true;
    });

    daemon->initialize();

    EXPECT_TRUE(managerInitialized);
}

TEST_F(UT_RecentDaemon, start_WithSuccessfulServiceRegistration_ReturnsTrue)
{
    bool serviceRegistered = false;
    bool objectRegistered = false;
    bool watchStarted = false;

    // Mock QDBusConnection::sessionBus
    stub.set_lamda(&QDBusConnection::sessionBus, []() {
        __DBG_STUB_INVOKE__
        return QDBusConnection("tets");
    });

    // Mock service registration success
    stub.set_lamda(&QDBusConnection::registerService, [&](QDBusConnection *, const QString &serviceName) {
        __DBG_STUB_INVOKE__
        serviceRegistered = (serviceName == "org.deepin.Filemanager.Daemon");
        return true;
    });

    // Mock object registration success
    stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject), [&](QDBusConnection *, const QString &path, QObject *, QDBusConnection::RegisterOptions) {
        __DBG_STUB_INVOKE__
        objectRegistered = (path == "/org/deepin/Filemanager/Daemon/RecentManager");

        return true;
    });

    // Mock RecentManager operations
    stub.set_lamda(&RecentManager::instance, []() -> RecentManager & {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::startWatch, [&](RecentManager *) {
        __DBG_STUB_INVOKE__
        watchStarted = true;
    });

    bool result = daemon->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(serviceRegistered);
    EXPECT_TRUE(objectRegistered);
    EXPECT_TRUE(watchStarted);
}

// TEST_F(UT_RecentDaemon, start_WithServiceRegistrationFailure_ExitsApplication)
// {
//     bool exitCalled = false;
//     int exitCode = 0;

//     // Mock QDBusConnection::sessionBus
//     stub.set_lamda(&QDBusConnection::sessionBus, []() {
//         __DBG_STUB_INVOKE__
//         return QDBusConnection("test");
//     });

//     // Mock service registration failure
//     stub.set_lamda(&QDBusConnection::registerService, [](QDBusConnection *, const QString &) {
//         __DBG_STUB_INVOKE__
//         return false;
//     });

//     // Mock ::exit to capture the call
//     stub.set_lamda(::exit, [&](int code) {
//         __DBG_STUB_INVOKE__
//         exitCalled = true;
//         exitCode = code;
//         // Don't actually exit in test
//     });

//     daemon->start();

//     EXPECT_TRUE(exitCalled);
//     EXPECT_EQ(exitCode, EXIT_FAILURE);
// }

TEST_F(UT_RecentDaemon, start_WithObjectRegistrationFailure_ReturnsFalse)
{
    bool serviceRegistered = false;
    bool objectRegistrationAttempted = false;

    // Mock QDBusConnection::sessionBus
    stub.set_lamda(&QDBusConnection::sessionBus, []() {
        __DBG_STUB_INVOKE__
        return QDBusConnection("test");
    });

    // Mock service registration success
    stub.set_lamda(&QDBusConnection::registerService, [&](QDBusConnection *, const QString &) {
        __DBG_STUB_INVOKE__
        serviceRegistered = true;
        return true;
    });

    // Mock object registration failure
    stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject),
                   [&](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
                       __DBG_STUB_INVOKE__
                       objectRegistrationAttempted = true;
                       return false;  // Return false to simulate registration failure
                   });
    bool result = daemon->start();

    EXPECT_FALSE(result);
    EXPECT_TRUE(serviceRegistered);
    EXPECT_TRUE(objectRegistrationAttempted);
}

TEST_F(UT_RecentDaemon, start_CreatesRecentManagerDBusInstance)
{
    bool recentManagerCreated = false;

    // Mock QDBusConnection operations
    stub.set_lamda(&QDBusConnection::sessionBus, []() {
        __DBG_STUB_INVOKE__
        return QDBusConnection("");
    });

    stub.set_lamda(&QDBusConnection::registerService, [](QDBusConnection *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject),
                   [&](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
                       __DBG_STUB_INVOKE__
                       recentManagerCreated = true;
                       return true;
                   });

    // Mock RecentManager operations
    stub.set_lamda(&RecentManager::instance, []() -> RecentManager & {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::startWatch, [](RecentManager *) {
        __DBG_STUB_INVOKE__
    });

    daemon->start();

    EXPECT_TRUE(recentManagerCreated);
}

TEST_F(UT_RecentDaemon, start_WithValidSetup_StartsFileWatching)
{
    bool watchStarted = false;

    // Mock all DBus operations to succeed
    stub.set_lamda(&QDBusConnection::sessionBus, []() {
        __DBG_STUB_INVOKE__
        return QDBusConnection("test");
    });

    stub.set_lamda(&QDBusConnection::registerService, [](QDBusConnection *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject),
                   [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    // Mock RecentManager operations
    stub.set_lamda(&RecentManager::instance, []() -> RecentManager & {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::startWatch, [&](RecentManager *) {
        __DBG_STUB_INVOKE__
        watchStarted = true;
    });

    bool result = daemon->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(watchStarted);
}

TEST_F(UT_RecentDaemon, stop_CallsManagerFinalize)
{
    bool managerFinalized = false;

    // Mock RecentManager operations
    stub.set_lamda(&RecentManager::instance, []() -> RecentManager & {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::finalize, [&](RecentManager *) {
        __DBG_STUB_INVOKE__
        managerFinalized = true;
    });

    daemon->stop();

    EXPECT_TRUE(managerFinalized);
}

TEST_F(UT_RecentDaemon, start_RegistersCorrectDBusService)
{
    QString registeredService;
    bool serviceRegistered = false;

    // Mock QDBusConnection::sessionBus
    stub.set_lamda(&QDBusConnection::sessionBus, []() {
        __DBG_STUB_INVOKE__
        return QDBusConnection("test");
    });

    // Capture service registration details
    stub.set_lamda(&QDBusConnection::registerService, [&](QDBusConnection *, const QString &serviceName) {
        __DBG_STUB_INVOKE__
        registeredService = serviceName;
        serviceRegistered = true;
        return true;
    });

    stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject),
                   [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&RecentManager::instance, []() -> RecentManager & {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::startWatch, [](RecentManager *) {
        __DBG_STUB_INVOKE__
    });

    daemon->start();

    EXPECT_TRUE(serviceRegistered);
    EXPECT_EQ(registeredService, "org.deepin.Filemanager.Daemon");
}

TEST_F(UT_RecentDaemon, start_RegistersCorrectDBusObject)
{
    QString registeredObjectPath;
    bool objectRegistered = false;

    // Mock QDBusConnection operations
    stub.set_lamda(&QDBusConnection::sessionBus, []() {
        __DBG_STUB_INVOKE__
        return QDBusConnection("test");
    });

    stub.set_lamda(&QDBusConnection::registerService, [](QDBusConnection *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Capture object registration details
    stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject),
                   [&](QDBusConnection *, const QString &path, QObject *object, QDBusConnection::RegisterOptions) {
                       __DBG_STUB_INVOKE__
                       registeredObjectPath = path;
                       objectRegistered = (object != nullptr);
                       return true;
                   });

    stub.set_lamda(&RecentManager::instance, []() -> RecentManager & {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::startWatch, [](RecentManager *) {
        __DBG_STUB_INVOKE__
    });

    daemon->start();

    EXPECT_TRUE(objectRegistered);
    EXPECT_EQ(registeredObjectPath, "/org/deepin/Filemanager/Daemon/RecentManager");
}

TEST_F(UT_RecentDaemon, lifecycle_InitializeStartStop_WorksCorrectly)
{
    bool initialized = false;
    bool started = false;
    bool stopped = false;

    stub.set_lamda(&RecentManager::instance, []() -> RecentManager & {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::initialize, [&](RecentManager *) {
        __DBG_STUB_INVOKE__
        initialized = true;
    });

    // Mock start operations
    stub.set_lamda(&QDBusConnection::sessionBus, []() {
        __DBG_STUB_INVOKE__
        return QDBusConnection("test");
    });

    stub.set_lamda(&QDBusConnection::registerService, [](QDBusConnection *, const QString &) {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(static_cast<RegisterObjectFuncPtr>(&QDBusConnection::registerObject),
                   [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(&RecentManager::startWatch, [&](RecentManager *) {
        __DBG_STUB_INVOKE__
        started = true;
    });

    // Mock stop operations
    stub.set_lamda(&RecentManager::finalize, [&](RecentManager *) {
        __DBG_STUB_INVOKE__
        stopped = true;
    });

    // Test lifecycle
    daemon->initialize();
    EXPECT_TRUE(initialized);

    bool startResult = daemon->start();
    EXPECT_TRUE(startResult);
    EXPECT_TRUE(started);

    daemon->stop();
    EXPECT_TRUE(stopped);
}
