// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_typegroupstrategy_1.cpp
 * @brief Unit tests for TypeGroupStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/typegroupstrategy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class TypeGroupStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TypeGroupStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TypeGroupStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TypeGroupStrategyTest, getDisplayNames)
{
    // Test getter: QHash<QString, QString> getDisplayNames()
    auto result = obj->getDisplayNames();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TypeGroupStrategyTest, getGroupDisplayName)
{
    // Test method: QString getGroupDisplayName((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getGroupDisplayName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TypeGroupStrategyTest, getGroupDisplayOrder)
{
    // Test method: int getGroupDisplayOrder((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getGroupDisplayOrder(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(TypeGroupStrategyTest, getGroupKey)
{
    // Test method: QString getGroupKey((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->getGroupKey(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TypeGroupStrategyTest, getGroupOrder)
{
    // Test getter: QStringList getGroupOrder()
    auto result = obj->getGroupOrder();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TypeGroupStrategyTest, getStrategyName)
{
    // Test getter: QString getStrategyName()
    auto result = obj->getStrategyName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TypeGroupStrategyTest, isGroupVisible)
{
    // Test method: bool isGroupVisible((const QString &groupKey, const QList<FileInfoPointer> &infos))
    QString _arg0{};
    QList<FileInfoPointer> _arg1{};
    auto result = obj->isGroupVisible(_arg0, _arg1);
    EXPECT_FALSE(result);

}
