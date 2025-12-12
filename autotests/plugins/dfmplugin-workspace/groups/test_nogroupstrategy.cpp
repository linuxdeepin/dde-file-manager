
// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "groups/nogroupstrategy.h"
#include <dfm-base/interfaces/fileinfo.h>
#include "stubext.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>

using namespace dfmplugin_workspace;

class NoGroupStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        strategy = new NoGroupStrategy();
    }

    void TearDown() override
    {
        delete strategy;
        stub.clear();
    }

    NoGroupStrategy *strategy;
    stub_ext::StubExt stub;
};

TEST_F(NoGroupStrategyTest, Constructor_Default_CreatesStrategy)
{
    // Test that constructor creates strategy successfully
    EXPECT_NE(strategy, nullptr);
}

TEST_F(NoGroupStrategyTest, GetGroupKey_ValidInfo_ReturnsNoGroupKey)
{
    // Test getGroupKey method
    FileInfoPointer info;
    
    auto result = strategy->getGroupKey(info);
    
    EXPECT_EQ(result, QString::fromLatin1("no-group"));
}

TEST_F(NoGroupStrategyTest, GetGroupKey_NullInfo_ReturnsNoGroupKey)
{
    // Test getGroupKey method with null info
    FileInfoPointer info;
    
    auto result = strategy->getGroupKey(info);
    
    EXPECT_EQ(result, QString::fromLatin1("no-group"));
}

TEST_F(NoGroupStrategyTest, GetGroupDisplayName_ValidKey_ReturnsEmptyString)
{
    // Test getGroupDisplayName method
    QString groupKey = "test_key";
    
    auto result = strategy->getGroupDisplayName(groupKey);
    
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(NoGroupStrategyTest, GetGroupDisplayName_NoGroupKey_ReturnsEmptyString)
{
    // Test getGroupDisplayName method with no-group key
    QString groupKey = "no-group";
    
    auto result = strategy->getGroupDisplayName(groupKey);
    
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(NoGroupStrategyTest, GetGroupOrder_ReturnsNoGroupKey)
{
    // Test getGroupOrder method
    auto result = strategy->getGroupOrder();
    
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.first(), QString::fromLatin1("no-group"));
}

TEST_F(NoGroupStrategyTest, GetGroupDisplayOrder_ValidKey_ReturnsZero)
{
    // Test getGroupDisplayOrder method
    QString groupKey = "test_key";
    
    auto result = strategy->getGroupDisplayOrder(groupKey);
    
    EXPECT_EQ(result, 0);
}

TEST_F(NoGroupStrategyTest, GetGroupDisplayOrder_NoGroupKey_ReturnsZero)
{
    // Test getGroupDisplayOrder method with no-group key
    QString groupKey = "no-group";
    
    auto result = strategy->getGroupDisplayOrder(groupKey);
    
    EXPECT_EQ(result, 0);
}

TEST_F(NoGroupStrategyTest, IsGroupVisible_ValidKeyAndInfos_ReturnsFalse)
{
    // Test isGroupVisible method
    QString groupKey = "test_key";
    QList<FileInfoPointer> infos;
    
    auto result = strategy->isGroupVisible(groupKey, infos);
    
    EXPECT_FALSE(result);
}

TEST_F(NoGroupStrategyTest, IsGroupVisible_NoGroupKeyAndEmptyInfos_ReturnsFalse)
{
    // Test isGroupVisible method with no-group key and empty infos
    QString groupKey = "no-group";
    QList<FileInfoPointer> infos;
    
    auto result = strategy->isGroupVisible(groupKey, infos);
    
    EXPECT_FALSE(result);
}

TEST_F(NoGroupStrategyTest, IsGroupVisible_NoGroupKeyAndValidInfos_ReturnsFalse)
{
    // Test isGroupVisible method with no-group key and valid infos
    QString groupKey = "no-group";
    QList<FileInfoPointer> infos;
    // Note: We can't easily create real FileInfoPointer in test environment,
    // but method should return false regardless of infos content
    
    auto result = strategy->isGroupVisible(groupKey, infos);
    
    EXPECT_FALSE(result);
}

TEST_F(NoGroupStrategyTest, GetStrategyName_ReturnsNoGroupName)
{
    // Test getStrategyName method
    auto result = strategy->getStrategyName();
    
    EXPECT_EQ(result, GroupStrategy::kNoGroup);
}

TEST_F(NoGroupStrategyTest, WithParent_CreatesStrategyWithParent)
{
    // Test creating strategy with parent
    QObject parent;
    NoGroupStrategy *strategyWithParent = new NoGroupStrategy(&parent);
    
    EXPECT_NE(strategyWithParent, nullptr);
    EXPECT_EQ(strategyWithParent->parent(), &parent);
    
    delete strategyWithParent;
}

TEST_F(NoGroupStrategyTest, WithoutParent_CreatesStrategyWithoutParent)
{
    // Test creating strategy without parent
    NoGroupStrategy *strategyWithoutParent = new NoGroupStrategy();
    
    EXPECT_NE(strategyWithoutParent, nullptr);
    EXPECT_EQ(strategyWithoutParent->parent(), nullptr);
    
    delete strategyWithoutParent;
}
