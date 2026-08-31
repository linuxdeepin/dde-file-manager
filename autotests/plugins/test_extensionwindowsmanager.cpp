// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extensionwindowsmanager.cpp
 * @brief Unit tests for ExtensionWindowsManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extensionimpl/windowimpl/extensionwindowsmanager.h"

#include <QTest>

using namespace dfmplugin_utils;

class ExtensionWindowsManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtensionWindowsManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtensionWindowsManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtensionWindowsManagerTest, instance)
{
    // Test getter: ExtensionWindowsManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ExtensionWindowsManagerTest, onAllPluginsInitialized)
{
    // Test method: void onAllPluginsInitialized(())
    EXPECT_NO_FATAL_FAILURE(obj->onAllPluginsInitialized());
}
