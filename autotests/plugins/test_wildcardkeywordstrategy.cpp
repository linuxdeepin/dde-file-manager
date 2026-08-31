// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_wildcardkeywordstrategy.cpp
 * @brief Unit tests for WildcardKeywordStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/keywordextractor.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WildcardKeywordStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WildcardKeywordStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WildcardKeywordStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WildcardKeywordStrategyTest, extractFromWildcardPattern)
{
    // Test method: QStringList extractFromWildcardPattern((const QString &pattern))
    QString _arg0{};
    auto result = obj->extractFromWildcardPattern(_arg0);
    EXPECT_TRUE(result.isEmpty());

}
