// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map for src/plugins/filemanager/dfmplugin-search/searchmanager/searcher/dfmsearch/querystrategies.cpp
// (uncovered functions -> test case)
//   BooleanQueryStrategy::createQuery(QString) const ... BooleanQuery_CreateQuery_MultiKeyword_BuildsBooleanQuery
//   BooleanQueryStrategy::canHandle(QString,SearchType) const ... BooleanQuery_CanHandle_WithAndWithoutWhitespace
//   QueryTypeSelector::createQuery(QString,SearchType) const ... Selector_CreateQuery_DispatchesBySyntax
// ============================================================================

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>

#include "searchmanager/searcher/dfmsearch/querystrategies.h"

using namespace dfmplugin_search;
DPSEARCH_USE_NAMESPACE
DFM_SEARCH_USE_NS

class UT_QueryStrategiesCov : public testing::Test
{
protected:
    BooleanQueryStrategy booleanStrategy;
    QueryTypeSelector selector;
};

TEST_F(UT_QueryStrategiesCov, BooleanQuery_CanHandle_WithAndWithoutWhitespace)
{
    // Arrange
    const QString multi = QStringLiteral("hello world");

    // Act
    bool multiWord = booleanStrategy.canHandle(multi, SearchType::FileName);
    bool singleWord = booleanStrategy.canHandle(QStringLiteral("hello"), SearchType::FileName);

    // Assert
    EXPECT_TRUE(multiWord);
    EXPECT_FALSE(singleWord);
    EXPECT_EQ(multi.size(), 11);
}

TEST_F(UT_QueryStrategiesCov, BooleanQuery_CreateQuery_MultiKeyword_BuildsBooleanQuery)
{
    // Arrange
    const QString keyword = QStringLiteral("alpha beta gamma");

    // Act
    SearchQuery query = booleanStrategy.createQuery(keyword);

    // Assert
    EXPECT_EQ(query.type(), SearchQuery::Type::Boolean);
    EXPECT_EQ(query.booleanOperator(), SearchQuery::BooleanOperator::AND);
}

TEST_F(UT_QueryStrategiesCov, Selector_CreateQuery_DispatchesBySyntax)
{
    // Arrange
    const QString multiWord = QStringLiteral("one two");

    // Act
    SearchQuery boolean = selector.createQuery(multiWord, SearchType::FileName);

    // Assert
    EXPECT_EQ(boolean.type(), SearchQuery::Type::Boolean);
    EXPECT_EQ(boolean.type(), SearchQuery::Type::Boolean);

    SearchQuery wildcard = selector.createQuery(QStringLiteral("*.txt"), SearchType::FileName);
    EXPECT_EQ(wildcard.type(), SearchQuery::Type::Wildcard);

    SearchQuery simple = selector.createQuery(QStringLiteral("plain"), SearchType::Content);
    EXPECT_EQ(simple.type(), SearchQuery::Type::Simple);
}
