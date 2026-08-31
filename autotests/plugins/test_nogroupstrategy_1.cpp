// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_nogroupstrategy_1.cpp
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

TEST_F(NoGroupStrategyTest, getGroupDisplayName)
{
    // Test method: QString getGroupDisplayName((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getGroupDisplayName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NoGroupStrategyTest, getGroupDisplayOrder)
{
    // Test method: int getGroupDisplayOrder((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getGroupDisplayOrder(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(NoGroupStrategyTest, getGroupOrder)
{
    // Test getter: QStringList getGroupOrder()
    auto result = obj->getGroupOrder();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NoGroupStrategyTest, getStrategyName)
{
    // Test getter: QString getStrategyName()
    auto result = obj->getStrategyName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NoGroupStrategyTest, isGroupVisible)
{
    // Test method: bool isGroupVisible((const QString &groupKey, const QList<FileInfoPointer> &infos))
    QString _arg0{};
    QList<FileInfoPointer> _arg1{};
    auto result = obj->isGroupVisible(_arg0, _arg1);
    EXPECT_FALSE(result);

}
