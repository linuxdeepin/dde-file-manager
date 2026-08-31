// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionpluginloader.cpp
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

TEST_F(ExtensionPluginLoaderTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ExtensionPluginLoaderTest, shutdown)
{
    // Test bool getter: shutdown()
    bool result = obj->shutdown();
    EXPECT_FALSE(result);

}
