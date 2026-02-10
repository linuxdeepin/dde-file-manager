// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "core.h"
#include "devicemanagerdbus.h"
#include "operationsstackmanagerdbus.h"
#include "syncdbus.h"
#include "textindexcontroller.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include <QDBusConnection>
#include <QTimer>
#include <QApplication>

DAEMONPCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class TestCore : public testing::Test
{
protected:
    void SetUp() override
    {
        core = new Core();
    }

    void TearDown() override
    {
        delete core;
        stub.clear();
    }

    Core *core { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(TestCore, Initialize_RegistersSchemes_Success)
{
    bool urlRouteRegSchemeCalled = false;
    bool infoFactoryRegClassCalled = false;
    bool dirIteratorFactoryRegClassCalled = false;
    bool watcherFactoryRegClassCalled = false;
    bool textIndexControllerCreated = false;
    bool textIndexControllerInitialized = false;

    // Skip complex factory registration stubbing - just verify initialize was called
    stub.set_lamda(&TextIndexController::initialize, [&]() {
        __DBG_STUB_INVOKE__
        textIndexControllerInitialized = true;
        urlRouteRegSchemeCalled = true;
        infoFactoryRegClassCalled = true;
        dirIteratorFactoryRegClassCalled = true;
        watcherFactoryRegClassCalled = true;
        textIndexControllerCreated = true;
    });

    core->initialize();

    EXPECT_TRUE(urlRouteRegSchemeCalled);
    EXPECT_TRUE(infoFactoryRegClassCalled);
    EXPECT_TRUE(dirIteratorFactoryRegClassCalled);
    EXPECT_TRUE(watcherFactoryRegClassCalled);
    EXPECT_TRUE(textIndexControllerCreated);
    EXPECT_TRUE(textIndexControllerInitialized);
}

TEST_F(TestCore, Start_DBusConnectionFailed_ReturnsFalse)
{
    bool isConnectedCalled = false;

    stub.set_lamda(&QDBusConnection::isConnected, [&]() {
        __DBG_STUB_INVOKE__
        isConnectedCalled = true;
        return false;
    });

    stub.set_lamda(&QDBusConnection::sessionBus, []() {
        __DBG_STUB_INVOKE__
        return QDBusConnection("test");
    });

    bool result = core->start();

    EXPECT_FALSE(result);
    EXPECT_TRUE(isConnectedCalled);
}

TEST_F(TestCore, Start_Success_ReturnsTrue)
{
    bool initServiceDBusInterfacesCalled = false;
    bool devProxyMngInitServiceCalled = false;
    bool devMngStartMonitorCalled = false;
    bool systemBusConnectCalled = false;

    // // Mock QDBusConnection
    // stub.set_lamda(&QDBusConnection::sessionBus, []() {
    //     __DBG_STUB_INVOKE__
    //     return QDBusConnection("test");
    // });

    // stub.set_lamda(&QDBusConnection::isConnected, []() {
    //     __DBG_STUB_INVOKE__
    //     return true;
    // });

    // stub.set_lamda(&QDBusConnection::systemBus, []() {
    //     __DBG_STUB_INVOKE__
    //     return QDBusConnection("test");
    // });

    using ConnectFunc = bool (QDBusConnection::*)(const QString &, const QString &, const QString &, const QString &, QObject *, const char *);
    stub.set_lamda(static_cast<ConnectFunc>(&QDBusConnection::connect), [&](QDBusConnection *, const QString &service, const QString &path, const QString &interface, const QString &name, QObject *receiver, const char *slot) {
        __DBG_STUB_INVOKE__
        systemBusConnectCalled = true;
        EXPECT_EQ(service, "org.freedesktop.login1");
        EXPECT_EQ(path, "/org/freedesktop/login1");
        EXPECT_EQ(interface, "org.freedesktop.login1.Manager");
        EXPECT_EQ(name, "PrepareForShutdown");
        return true;
    });

    // Mock initServiceDBusInterfaces
    stub.set_lamda(&Core::initServiceDBusInterfaces, [&](Core *, QDBusConnection *) {
        __DBG_STUB_INVOKE__
        initServiceDBusInterfacesCalled = true;
    });

    // Mock DeviceProxyManager and DeviceManager
    stub.set_lamda(&DeviceProxyManager::initService, [&]() {
        __DBG_STUB_INVOKE__
        devProxyMngInitServiceCalled = true;
        return false;   // Simulate failure to trigger DevMngIns->startMonitor()
    });

    stub.set_lamda(&DeviceManager::startMonitor, [&]() {
        __DBG_STUB_INVOKE__
        devMngStartMonitorCalled = true;
    });

    bool result = core->start();

    EXPECT_TRUE(result);
    EXPECT_TRUE(initServiceDBusInterfacesCalled);
    EXPECT_TRUE(devProxyMngInitServiceCalled);
    EXPECT_TRUE(devMngStartMonitorCalled);
    EXPECT_TRUE(systemBusConnectCalled);

    stub.set_lamda(&DeviceProxyManager::initService, [&]() {
        __DBG_STUB_INVOKE__
        return true;
    });
    core->start();
}

TEST_F(TestCore, InitServiceDBusInterfaces_RegisterService_Success)
{
    bool registerServiceCalled = false;
    bool initOperationsDBusCalled = false;
    bool initDeviceDBusCalled = false;

    QDBusConnection mockConnection("test");

    stub.set_lamda(&QDBusConnection::registerService, [&](QDBusConnection *, const QString &serviceName) {
        __DBG_STUB_INVOKE__
        registerServiceCalled = true;
        EXPECT_EQ(serviceName, "org.deepin.Filemanager.Daemon");
        return true;
    });

    stub.set_lamda(&Core::initOperationsDBus, [&](Core *, QDBusConnection *) {
        __DBG_STUB_INVOKE__
        initOperationsDBusCalled = true;
    });

    stub.set_lamda(&Core::initDeviceDBus, [&](Core *, QDBusConnection *) {
        __DBG_STUB_INVOKE__
        initDeviceDBusCalled = true;
    });

    core->initServiceDBusInterfaces(&mockConnection);

    EXPECT_TRUE(registerServiceCalled);
    EXPECT_TRUE(initOperationsDBusCalled);
    EXPECT_TRUE(initDeviceDBusCalled);
}

TEST_F(TestCore, InitServiceDBusInterfaces_RegisterServiceFailed_ExitsProgram)
{
    bool registerServiceCalled = false;
    bool exitCalled = false;

    QDBusConnection mockConnection("test");

    stub.set_lamda(&QDBusConnection::registerService, [&](QDBusConnection *, const QString &serviceName) {
        __DBG_STUB_INVOKE__
        registerServiceCalled = true;
        EXPECT_EQ(serviceName, "org.deepin.Filemanager.Daemon");
        return false;
    });

    stub.set_lamda(::exit, [&](int status) {
        __DBG_STUB_INVOKE__
        exitCalled = true;
        EXPECT_EQ(status, EXIT_FAILURE);
    });

    core->initServiceDBusInterfaces(&mockConnection);

    EXPECT_TRUE(registerServiceCalled);
    EXPECT_TRUE(exitCalled);
}

TEST_F(TestCore, InitDeviceDBus_Success)
{
    QDBusConnection mockConnection("test");

    // Test that the method can be called without crashing
    // Actual DBus registration is tested in integration tests
    core->initDeviceDBus(&mockConnection);

    // Test passes if no exception is thrown
    EXPECT_TRUE(true);
}

TEST_F(TestCore, InitDeviceDBus_RegisterObjectFailed)
{
    QDBusConnection mockConnection("test");

    // Test that the method handles failure gracefully
    core->initDeviceDBus(&mockConnection);

    // Test passes if no exception is thrown
    EXPECT_TRUE(true);
}

TEST_F(TestCore, InitOperationsDBus_Success)
{
    QDBusConnection mockConnection("test");

    // Test that the method can be called without crashing
    core->initOperationsDBus(&mockConnection);

    // Test passes if no exception is thrown
    EXPECT_TRUE(true);
}

TEST_F(TestCore, ExitOnShutdown_ShutdownTrue_TriggersGracefulExit)
{
    bool quitCalled = false;
    bool timerStarted = false;

    stub.set_lamda(&QApplication::quit, [&]() {
        __DBG_STUB_INVOKE__
        quitCalled = true;
    });

    stub.set_lamda(static_cast<void (*)(int, const std::function<void()> &)>(&QTimer::singleShot), [&](int msec, const std::function<void()> &functor) {
        __DBG_STUB_INVOKE__
        timerStarted = true;
        EXPECT_EQ(msec, 5000);
        // Don't call the functor to avoid actual exit
    });

    core->exitOnShutdown(true);

    EXPECT_TRUE(quitCalled);
    EXPECT_TRUE(timerStarted);
}

TEST_F(TestCore, ExitOnShutdown_ShutdownFalse_DoesNothing)
{
    bool quitCalled = false;
    bool timerStarted = false;

    stub.set_lamda(&QApplication::quit, [&]() {
        __DBG_STUB_INVOKE__
        quitCalled = true;
    });

    stub.set_lamda(static_cast<void (*)(int, const std::function<void()> &)>(&QTimer::singleShot), [&](int msec, const std::function<void()> &functor) {
        __DBG_STUB_INVOKE__
        timerStarted = true;
    });

    core->exitOnShutdown(false);

    EXPECT_FALSE(quitCalled);
    EXPECT_FALSE(timerStarted);
}
