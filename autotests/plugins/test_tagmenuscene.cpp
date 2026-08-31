// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagmenuscene.cpp
 * @brief Unit tests for TagMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/tagmenuscene.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagMenuSceneTest, TagMenuScene)
{
    // Test constructor: TagMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TagMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}
