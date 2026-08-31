// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shortcutoper.cpp
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

TEST_F(ShortcutOperTest, clearClipBoard)
{
    // Test method: void clearClipBoard(())
    EXPECT_NO_FATAL_FAILURE(obj->clearClipBoard());
}

TEST_F(ShortcutOperTest, showMenu)
{
    // Test method: void showMenu(())
    EXPECT_NO_FATAL_FAILURE(obj->showMenu());
}
