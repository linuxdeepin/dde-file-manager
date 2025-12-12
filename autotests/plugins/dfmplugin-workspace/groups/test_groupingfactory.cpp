// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "dfm-base/dfm_log_defines.h"
#include "groups/groupingfactory.h"
#include "groups/nogroupstrategy.h"
#include "groups/namegroupstrategy.h"
#include "groups/sizegroupstrategy.h"
#include "groups/timegroupstrategy.h"
#include "groups/typegroupstrategy.h"
#include "groups/pathgroupstrategy.h"
#include "stubext.h"

#include <QObject>
#include <QString>

using namespace dfmplugin_workspace;

class GroupingFactoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        parent = new QObject();
    }

    void TearDown() override
    {
        delete parent;
        stub.clear();
    }

    QObject *parent;
    stub_ext::StubExt stub;
};

TEST_F(GroupingFactoryTest, CreateStrategy_NoGroup_CreatesNoGroupStrategy)
{
    // Test creating NoGroup strategy
    auto strategy = GroupingFactory::createStrategy(GroupStrategy::kNoGroup, parent);
    
    EXPECT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->getStrategyName(), GroupStrategy::kNoGroup);
    
    delete strategy;
}

TEST_F(GroupingFactoryTest, CreateStrategy_Name_CreatesNameGroupStrategy)
{
    // Test creating NameGroup strategy
    auto strategy = GroupingFactory::createStrategy(GroupStrategy::kName, parent);
    
    EXPECT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->getStrategyName(), GroupStrategy::kName);
    
    delete strategy;
}

TEST_F(GroupingFactoryTest, CreateStrategy_Size_CreatesSizeGroupStrategy)
{
    // Test creating SizeGroup strategy
    auto strategy = GroupingFactory::createStrategy(GroupStrategy::kSize, parent);
    
    EXPECT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->getStrategyName(), GroupStrategy::kSize);
    
    delete strategy;
}

TEST_F(GroupingFactoryTest, CreateStrategy_ModifiedTime_CreatesTimeGroupStrategy)
{
    // Test creating ModifiedTime strategy
    auto strategy = GroupingFactory::createStrategy(GroupStrategy::kModifiedTime, parent);
    
    EXPECT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->getStrategyName(), GroupStrategy::kModifiedTime);
    
    delete strategy;
}

TEST_F(GroupingFactoryTest, CreateStrategy_CreatedTime_CreatesTimeGroupStrategy)
{
    // Test creating CreatedTime strategy
    auto strategy = GroupingFactory::createStrategy(GroupStrategy::kCreatedTime, parent);
    
    EXPECT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->getStrategyName(), GroupStrategy::kCreatedTime);
    
    delete strategy;
}

TEST_F(GroupingFactoryTest, CreateStrategy_Type_CreatesTypeGroupStrategy)
{
    // Test creating TypeGroup strategy
    auto strategy = GroupingFactory::createStrategy(GroupStrategy::kType, parent);
    
    EXPECT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->getStrategyName(), GroupStrategy::kType);
    
    delete strategy;
}

TEST_F(GroupingFactoryTest, CreateStrategy_CustomPath_CreatesPathGroupStrategy)
{
    // Test creating CustomPath strategy
    auto strategy = GroupingFactory::createStrategy(GroupStrategy::kCustomPath, parent);
    
    EXPECT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->getStrategyName(), GroupStrategy::kCustomPath);
    
    delete strategy;
}

TEST_F(GroupingFactoryTest, CreateStrategy_CustomTime_CreatesTimeGroupStrategy)
{
    // Test creating CustomTime strategy
    auto strategy = GroupingFactory::createStrategy(GroupStrategy::kCustomTime, parent);
    
    EXPECT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->getStrategyName(), GroupStrategy::kCustomTime);
    
    delete strategy;
}

TEST_F(GroupingFactoryTest, CreateStrategy_UnknownStrategy_ReturnsNullptr)
{
    // Test creating unknown strategy
    auto strategy = GroupingFactory::createStrategy("unknown_strategy", parent);
    
    EXPECT_EQ(strategy, nullptr);
}

TEST_F(GroupingFactoryTest, CreateStrategy_NoParent_CreatesStrategyWithoutParent)
{
    // Test creating strategy without parent
    auto strategy = GroupingFactory::createStrategy(GroupStrategy::kNoGroup, nullptr);
    
    EXPECT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->parent(), nullptr);
    
    delete strategy;
}

TEST_F(GroupingFactoryTest, CreateStrategy_WithParent_CreatesStrategyWithParent)
{
    // Test creating strategy with parent
    auto strategy = GroupingFactory::createStrategy(GroupStrategy::kNoGroup, parent);
    
    EXPECT_NE(strategy, nullptr);
    EXPECT_EQ(strategy->parent(), parent);
    
    delete strategy;
}

TEST_F(GroupingFactoryTest, CreateStrategy_EmptyString_ReturnsNullptr)
{
    // Test creating strategy with empty string
    auto strategy = GroupingFactory::createStrategy("", parent);
    
    EXPECT_EQ(strategy, nullptr);
}

TEST_F(GroupingFactoryTest, CreateStrategy_DifferentInstances_CreatesDifferentInstances)
{
    // Test that multiple calls create different instances
    auto strategy1 = GroupingFactory::createStrategy(GroupStrategy::kNoGroup, parent);
    auto strategy2 = GroupingFactory::createStrategy(GroupStrategy::kNoGroup, parent);
    
    EXPECT_NE(strategy1, nullptr);
    EXPECT_NE(strategy2, nullptr);
    EXPECT_NE(strategy1, strategy2);
    
    delete strategy1;
    delete strategy2;
}
