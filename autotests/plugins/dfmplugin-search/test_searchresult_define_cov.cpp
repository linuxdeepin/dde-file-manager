// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map for searchmanager/searcher/searchresult_define.h
//   DFMSearchResultData::DFMSearchResultData(QUrl const&) ... DataCtor_StoresUrl
//   DFMSearchResult::DFMSearchResult(QUrl const&) ........... ResultCtor_StoresUrl_Defaults
//   DFMSearchResult::keyword() const ......................... Keyword_GetSetRoundTrip
//   DFMSearchResult::setKeyword(QString const&) .............. Keyword_GetSetRoundTrip
//   DFMSearchResult::searchType() const ...................... SearchType_GetSetRoundTrip
//   DFMSearchResult::setSearchType(SearchType) ............... SearchType_GetSetRoundTrip
// ============================================================================

#include <gtest/gtest.h>
#include <QUrl>
#include <QString>

#include <dfm-search/dsearch_global.h>

#include "searchmanager/searcher/searchresult_define.h"

using namespace dfmplugin_search;
DFM_SEARCH_USE_NS

class UT_SearchResultDefineCov : public testing::Test
{
};

TEST_F(UT_SearchResultDefineCov, DataCtor_StoresUrl)
{
    // Arrange
    const QUrl url = QUrl::fromLocalFile("/tmp/data-result.txt");

    // Act
    DFMSearchResultData data(url);

    // Assert
    EXPECT_EQ(data.url, url);
    EXPECT_TRUE(data.keyword.isEmpty());
    EXPECT_DOUBLE_EQ(data.matchScore, 0.0);
}

TEST_F(UT_SearchResultDefineCov, ResultCtor_StoresUrl_Defaults)
{
    // Arrange
    const QUrl url = QUrl::fromLocalFile("/tmp/result.txt");

    // Act
    DFMSearchResult result(url);

    // Assert
    EXPECT_EQ(result.url(), url);
    EXPECT_EQ(result.searchType(), SearchType::FileName);   // default type
    EXPECT_TRUE(result.keyword().isEmpty());
}

TEST_F(UT_SearchResultDefineCov, Keyword_GetSetRoundTrip)
{
    // Arrange
    DFMSearchResult result(QUrl::fromLocalFile("/tmp/kw.txt"));

    // Act
    result.setKeyword(QStringLiteral("semester report"));

    // Assert
    EXPECT_EQ(result.keyword(), QString("semester report"));
    EXPECT_EQ(result.url(), QUrl::fromLocalFile("/tmp/kw.txt"));
}

TEST_F(UT_SearchResultDefineCov, SearchType_GetSetRoundTrip)
{
    // Arrange
    DFMSearchResult result(QUrl::fromLocalFile("/tmp/type.txt"));

    // Act
    result.setSearchType(SearchType::Ocr);

    // Assert
    EXPECT_EQ(result.searchType(), SearchType::Ocr);
    EXPECT_NE(result.searchType(), SearchType::FileName);
}

TEST_F(UT_SearchResultDefineCov, MatchScore_SetIsReflectedInCopy)
{
    // Arrange
    DFMSearchResult result(QUrl::fromLocalFile("/tmp/score.txt"));

    // Act
    result.setMatchScore(0.75);
    DFMSearchResult copy = result;   // implicit sharing

    // Assert
    EXPECT_DOUBLE_EQ(copy.matchScore(), 0.75);
    EXPECT_DOUBLE_EQ(result.matchScore(), 0.75);
}
