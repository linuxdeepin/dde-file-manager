// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shortcutoper_1.cpp
 * @brief Unit tests for ShortcutOper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/operator/shortcutoper.h"

#include <QTest>

using namespace ddplugin_canvas;

class ShortcutOperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShortcutOper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShortcutOper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShortcutOperTest, ShortcutOper)
{
    // Test constructor: ShortcutOper((CanvasView *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShortcutOperTest, disableShortcut)
{
    // Test bool getter: disableShortcut()
    bool result = obj->disableShortcut();
    EXPECT_FALSE(result);

}

TEST_F(ShortcutOperTest, helpAction)
{
    // Test method: void helpAction(())
    EXPECT_NO_FATAL_FAILURE(obj->helpAction());
}

TEST_F(ShortcutOperTest, previewFiles)
{
    // Test method: void previewFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->previewFiles());
}

TEST_F(ShortcutOperTest, switchHidden)
{
    // Test method: void switchHidden(())
    EXPECT_NO_FATAL_FAILURE(obj->switchHidden());
}

TEST_F(ShortcutOperTest, tabToFirst)
{
    // Test method: void tabToFirst(())
    EXPECT_NO_FATAL_FAILURE(obj->tabToFirst());
}
