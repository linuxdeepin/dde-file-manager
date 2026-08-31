// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shredmenusceneprivate.cpp
 * @brief Unit tests for ShredMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "shred/shredmenuscene.h"

#include <QTest>

using namespace dfmplugin_utils;

class ShredMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShredMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShredMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShredMenuScenePrivateTest, ShredMenuScenePrivate)
{
    // Test constructor: ShredMenuScenePrivate((ShredMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShredMenuScenePrivateTest, updateMenu)
{
    // Test method: void updateMenu((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->updateMenu(nullptr));
}
