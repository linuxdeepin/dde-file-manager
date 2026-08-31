// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_timegroupstrategy_1.cpp
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

TEST_F(TimeGroupStrategyTest, getDisplayNames)
{
    // Test getter: QHash<QString, QString> getDisplayNames()
    auto result = obj->getDisplayNames();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TimeGroupStrategyTest, getGroupDisplayName)
{
    // Test method: QString getGroupDisplayName((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getGroupDisplayName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TimeGroupStrategyTest, getGroupOrder)
{
    // Test getter: QStringList getGroupOrder()
    auto result = obj->getGroupOrder();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TimeGroupStrategyTest, getStrategyName)
{
    // Test getter: QString getStrategyName()
    auto result = obj->getStrategyName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TimeGroupStrategyTest, isGroupVisible)
{
    // Test method: bool isGroupVisible((const QString &groupKey, const QList<FileInfoPointer> &infos))
    QString _arg0{};
    QList<FileInfoPointer> _arg1{};
    auto result = obj->isGroupVisible(_arg0, _arg1);
    EXPECT_FALSE(result);

}
