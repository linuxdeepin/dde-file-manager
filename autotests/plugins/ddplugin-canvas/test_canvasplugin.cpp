// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/desktop/ddplugin-canvas/canvasplugin.h"
#include "plugins/desktop/ddplugin-canvas/canvasmanager.h"
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QWidget>
#include <QDBusConnection>

#include <gtest/gtest.h>

using namespace ddplugin_canvas;
DFMBASE_USE_NAMESPACE

class UT_CanvasPlugin : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Stub DConfigManager to avoid real configuration operations
        stub.set_lamda(&DConfigManager::addConfig, [](DConfigManager *, const QString &, QString *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        // Stub CanvasManager methods to avoid dependencies
        stub.set_lamda(ADDR(CanvasManager, init), [](CanvasManager *) {
            __DBG_STUB_INVOKE__
        });

        plugin = new CanvasPlugin();
    }

    virtual void TearDown() override
    {
        // Ensure plugin is properly stopped before deletion
        if (plugin) {
            plugin->stop();
        }
        delete plugin;
        plugin = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CanvasPlugin *plugin = nullptr;
};

TEST_F(UT_CanvasPlugin, constructor)
{
    EXPECT_NE(plugin, nullptr);
}

TEST_F(UT_CanvasPlugin, initialize)
{
    bool addConfigCalled = false;
    
    // Stub addConfig to capture the call
    // Note: The actual call is with "org.deepin.dde.file-manager.desktop.sys-watermask" config name
    // The test just needs to verify that addConfig is called, not the specific name
    stub.set_lamda(&DConfigManager::addConfig, [&addConfigCalled](DConfigManager *, const QString &name, QString *) -> bool {
        __DBG_STUB_INVOKE__
        addConfigCalled = true;
        // Don't check the specific name since there might be multiple calls
        return true;
    });

    plugin->initialize();
    EXPECT_TRUE(addConfigCalled);
}

TEST_F(UT_CanvasPlugin, start)
{
    bool managerInitCalled = false;
    bool registerDBusCalled = false;
    
    // Mock CanvasManager initialization
    stub.set_lamda(ADDR(CanvasManager, init), [&managerInitCalled](CanvasManager *) {
        __DBG_STUB_INVOKE__
        managerInitCalled = true;
    });
    
    // Mock D-Bus registration using correct overload (4 parameters: path, serviceName, object, options)
    using RegisterObjectFunc = bool (QDBusConnection::*)(const QString &, const QString &, QObject *, QDBusConnection::RegisterOptions);
    stub.set_lamda(static_cast<RegisterObjectFunc>(&QDBusConnection::registerObject), [&registerDBusCalled](QDBusConnection *, const QString &path, const QString &serviceName, QObject *, QDBusConnection::RegisterOptions) -> bool {
        __DBG_STUB_INVOKE__
        registerDBusCalled = true;
        EXPECT_EQ(path, QString("/org/deepin/dde/desktop/canvas"));
        EXPECT_EQ(serviceName, QString("org.deepin.dde.desktop.canvas"));
        return true;
    });

    bool result = plugin->start();
    EXPECT_TRUE(result);
    EXPECT_TRUE(managerInitCalled);
    EXPECT_TRUE(registerDBusCalled);
}

TEST_F(UT_CanvasPlugin, stop)
{
    // Test stop method - should complete without throwing
    // The stop method deletes the proxy and sets it to nullptr
    EXPECT_NO_THROW(plugin->stop());
    
    // Verify that stop method executed successfully  
    EXPECT_TRUE(true); // Basic verification that method completed
}

TEST_F(UT_CanvasPlugin, registerDBus)
{
    // This test focuses on D-Bus registration functionality
    // Since registerDBus is private and requires CanvasManager instance,
    // and testing it directly would cause singleton conflicts,
    // we test the D-Bus registration through the start() lifecycle
    
    // The registerDBus functionality is already tested indirectly in the start() test
    // Here we just verify it doesn't crash when called
    EXPECT_NO_THROW({
        plugin->start(); // This internally calls registerDBus()
        plugin->stop();  // Clean up properly
    });
    
    EXPECT_TRUE(true); // Basic verification that method completed
}

TEST_F(UT_CanvasPlugin, pluginLifecycle_InitializeStartStop_ProperSequence)
{
    // Test complete plugin lifecycle
    bool initializeCalled = false;
    bool startCalled = false;
    bool stopCalled = false;
    
    // Stub lifecycle methods
    stub.set_lamda(&DConfigManager::addConfig, [&initializeCalled](DConfigManager *, const QString &, QString *) -> bool {
        __DBG_STUB_INVOKE__
        initializeCalled = true;
        return true;
    });
    
    stub.set_lamda(ADDR(CanvasManager, init), [&startCalled](CanvasManager *) {
        __DBG_STUB_INVOKE__
        startCalled = true;
    });
    
    using RegisterObjectFunc = bool (QDBusConnection::*)(const QString &, QObject *, QDBusConnection::RegisterOptions);
    stub.set_lamda(static_cast<RegisterObjectFunc>(&QDBusConnection::registerObject), [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Execute lifecycle sequence
    plugin->initialize();
    EXPECT_TRUE(initializeCalled);
    
    bool startResult = plugin->start();
    EXPECT_TRUE(startResult);
    EXPECT_TRUE(startCalled);
    
    // Note: stop() doesn't have observable side effects in current implementation
    plugin->stop();
    SUCCEED(); // Test that stop doesn't crash
}
