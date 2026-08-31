// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentmenucreator.cpp
 * @brief Unit tests for RecentMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/recentmenuscene.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentMenuCreatorTest, create)
{
    // Test getter: DFMBASE_USE_NAMESPACE create()
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });
}

TEST_F(RecentMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}
