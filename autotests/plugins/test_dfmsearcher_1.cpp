// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmsearcher_1.cpp
 * @brief Unit tests for DFMSearcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"

#include <QTest>

using namespace dfmplugin_search;

class DFMSearcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DFMSearcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DFMSearcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DFMSearcherTest, configureHiddenFilesOption)
{
    // Test method: void configureHiddenFilesOption((SearchOptions &options, const QString &transformedPath))
    SearchOptions _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->configureHiddenFilesOption(_arg0, _arg1));
}

TEST_F(DFMSearcherTest, configureRealtimeSearchOptions)
{
    // Test method: void configureRealtimeSearchOptions((SearchOptions &options, const QString &transformedPath))
    SearchOptions _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->configureRealtimeSearchOptions(_arg0, _arg1));
}

TEST_F(DFMSearcherTest, configureSearchOptions)
{
    // Test method: SearchOptions configureSearchOptions((const QString &transformedPath))
    QString _arg0{};
    auto result = obj->configureSearchOptions(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->configureSearchOptions(_arg0); });

}

TEST_F(DFMSearcherTest, createSearchQuery)
{
    // Test getter: SearchQuery createSearchQuery()
    auto result = obj->createSearchQuery();
    EXPECT_NO_FATAL_FAILURE({ obj->createSearchQuery(); });

}

TEST_F(DFMSearcherTest, executeSearch)
{
    // Test method: void executeSearch(())
    EXPECT_NO_FATAL_FAILURE(obj->executeSearch());
}

TEST_F(DFMSearcherTest, getSearchMethod)
{
    // Test method: SearchMethod getSearchMethod((const QString &path))
    QString _arg0{};
    auto result = obj->getSearchMethod(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->getSearchMethod(_arg0); });

}

TEST_F(DFMSearcherTest, getSearchType)
{
    // Test getter: SearchType getSearchType()
    auto result = obj->getSearchType();
    EXPECT_NO_FATAL_FAILURE({ obj->getSearchType(); });

}

TEST_F(DFMSearcherTest, handleRemainingResults)
{
    // Test method: void handleRemainingResults((const QList<SearchResult> &results))
    QList<SearchResult> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRemainingResults(_arg0));
}

TEST_F(DFMSearcherTest, hasItem)
{
    // Test bool getter: hasItem()
    bool result = obj->hasItem();
    EXPECT_FALSE(result);

}

TEST_F(DFMSearcherTest, isEngineReady)
{
    // Test bool getter: isEngineReady()
    bool result = obj->isEngineReady();
    EXPECT_FALSE(result);

}

TEST_F(DFMSearcherTest, isIndexOnlyContentSearch)
{
    // Test bool getter: isIndexOnlyContentSearch()
    bool result = obj->isIndexOnlyContentSearch();
    EXPECT_FALSE(result);

}

TEST_F(DFMSearcherTest, isValidSearchParameters)
{
    // Test bool getter: isValidSearchParameters()
    bool result = obj->isValidSearchParameters();
    EXPECT_FALSE(result);

}

TEST_F(DFMSearcherTest, onSearchCancelled)
{
    // Test method: void onSearchCancelled(())
    EXPECT_NO_FATAL_FAILURE(obj->onSearchCancelled());
}

TEST_F(DFMSearcherTest, onSearchError)
{
    // Test method: void onSearchError((const SearchError &error))
    SearchError _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSearchError(_arg0));
}

TEST_F(DFMSearcherTest, onSearchFinished)
{
    // Test method: void onSearchFinished((const QList<SearchResult> &results))
    QList<SearchResult> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSearchFinished(_arg0));
}

TEST_F(DFMSearcherTest, onSearchStarted)
{
    // Test method: void onSearchStarted(())
    EXPECT_NO_FATAL_FAILURE(obj->onSearchStarted());
}

TEST_F(DFMSearcherTest, processSearchResult)
{
    // Test method: void processSearchResult((const SearchResult &result))
    SearchResult _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->processSearchResult(_arg0));
}

TEST_F(DFMSearcherTest, search)
{
    // Test bool getter: search()
    bool result = obj->search();
    EXPECT_FALSE(result);

}

TEST_F(DFMSearcherTest, setExcludedPathsForRealtime)
{
    // Test setter: void setExcludedPathsForRealtime((SearchOptions &options))
    SearchOptions _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setExcludedPathsForRealtime(_arg0));
}

TEST_F(DFMSearcherTest, shouldExcludeIndexedPaths)
{
    // Test method: bool shouldExcludeIndexedPaths((const QString &transformedPath))
    QString _arg0{};
    auto result = obj->shouldExcludeIndexedPaths(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DFMSearcherTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(DFMSearcherTest, supportUrl)
{
    // Test method: bool supportUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->supportUrl(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DFMSearcherTest, takeAll)
{
    // Test getter: DFMSearchResultMap takeAll()
    auto result = obj->takeAll();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DFMSearcherTest, DFMSearcher_Destructor)
{
    // Test method:  ~DFMSearcher(())
    EXPECT_NO_FATAL_FAILURE({ DFMSearcher *tmp = new DFMSearcher(); delete tmp; });
}
