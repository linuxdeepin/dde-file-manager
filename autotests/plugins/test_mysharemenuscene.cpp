// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mysharemenuscene.cpp
 * @brief Unit tests for MyShareMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/mysharemenuscene.h"

#include <QTest>

using namespace dfmplugin_myshares;

class MyShareMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MyShareMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MyShareMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MyShareMenuSceneTest, MyShareMenuScene)
{
    // Test constructor: MyShareMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MyShareMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(MyShareMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(MyShareMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}
