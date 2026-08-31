// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pathgroupstrategy.cpp
 * @brief Unit tests for PathGroupStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/pathgroupstrategy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class PathGroupStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PathGroupStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PathGroupStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PathGroupStrategyTest, PathGroupStrategy)
{
    // Test constructor: PathGroupStrategy((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(PathGroupStrategyTest, getGroupOrder)
{
    // Test getter: QStringList getGroupOrder()
    auto result = obj->getGroupOrder();
    EXPECT_TRUE(result.isEmpty());

}
