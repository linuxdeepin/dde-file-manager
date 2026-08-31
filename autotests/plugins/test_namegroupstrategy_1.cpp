// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_namegroupstrategy_1.cpp
 * @brief Unit tests for NameGroupStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/namegroupstrategy.h"

#include <QTest>

using namespace dfmplugin_workspace;

class NameGroupStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NameGroupStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NameGroupStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NameGroupStrategyTest, getDisplayNames)
{
    // Test getter: QHash<QString, QString> getDisplayNames()
    auto result = obj->getDisplayNames();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NameGroupStrategyTest, getGroupDisplayName)
{
    // Test method: QString getGroupDisplayName((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getGroupDisplayName(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NameGroupStrategyTest, getGroupDisplayOrder)
{
    // Test method: int getGroupDisplayOrder((const QString &groupKey))
    QString _arg0{};
    auto result = obj->getGroupDisplayOrder(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(NameGroupStrategyTest, getGroupKey)
{
    // Test method: QString getGroupKey((const FileInfoPointer &info))
    FileInfoPointer _arg0{};
    auto result = obj->getGroupKey(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NameGroupStrategyTest, getGroupOrder)
{
    // Test getter: QStringList getGroupOrder()
    auto result = obj->getGroupOrder();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NameGroupStrategyTest, getPinyin)
{
    // Test method: QString getPinyin((const QChar &ch))
    QChar _arg0{};
    auto result = obj->getPinyin(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NameGroupStrategyTest, getStrategyName)
{
    // Test getter: QString getStrategyName()
    auto result = obj->getStrategyName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(NameGroupStrategyTest, isChinese)
{
    // Test method: bool isChinese((const QChar &ch))
    QChar _arg0{};
    auto result = obj->isChinese(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(NameGroupStrategyTest, isGroupVisible)
{
    // Test method: bool isGroupVisible((const QString &groupKey, const QList<FileInfoPointer> &infos))
    QString _arg0{};
    QList<FileInfoPointer> _arg1{};
    auto result = obj->isGroupVisible(_arg0, _arg1);
    EXPECT_FALSE(result);

}
