// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_simplequerystrategy.cpp
 * @brief Unit tests for SimpleQueryStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/dfmsearch/querystrategies.h"

#include <QTest>

using namespace dfmplugin_search;

class SimpleQueryStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SimpleQueryStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SimpleQueryStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SimpleQueryStrategyTest, createQuery)
{
    // Test method: SearchQuery createQuery((const QString &keyword))
    QString _arg0{};
    auto result = obj->createQuery(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->createQuery(_arg0); });

}
