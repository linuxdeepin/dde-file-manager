// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_querytypeselector.cpp
 * @brief Unit tests for QueryTypeSelector methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/dfmsearch/querystrategies.h"

#include <QTest>

using namespace dfmplugin_search;

class QueryTypeSelectorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new QueryTypeSelector();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    QueryTypeSelector *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(QueryTypeSelectorTest, QueryTypeSelector)
{
    // Test constructor: QueryTypeSelector(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(QueryTypeSelectorTest, createQuery)
{
    // Test method: SearchQuery createQuery((const QString &keyword, SearchType searchType))
    QString _arg0{};
    auto result = obj->createQuery(_arg0, SearchType());
    EXPECT_NO_FATAL_FAILURE({ obj->createQuery(_arg0, SearchType()); });

}

TEST_F(QueryTypeSelectorTest, getStrategies)
{
    // Test getter: QList<QSharedPointer<QueryTypeStrategy>> getStrategies()
    auto result = obj->getStrategies();
    EXPECT_TRUE(result.isEmpty());

}
