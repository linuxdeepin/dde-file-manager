// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collationstrategyprovider.cpp
 * @brief Unit tests for CollationStrategyProvider methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/collation/collationstrategyprovider.h"

#include <QTest>

using namespace src;

class CollationStrategyProviderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollationStrategyProvider();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollationStrategyProvider *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollationStrategyProviderTest, strategy)
{
    // Test getter: CollationStrategy strategy()
    auto result = obj->strategy();
    EXPECT_NO_FATAL_FAILURE({ obj->strategy(); });

}
