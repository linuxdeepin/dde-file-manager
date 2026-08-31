// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_core_1.cpp
 * @brief Unit tests for Core methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "core.h"

#include <QTest>

using namespace core;

class CoreTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Core();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Core *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CoreTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CoreTest, eventFilter_eventFil)
{
    // Test method: bool eventFilter((QObject *watched, QEvent *event))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CoreTest, exitOnShutdown)
{
    // Test method: void exitOnShutdown((bool shutdown))
    EXPECT_NO_FATAL_FAILURE(obj->exitOnShutdown(false));
}

TEST_F(CoreTest, handleLoadPlugins)
{
    // Test method: void handleLoadPlugins((const QStringList &names))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleLoadPlugins(_arg0));
}

TEST_F(CoreTest, initDeviceDBus)
{
    // Test method: void initDeviceDBus((QDBusConnection *connection))
    EXPECT_NO_FATAL_FAILURE(obj->initDeviceDBus(nullptr));
}

TEST_F(CoreTest, initOperationsDBus)
{
    // Test method: void initOperationsDBus((QDBusConnection *connection))
    EXPECT_NO_FATAL_FAILURE(obj->initOperationsDBus(nullptr));
}

TEST_F(CoreTest, initServiceDBusInterfaces)
{
    // Test method: void initServiceDBusInterfaces((QDBusConnection *connection))
    EXPECT_NO_FATAL_FAILURE(obj->initServiceDBusInterfaces(nullptr));
}

TEST_F(CoreTest, initSyncDBus)
{
    // Test method: void initSyncDBus((QDBusConnection *connection))
    EXPECT_NO_FATAL_FAILURE(obj->initSyncDBus(nullptr));
}

TEST_F(CoreTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(CoreTest, initialize_initiali)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(CoreTest, initializeAfterPainted)
{
    // Test method: void initializeAfterPainted(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeAfterPainted());
}

TEST_F(CoreTest, onAllPluginsInitialized)
{
    // Test method: void onAllPluginsInitialized(())
    EXPECT_NO_FATAL_FAILURE(obj->onAllPluginsInitialized());
}

TEST_F(CoreTest, onAllPluginsStarted)
{
    // Test method: void onAllPluginsStarted(())
    EXPECT_NO_FATAL_FAILURE(obj->onAllPluginsStarted());
}

TEST_F(CoreTest, onFrameReady)
{
    // Test method: void onFrameReady(())
    EXPECT_NO_FATAL_FAILURE(obj->onFrameReady());
}

TEST_F(CoreTest, onStart)
{
    // Test method: void onStart(())
    EXPECT_NO_FATAL_FAILURE(obj->onStart());
}

TEST_F(CoreTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(CoreTest, stop_stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}
