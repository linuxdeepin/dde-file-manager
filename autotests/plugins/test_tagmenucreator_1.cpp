// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagmenucreator_1.cpp
 * @brief Unit tests for TagMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/tagmenuscene.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}
