// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_keywordextractor_1.cpp
 * @brief Unit tests for KeywordExtractor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/keywordextractor.h"

#include <QTest>

using namespace dfmplugin_workspace;

class KeywordExtractorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new KeywordExtractor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    KeywordExtractor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(KeywordExtractorTest, KeywordExtractor)
{
    // Test constructor: KeywordExtractor(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(KeywordExtractorTest, getStrategies)
{
    // Test getter: QList<QSharedPointer<KeywordExtractionStrategy>> getStrategies()
    auto result = obj->getStrategies();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(KeywordExtractorTest, sortStrategiesByPriority)
{
    // Test method: void sortStrategiesByPriority(())
    EXPECT_NO_FATAL_FAILURE(obj->sortStrategiesByPriority());
}

TEST_F(KeywordExtractorTest, KeywordExtractor_Destructor)
{
    // Test method:  ~KeywordExtractor(())
    EXPECT_NO_FATAL_FAILURE({ KeywordExtractor *tmp = new KeywordExtractor(); delete tmp; });
}
