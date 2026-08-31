// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_timegroupstrategy.cpp
 * @brief Unit tests for TimeGroupStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/timegroupstrategy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class TimeGroupStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TimeGroupStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TimeGroupStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TimeGroupStrategyTest, TimeGroupStrategy)
{
    // Test constructor: TimeGroupStrategy((TimeType timeType, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TimeGroupStrategyTest, calculateTimeGroup)
{
    // Test method: QString calculateTimeGroup((const QDateTime &fileTime))
    QDateTime _arg0{};
    auto result = obj->calculateTimeGroup(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TimeGroupStrategyTest, getGroupDisplayOrder)
{
    // Test method: int getGroupDisplayOrder((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getGroupDisplayOrder(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(TimeGroupStrategyTest, getGroupKey)
{
    // Test method: QString getGroupKey((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->getGroupKey(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
