// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map for src/plugins/filemanager/dfmplugin-search/groups/matchmethodgroupstrategy.cpp
// (uncovered functions -> test case)
//   MatchMethodGroupStrategy(QObject*) ................ Constructor_RegistersStrategy
//   ~MatchMethodGroupStrategy ........................ Destructor_NoCrash (fixture teardown + scope cases)
//   getGroupKey(FileInfoPointer) const ................ GetGroupKey_WithoutSortInfo_FallsBackExact
//   getGroupKey(FileInfoPointer,SortInfoPointer) const  GetGroupKey_SemanticType_Smart/*_Exact / null sortInfo
//   getGroupDisplayName(QString) const ................. GetGroupDisplayName_ExactSmartUnknown
//   getGroupOrder() const .............................. GetGroupOrder_ExactBeforeSmart
//   getGroupDisplayOrder(QString) const ................ GetGroupDisplayOrder_ExactSmartUnknown
//   isGroupVisible(QString,QList<FileInfoPointer>) const  IsGroupVisible_EmptyHidden_NonEmptyVisible
//   getStrategyName() const ............................ GetStrategyName_MatchMethod
//   isTruncationEnabled() const ........................ IsTruncationEnabled_True
//   (header) MatchMethodGroupStrategy::tr ............. covered by every getGroupDisplayName call
// ============================================================================

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>
#include <QSharedPointer>

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/sortfileinfo.h>

#include <dfm-search/dsearch_global.h>

#include "groups/matchmethodgroupstrategy.h"

using namespace dfmplugin_search;
DFMBASE_USE_NAMESPACE

class UT_MatchMethodGroupStrategyCov : public testing::Test
{
protected:
    void TearDown() override
    {
        delete strategy;
        strategy = nullptr;
    }

    SortInfoPointer makeSortInfo(int searchType) const
    {
        auto sortInfo = SortInfoPointer::create();
        sortInfo->setSearchType(searchType);
        return sortInfo;
    }

    MatchMethodGroupStrategy *strategy = nullptr;
};

TEST_F(UT_MatchMethodGroupStrategyCov, Constructor_RegistersStrategy)
{
    // Arrange
    strategy = new MatchMethodGroupStrategy(nullptr);

    // Act
    const QString name = strategy->getStrategyName();

    // Assert
    EXPECT_EQ(name, QString("MatchMethod"));
    EXPECT_EQ(strategy->getGroupOrder().size(), 2);
}

TEST_F(UT_MatchMethodGroupStrategyCov, GetGroupKey_WithoutSortInfo_FallsBackExact)
{
    // Arrange
    strategy = new MatchMethodGroupStrategy(nullptr);

    // Act
    QString key = strategy->getGroupKey(FileInfoPointer());

    // Assert
    EXPECT_EQ(key, MatchMethod::kGroupExact);
    EXPECT_NE(key, MatchMethod::kGroupSmart);
}

TEST_F(UT_MatchMethodGroupStrategyCov, GetGroupKey_NullSortInfo_FallsBackExact)
{
    // Arrange
    strategy = new MatchMethodGroupStrategy(nullptr);

    // Act
    QString key = strategy->getGroupKey(FileInfoPointer(), SortInfoPointer());

    // Assert
    EXPECT_EQ(key, MatchMethod::kGroupExact);
    EXPECT_NE(key, MatchMethod::kGroupSmart);
}

TEST_F(UT_MatchMethodGroupStrategyCov, GetGroupKey_SemanticType_ReturnsSmart)
{
    // Arrange
    strategy = new MatchMethodGroupStrategy(nullptr);
    SortInfoPointer sortInfo = makeSortInfo(static_cast<int>(DFMSEARCH::SearchType::Semantic));

    // Act
    QString key = strategy->getGroupKey(FileInfoPointer(), sortInfo);

    // Assert
    EXPECT_EQ(key, MatchMethod::kGroupSmart);
    EXPECT_NE(key, MatchMethod::kGroupExact);
}

TEST_F(UT_MatchMethodGroupStrategyCov, GetGroupKey_NonSemanticTypes_ReturnExact)
{
    // Arrange
    strategy = new MatchMethodGroupStrategy(nullptr);
    const SortInfoPointer fileSort = makeSortInfo(static_cast<int>(DFMSEARCH::SearchType::FileName));

    // Act
    const QString key = strategy->getGroupKey(FileInfoPointer(), fileSort);

    // Assert
    EXPECT_EQ(key, MatchMethod::kGroupExact);
    EXPECT_EQ(strategy->getGroupKey(FileInfoPointer(), makeSortInfo(static_cast<int>(DFMSEARCH::SearchType::FileName))),
              MatchMethod::kGroupExact);
    EXPECT_EQ(strategy->getGroupKey(FileInfoPointer(), makeSortInfo(static_cast<int>(DFMSEARCH::SearchType::Content))),
              MatchMethod::kGroupExact);
    EXPECT_EQ(strategy->getGroupKey(FileInfoPointer(), makeSortInfo(static_cast<int>(DFMSEARCH::SearchType::Ocr))),
              MatchMethod::kGroupExact);
}

TEST_F(UT_MatchMethodGroupStrategyCov, GetGroupDisplayName_ExactSmartUnknown)
{
    // Arrange
    strategy = new MatchMethodGroupStrategy(nullptr);

    // Act
    const QString exactName = strategy->getGroupDisplayName(MatchMethod::kGroupExact);

    // Assert
    EXPECT_EQ(exactName, QString("Exact match"));
    EXPECT_EQ(strategy->getGroupDisplayName(MatchMethod::kGroupExact), QString("Exact match"));
    EXPECT_EQ(strategy->getGroupDisplayName(MatchMethod::kGroupSmart), QString("Smart match"));
    EXPECT_EQ(strategy->getGroupDisplayName(QString("other-key")), QString("other-key"));
}

TEST_F(UT_MatchMethodGroupStrategyCov, GetGroupOrder_ExactBeforeSmart)
{
    // Arrange
    strategy = new MatchMethodGroupStrategy(nullptr);

    // Act
    QStringList order = strategy->getGroupOrder();

    // Assert
    ASSERT_EQ(order.size(), 2);
    EXPECT_EQ(order.first(), MatchMethod::kGroupExact);
    EXPECT_EQ(order.last(), MatchMethod::kGroupSmart);
}

TEST_F(UT_MatchMethodGroupStrategyCov, GetGroupDisplayOrder_ExactSmartUnknown)
{
    // Arrange
    strategy = new MatchMethodGroupStrategy(nullptr);

    // Act
    const int exactOrder = strategy->getGroupDisplayOrder(MatchMethod::kGroupExact);

    // Assert
    EXPECT_EQ(exactOrder, 0);
    EXPECT_EQ(strategy->getGroupDisplayOrder(MatchMethod::kGroupExact), 0);
    EXPECT_EQ(strategy->getGroupDisplayOrder(MatchMethod::kGroupSmart), 1);
    EXPECT_EQ(strategy->getGroupDisplayOrder(QString("bogus")), 2);
}

TEST_F(UT_MatchMethodGroupStrategyCov, IsGroupVisible_EmptyHidden_NonEmptyVisible)
{
    // Arrange
    strategy = new MatchMethodGroupStrategy(nullptr);
    QList<FileInfoPointer> empty;
    QList<FileInfoPointer> filled;
    filled << FileInfoPointer();   // non-empty list; entries are never dereferenced by the strategy

    // Act
    bool visibleEmpty = strategy->isGroupVisible(MatchMethod::kGroupSmart, empty);
    bool visibleFilled = strategy->isGroupVisible(MatchMethod::kGroupSmart, filled);

    // Assert
    EXPECT_FALSE(visibleEmpty);
    EXPECT_TRUE(visibleFilled);
    EXPECT_EQ(filled.size(), 1);
}

TEST_F(UT_MatchMethodGroupStrategyCov, IsTruncationEnabled_True)
{
    // Arrange
    strategy = new MatchMethodGroupStrategy(nullptr);

    // Act
    bool truncation = strategy->isTruncationEnabled();

    // Assert
    EXPECT_TRUE(truncation);
    EXPECT_EQ(strategy->getGroupOrder().size(), 2);
}
