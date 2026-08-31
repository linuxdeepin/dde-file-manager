// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wildcardquerystrategy.cpp
 * @brief Unit tests for WildcardQueryStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/dfmsearch/querystrategies.h"

#include <QTest>

using namespace dfmplugin_search;

class WildcardQueryStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WildcardQueryStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WildcardQueryStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WildcardQueryStrategyTest, canHandle)
{
    // Test method: bool canHandle((const QString &keyword, SearchType searchType))
    QString _arg0{};
    auto result = obj->canHandle(_arg0, SearchType());
    EXPECT_FALSE(result);

}

TEST_F(WildcardQueryStrategyTest, createQuery)
{
    // Test method: SearchQuery createQuery((const QString &keyword))
    QString _arg0{};
    auto result = obj->createQuery(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createQuery(_arg0); });

}
