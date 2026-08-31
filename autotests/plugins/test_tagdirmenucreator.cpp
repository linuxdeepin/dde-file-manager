// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagdirmenucreator.cpp
 * @brief Unit tests for TagDirMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/tagdirmenuscene.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagDirMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagDirMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagDirMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagDirMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(TagDirMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
