// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_keywordextractionstrategy.cpp
 * @brief Unit tests for KeywordExtractionStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/keywordextractor.h"

#include <QTest>

using namespace dfmplugin_workspace;

class KeywordExtractionStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new KeywordExtractionStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    KeywordExtractionStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(KeywordExtractionStrategyTest, KeywordExtractionStrategy_Destructor)
{
    // Test method:  ~KeywordExtractionStrategy(())
    EXPECT_NO_FATAL_FAILURE({ KeywordExtractionStrategy *tmp = new KeywordExtractionStrategy(); delete tmp; });
}
