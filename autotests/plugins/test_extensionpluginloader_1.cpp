// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionpluginloader_1.cpp
 * @brief Unit tests for ExtensionPluginLoader methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/pluginsload/extensionpluginloader.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionPluginLoaderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionPluginLoader();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionPluginLoader *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionPluginLoaderTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(ExtensionPluginLoaderTest, lastError)
{
    // Test getter: QString lastError()
    auto result = obj->lastError();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ExtensionPluginLoaderTest, loadPlugin)
{
    // Test bool getter: loadPlugin()
    bool result = obj->loadPlugin();
    EXPECT_FALSE(result);

}

TEST_F(ExtensionPluginLoaderTest, resolveEmblemPlugin)
{
    // Test getter: DFMEXT::DFMExtEmblemIconPlugin resolveEmblemPlugin()
    auto result = obj->resolveEmblemPlugin();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ExtensionPluginLoaderTest, resolveFilePlugin)
{
    // Test getter: DFMEXT::DFMExtFilePlugin resolveFilePlugin()
    auto result = obj->resolveFilePlugin();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ExtensionPluginLoaderTest, resolveMenuPlugin)
{
    // Test getter: DFMEXT::DFMExtMenuPlugin resolveMenuPlugin()
    auto result = obj->resolveMenuPlugin();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(ExtensionPluginLoaderTest, resolveWindowPlugin)
{
    // Test getter: DFMEXT::DFMExtWindowPlugin resolveWindowPlugin()
    auto result = obj->resolveWindowPlugin();
    EXPECT_GE(static_cast<int>(result), 0);

}
