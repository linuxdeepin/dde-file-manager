// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagdirmenuscene.cpp
 * @brief Unit tests for TagDirMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/tagdirmenuscene.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagDirMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagDirMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagDirMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagDirMenuSceneTest, TagDirMenuScene)
{
    // Test constructor: TagDirMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagDirMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(TagDirMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
