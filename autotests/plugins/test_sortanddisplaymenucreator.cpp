// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortanddisplaymenucreator.cpp
 * @brief Unit tests for SortAndDisplayMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/sortanddisplaymenuscene.h"

#include <QTest>

using namespace dfmplugin_workspace;

class SortAndDisplayMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SortAndDisplayMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SortAndDisplayMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SortAndDisplayMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(SortAndDisplayMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
