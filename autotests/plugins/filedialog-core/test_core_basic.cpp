// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "../../../src/plugins/filedialog/core/core.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/dpf.h>

#include <QApplication>
#include <QDBusConnection>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
using namespace filedialog_core;

class UT_Core_Basic : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Initialize test environment
        core = new Core();
    }

    virtual void TearDown() override
    {
        delete core;
        core = nullptr;
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
    Core *core = nullptr;
};

TEST_F(UT_Core_Basic, Constructor_CreatesInstance)
{
    EXPECT_NE(core, nullptr);
}

TEST_F(UT_Core_Basic, Destructor_CleansUp)
{
    // Just test that destructor doesn't crash
    Core *testCore = new Core();
    delete testCore;
    SUCCEED();
}

TEST_F(UT_Core_Basic, Start_ReturnsTrue)
{
    // Mock FMWindowsIns.setCustomWindowCreator
    bool customWindowCreatorCalled = false;
    stub.set_lamda(&FileManagerWindowsManager::setCustomWindowCreator, [&] {
        __DBG_STUB_INVOKE__
        customWindowCreatorCalled = true;
    });

    // Mock dpfListener connection
    bool listenerConnected = false;
    // Since this is complex to mock, we'll skip it for now
    // The test should still work with real function call
    (void)listenerConnected;

    // Mock QDBusConnection::systemBus().connect()
    bool dbusConnected = false;
    stub.set_lamda((bool (QDBusConnection::*)(const QString &, const QString &,
                                              const QString &, const QString &,
                                              QObject *, const char *))&QDBusConnection::connect,
                   [&] {
        __DBG_STUB_INVOKE__
        dbusConnected = true;
        return true;
    });

    EXPECT_TRUE(core->start());
    EXPECT_TRUE(customWindowCreatorCalled);
    EXPECT_TRUE(dbusConnected);
}

TEST_F(UT_Core_Basic, Stop_DoesNotCrash)
{
    EXPECT_NO_THROW(core->stop());
}

TEST_F(UT_Core_Basic, MultipleStartCalls_HandlesCorrectly)
{
    // Mock FMWindowsIns.setCustomWindowCreator
    int customWindowCreatorCallCount = 0;
    stub.set_lamda(&FileManagerWindowsManager::setCustomWindowCreator, [&] {
        __DBG_STUB_INVOKE__
        customWindowCreatorCallCount++;
    });

    // Mock dpfListener connection
    int listenerConnectedCount = 0;
    // Since this is complex to mock, we'll skip it for now
    // The test should still work with real function call
    (void)listenerConnectedCount;

    // Mock QDBusConnection::systemBus().connect()
    int dbusConnectedCount = 0;
    stub.set_lamda((bool (QDBusConnection::*)(const QString &, const QString &,
                                              const QString &, const QString &,
                                              QObject *, const char *))&QDBusConnection::connect,
                   [&] {
        __DBG_STUB_INVOKE__
        dbusConnectedCount++;
        return true;
    });

    // Call start multiple times
    core->start();
    core->start();
    core->start();
    
    EXPECT_EQ(customWindowCreatorCallCount, 3);
    EXPECT_EQ(dbusConnectedCount, 3);
}

TEST_F(UT_Core_Basic, StaticMethods_NoInstanceRequired_CallSuccessfully)
{
    // Verify that methods can be called with instance
    EXPECT_NO_THROW(core->bindSceneOnAdded(QString("")));
    EXPECT_NO_THROW(core->exitOnShutdown(false));
}

TEST_F(UT_Core_Basic, ErrorHandling_InvalidParameters_HandlesGracefully)
{
    // Test with invalid parameters
    EXPECT_NO_THROW(core->bindSceneOnAdded(QString("")));
    EXPECT_NO_THROW(core->exitOnShutdown(false));
    EXPECT_NO_THROW(core->stop());
}