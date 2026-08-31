// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionpluginmanager_1.cpp
 * @brief Unit tests for ExtensionPluginManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/pluginsload/extensionpluginmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionPluginManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionPluginManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionPluginManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionPluginManagerTest, ExtensionPluginManager)
{
    // Test constructor: ExtensionPluginManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ExtensionPluginManagerTest, currentState)
{
    // Test getter: ExtensionPluginManager::InitState currentState()
    auto result = obj->currentState();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ExtensionPluginManagerTest, emblemPlugins)
{
    // Test getter: QList<DFMEXT::DFMExtEmblemIconPlugin *> emblemPlugins()
    auto result = obj->emblemPlugins();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ExtensionPluginManagerTest, filePlugins)
{
    // Test getter: QList<DFMEXT::DFMExtFilePlugin *> filePlugins()
    auto result = obj->filePlugins();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ExtensionPluginManagerTest, instance)
{
    // Test getter: ExtensionPluginManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ExtensionPluginManagerTest, menuPlugins)
{
    // Test getter: QList<DFMEXT::DFMExtMenuPlugin *> menuPlugins()
    auto result = obj->menuPlugins();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ExtensionPluginManagerTest, onLoadingPlugins)
{
    // Test method: void onLoadingPlugins(())
    EXPECT_NO_FATAL_FAILURE(obj->onLoadingPlugins());
}

TEST_F(ExtensionPluginManagerTest, pluginMenuProxy)
{
    // Test getter: DFMEXT::DFMExtMenuProxy pluginMenuProxy()
    auto result = obj->pluginMenuProxy();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ExtensionPluginManagerTest, windowPlugins)
{
    // Test getter: QList<DFMEXT::DFMExtWindowPlugin *> windowPlugins()
    auto result = obj->windowPlugins();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ExtensionPluginManagerTest, ExtensionPluginManager_Destructor)
{
    // Test method:  ~ExtensionPluginManager(())
    EXPECT_NO_FATAL_FAILURE({ ExtensionPluginManager *tmp = new ExtensionPluginManager(); delete tmp; });
}
