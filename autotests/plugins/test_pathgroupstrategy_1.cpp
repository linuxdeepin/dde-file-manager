// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pathgroupstrategy_1.cpp
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

TEST_F(PathGroupStrategyTest, classifyByPath)
{
    // Test method: QString classifyByPath((const QString &path))
    QString _arg0{};
    auto result = obj->classifyByPath(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PathGroupStrategyTest, getDisplayNames)
{
    // Test getter: QHash<QString, QString> getDisplayNames()
    auto result = obj->getDisplayNames();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PathGroupStrategyTest, getGroupDisplayName)
{
    // Test method: QString getGroupDisplayName((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getGroupDisplayName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PathGroupStrategyTest, getGroupDisplayOrder)
{
    // Test method: int getGroupDisplayOrder((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getGroupDisplayOrder(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(PathGroupStrategyTest, getGroupKey)
{
    // Test method: QString getGroupKey((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->getGroupKey(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PathGroupStrategyTest, getPathOrder)
{
    // Test getter: QStringList getPathOrder()
    auto result = obj->getPathOrder();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PathGroupStrategyTest, getStrategyName)
{
    // Test getter: QString getStrategyName()
    auto result = obj->getStrategyName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(PathGroupStrategyTest, isGroupVisible)
{
    // Test method: bool isGroupVisible((const QString &groupKey, const QList<FileInfoPointer> &infos))
    QString _arg0{};
    QList<FileInfoPointer> _arg1{};
    auto result = obj->isGroupVisible(_arg0, _arg1);
    EXPECT_FALSE(result);

}
