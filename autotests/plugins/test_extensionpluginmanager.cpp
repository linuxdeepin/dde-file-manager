// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionpluginmanager.cpp
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

TEST_F(ExtensionPluginManagerTest, exists)
{
    // Test method: bool exists((ExtensionPluginManager::ExtensionType type))
    auto result = obj->exists(ExtensionPluginManager::ExtensionType());
    EXPECT_FALSE(result);

}

TEST_F(ExtensionPluginManagerTest, initialized)
{
    // Test bool getter: initialized()
    bool result = obj->initialized();
    EXPECT_FALSE(result);

}
