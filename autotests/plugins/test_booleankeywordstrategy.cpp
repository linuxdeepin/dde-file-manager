// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_booleankeywordstrategy.cpp
 * @brief Unit tests for BooleanKeywordStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/keywordextractor.h"

#include <QTest>

using namespace dfmplugin_workspace;

class BooleanKeywordStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BooleanKeywordStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BooleanKeywordStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BooleanKeywordStrategyTest, canHandle)
{
    // Test method: bool canHandle((const QString &keyword))
    QString _arg0{};
    auto result = obj->canHandle(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BooleanKeywordStrategyTest, extractKeywords)
{
    // Test method: QStringList extractKeywords((const QString &keyword))
    QString _arg0{};
    auto result = obj->extractKeywords(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BooleanKeywordStrategyTest, priority)
{
    // Test getter: int priority()
    auto result = obj->priority();
    EXPECT_EQ(result, 0);

}
