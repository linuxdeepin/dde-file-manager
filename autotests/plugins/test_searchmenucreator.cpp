// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchmenucreator.cpp
 * @brief Unit tests for SearchMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/searchmenuscene.h"

#include <QTest>

using namespace dfmplugin_search;

class SearchMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(SearchMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
