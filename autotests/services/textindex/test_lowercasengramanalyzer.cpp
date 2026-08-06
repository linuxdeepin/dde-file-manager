// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_lowercasengramanalyzer.cpp
 * @brief Unit tests for LowerCaseNGramAnalyzer (profile/lowercasengramanalyzer.cpp)
 */

#include <gtest/gtest.h>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/lowercasengramanalyzer.h"

#include <lucene++/LuceneHeaders.h>

using namespace SERVICETEXTINDEX_NAMESPACE;

TEST(LowerCaseNGramAnalyzerTest, ConstructAndDestruct)
{
    EXPECT_NO_FATAL_FAILURE({ LowerCaseNGramAnalyzer analyzer(2, 5); });
}

TEST(LowerCaseNGramAnalyzerTest, TokenStreamReturnsNonNull)
{
    LowerCaseNGramAnalyzer analyzer(2, 5);
    Lucene::String fieldName = L"content";
    Lucene::ReaderPtr reader = Lucene::newLucene<Lucene::StringReader>(L"hello world");
    Lucene::TokenStreamPtr stream = analyzer.tokenStream(fieldName, reader);
    EXPECT_NE(stream, nullptr);
}

TEST(LowerCaseNGramAnalyzerTest, ReusableTokenStreamReturnsNonNull)
{
    LowerCaseNGramAnalyzer analyzer(2, 5);
    Lucene::String fieldName = L"content";
    Lucene::ReaderPtr reader = Lucene::newLucene<Lucene::StringReader>(L"test text");
    Lucene::TokenStreamPtr stream = analyzer.reusableTokenStream(fieldName, reader);
    EXPECT_NE(stream, nullptr);
}
