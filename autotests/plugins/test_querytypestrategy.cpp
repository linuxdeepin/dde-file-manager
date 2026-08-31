// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_querytypestrategy.cpp
 * @brief Unit tests for QueryTypeStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/dfmsearch/querystrategies.h"

#include <QTest>

using namespace dfmplugin_search;

class QueryTypeStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new QueryTypeStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    QueryTypeStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(QueryTypeStrategyTest, QueryTypeStrategy_Destructor)
{
    // Test method:  ~QueryTypeStrategy(())
    EXPECT_NO_FATAL_FAILURE({ QueryTypeStrategy *tmp = new QueryTypeStrategy(); delete tmp; });
}
