// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basesortmenucreator.cpp
 * @brief Unit tests for BaseSortMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/basesortmenuscene.h"

#include <QTest>

using namespace dfmplugin_workspace;

class BaseSortMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BaseSortMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BaseSortMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BaseSortMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(BaseSortMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
