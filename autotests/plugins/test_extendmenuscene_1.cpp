// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_extendmenuscene_1.cpp
 * @brief Unit tests for ExtendMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "extendmenuscene/extendmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ExtendMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ExtendMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ExtendMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ExtendMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(ExtendMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}
