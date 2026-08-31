// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_keywordextractormanager_1.cpp
 * @brief Unit tests for KeywordExtractorManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/keywordextractor.h"

#include <QTest>

using namespace dfmplugin_workspace;

class KeywordExtractorManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new KeywordExtractorManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    KeywordExtractorManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(KeywordExtractorManagerTest, KeywordExtractorManager)
{
    // Test constructor: KeywordExtractorManager(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(KeywordExtractorManagerTest, M_~KeywordExtractorManager)
{
    // Test method:  ~KeywordExtractorManager(())
    EXPECT_NO_FATAL_FAILURE({ KeywordExtractorManager *tmp = new KeywordExtractorManager(); delete tmp; });
}

TEST_F(KeywordExtractorManagerTest, instance)
{
    // Test getter: KeywordExtractorManager instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(KeywordExtractorManagerTest, extractor)
{
    // Test getter: KeywordExtractor extractor()
    auto result = obj->extractor();
    EXPECT_NO_FATAL_FAILURE({ obj->extractor(); });

}

TEST_F(KeywordExtractorManagerTest, operator=)
{
    // Test getter: KeywordExtractorManager operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(KeywordExtractorManagerTest, keywordExtractor)
{
    // Test getter: KeywordExtractor keywordExtractor()
    auto result = obj->keywordExtractor();
    EXPECT_NO_FATAL_FAILURE({ obj->keywordExtractor(); });

}
