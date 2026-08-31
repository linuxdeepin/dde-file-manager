// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_lowercasengramanalyzer.cpp
 * @brief Unit tests for LowerCaseNGramAnalyzer methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/profile/lowercasengramanalyzer.h"

#include <QTest>

using namespace src;

class LowerCaseNGramAnalyzerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new LowerCaseNGramAnalyzer();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    LowerCaseNGramAnalyzer *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(LowerCaseNGramAnalyzerTest, LowerCaseNGramAnalyzer)
{
    // Test constructor: LowerCaseNGramAnalyzer((int32_t minGram, int32_t maxGram))
    ASSERT_NE(obj, nullptr);
}

TEST_F(LowerCaseNGramAnalyzerTest, reusableTokenStream)
{
    // Test method: Lucene::TokenStreamPtr reusableTokenStream((const Lucene::String &fieldName,
                                                                   const Lucene::ReaderPtr &reader))
    Lucene::String _arg0{};
    Lucene::ReaderPtr _arg1{};
    auto result = obj->reusableTokenStream(_arg0, _arg1);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(LowerCaseNGramAnalyzerTest, tokenStream)
{
    // Test method: Lucene::TokenStreamPtr tokenStream((const Lucene::String &fieldName,
                                                           const Lucene::ReaderPtr &reader))
    Lucene::String _arg0{};
    Lucene::ReaderPtr _arg1{};
    auto result = obj->tokenStream(_arg0, _arg1);
    EXPECT_NE(result.get(), nullptr);

}
