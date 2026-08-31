// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_nogroupstrategy.cpp
 * @brief Unit tests for NoGroupStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/nogroupstrategy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class NoGroupStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NoGroupStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NoGroupStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NoGroupStrategyTest, NoGroupStrategy)
{
    // Test constructor: NoGroupStrategy((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(NoGroupStrategyTest, getGroupKey)
{
    // Test method: QString getGroupKey((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->getGroupKey(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
