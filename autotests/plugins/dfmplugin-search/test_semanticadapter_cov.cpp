// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map for src/plugins/filemanager/dfmplugin-search/searchmanager/searcher/semantic/semanticadapter.cpp
//   SemanticAdapter::SemanticAdapter(QUrl,QString,QObject*) ........ Constructor_CreatesAdapter_*
//   SemanticAdapter::~SemanticAdapter() ............................. Destructor_WithParent_CleansUp
//   SemanticAdapter::search() ...................................... Search_EmptyKeyword_EmitsFinished_* /
//                                                                   Search_WithKeyword_CallsSemanticSearch
//   SemanticAdapter::stop() ........................................ Stop_CallsSemanticCancel
//   SemanticAdapter::hasItem() const ............................... HasItem_* 
//   SemanticAdapter::takeAll() ..................................... TakeAll_WithResults_ReturnsAndClears
//   SemanticAdapter::onIntentParsed(ParsedIntent const&) .......... OnIntentParsed_CachesKeywords
//   SemanticAdapter::processResult(SearchResult const&) ........... covered via OnResultsFound_/OnSearchFinished_*
//   SemanticAdapter::onResultsFound(SearchResultList const&) ...... OnResultsFound_WithResults_EmitsUnearthed
//   SemanticAdapter::onSearchFinished(SearchResultList const&) .... OnSearchFinished_WithResults_EmitsUnearthedAndFinished
//   SemanticAdapter::onSearchCancelled() .......................... OnSearchCancelled_EmitsFinished
//   SemanticAdapter::onErrorOccurred(SearchError const&) .......... OnErrorOccurred_EmitsFinished
// The dfm-search library SemanticSearcher is isolated with stubext (search/cancel),
// signals are emitted on the real child object found via findChild().
// ============================================================================

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QPointer>
#include <QUrl>
#include <QStringList>

#include "stubext.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>

#include <dfm-search/semanticsearcher.h>
#include <dfm-search/semantic_types.h>
#include <dfm-search/searchresult.h>
#include <dfm-search/searcherror.h>

#include "searchmanager/searcher/semantic/semanticadapter.h"
#include "searchmanager/searcher/abstractsearcher.h"

using namespace dfmplugin_search;

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFM_SEARCH_USE_NS

class UT_SemanticAdapterCov : public testing::Test
{
protected:
    void SetUp() override
    {
        // Isolate path conversions from global URL routing state.
        stub.set_lamda(&UrlRoute::urlToPath, [](const QUrl &url) -> QString {
            return url.toLocalFile();
        });
        stub.set_lamda(static_cast<QString (*)(const QString &, bool)>(&FileUtils::bindPathTransform),
                       [](const QString &path, bool) -> QString {
                           return path;
                       });
    }

    void TearDown() override
    {
        stub.clear();
        if (adapter) {
            delete adapter;
            adapter = nullptr;
        }
    }

    // Isolate the adapter from the real semantic backend.
    void isolateSemanticBackend()
    {
        stub.set_lamda(
                static_cast<void (DFMSEARCH::SemanticSearcher::*)(const QString &, const QStringList &)>(
                        &DFMSEARCH::SemanticSearcher::search),
                [this](DFMSEARCH::SemanticSearcher *, const QString &keyword, const QStringList &dirs) {
                    lastSearchKeyword = keyword;
                    lastSearchDirs = dirs;
                });

        stub.set_lamda(&DFMSEARCH::SemanticSearcher::cancel,
                       [this](DFMSEARCH::SemanticSearcher *) {
                           cancelCalled = true;
                       });
    }

    DFMSEARCH::SemanticSearcher *semanticChild() const
    {
        return adapter->findChild<DFMSEARCH::SemanticSearcher *>();
    }

    stub_ext::StubExt stub;
    SemanticAdapter *adapter = nullptr;
    QString lastSearchKeyword;
    QStringList lastSearchDirs;
    bool cancelCalled = false;
};

TEST_F(UT_SemanticAdapterCov, Constructor_CreatesAdapterWithSemanticChild)
{
    // Arrange
    const QUrl url = QUrl::fromLocalFile("/home/test");

    // Act
    adapter = new SemanticAdapter(url, "photos of school", nullptr);

    // Assert
    ASSERT_NE(adapter, nullptr);
    EXPECT_NE(semanticChild(), nullptr);
    EXPECT_EQ(adapter->searchUrl, QUrl::fromLocalFile("/home/test"));
    EXPECT_EQ(adapter->keyword, QString("photos of school"));
}

TEST_F(UT_SemanticAdapterCov, Destructor_WithParent_CleansUp)
{
    // Arrange
    QObject parent;
    auto *scoped = new SemanticAdapter(QUrl::fromLocalFile("/home/test"), "k", &parent);
    QPointer<DFMSEARCH::SemanticSearcher> guard(scoped->findChild<DFMSEARCH::SemanticSearcher *>());
    ASSERT_NE(guard.data(), nullptr);
    QObject *adapterParent = scoped->parent();

    // Act
    delete scoped;

    // Assert
    EXPECT_TRUE(guard.isNull());   // semantic child destroyed together with adapter
    EXPECT_EQ(adapterParent, &parent);
}

TEST_F(UT_SemanticAdapterCov, Search_EmptyKeyword_EmitsFinishedAndReturnsTrue)
{
    // Arrange
    isolateSemanticBackend();
    adapter = new SemanticAdapter(QUrl::fromLocalFile("/home/test"), "", nullptr);
    QSignalSpy finishedSpy(adapter, &SemanticAdapter::finished);

    // Act
    bool result = adapter->search();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_TRUE(lastSearchKeyword.isNull());   // backend search not invoked
}

TEST_F(UT_SemanticAdapterCov, Search_WithKeyword_CallsSemanticSearch)
{
    // Arrange
    isolateSemanticBackend();
    adapter = new SemanticAdapter(QUrl::fromLocalFile("/home/test"), "find documents", nullptr);

    // Act
    bool result = adapter->search();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(lastSearchKeyword, QString("find documents"));
    EXPECT_EQ(lastSearchDirs, QStringList { "/home/test" });
}

TEST_F(UT_SemanticAdapterCov, Stop_CallsSemanticCancel)
{
    // Arrange
    isolateSemanticBackend();
    adapter = new SemanticAdapter(QUrl::fromLocalFile("/home/test"), "find documents", nullptr);

    // Act
    adapter->stop();

    // Assert
    EXPECT_TRUE(cancelCalled);
    EXPECT_EQ(adapter->keyword, QString("find documents"));
}

TEST_F(UT_SemanticAdapterCov, OnResultsFound_WithResults_EmitsUnearthed)
{
    // Arrange
    adapter = new SemanticAdapter(QUrl::fromLocalFile("/home/test"), "find documents", nullptr);
    auto *child = semanticChild();
    ASSERT_NE(child, nullptr);
    QSignalSpy unearthedSpy(adapter, &SemanticAdapter::unearthed);

    // Act
    emit child->resultsFound({ DFMSEARCH::SearchResult("/home/test/report.txt") });

    // Assert
    EXPECT_EQ(unearthedSpy.count(), 1);
    EXPECT_TRUE(adapter->hasItem());
    EXPECT_EQ(adapter->takeAll().size(), 1);
}

TEST_F(UT_SemanticAdapterCov, OnIntentParsed_CachesKeywords)
{
    // Arrange
    adapter = new SemanticAdapter(QUrl::fromLocalFile("/home/test"), "photos of school", nullptr);
    auto *child = semanticChild();
    ASSERT_NE(child, nullptr);
    DFMSEARCH::ParsedIntent intent;
    intent.setKeywords({ "classmate", "school" });

    // Act
    emit child->intentParsed(intent);
    emit child->resultsFound({ DFMSEARCH::SearchResult("/home/test/photo.png") });

    // Assert
    DFMSearchResultMap results = adapter->takeAll();
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results.begin().value().keyword(), QString("classmate school"));
    EXPECT_EQ(results.begin().value().searchType(), DFMSEARCH::SearchType::Semantic);
}

TEST_F(UT_SemanticAdapterCov, TakeAll_WithResults_ReturnsAndClears)
{
    // Arrange
    adapter = new SemanticAdapter(QUrl::fromLocalFile("/home/test"), "k", nullptr);
    auto *child = semanticChild();
    emit child->resultsFound({ DFMSEARCH::SearchResult("/home/test/one.txt"),
                               DFMSEARCH::SearchResult("/home/test/two.txt") });

    // Act
    DFMSearchResultMap results = adapter->takeAll();

    // Assert
    EXPECT_EQ(results.size(), 2);
    EXPECT_FALSE(adapter->hasItem());   // drained

    QList<QUrl> urls = adapter->takeAllUrls();
    EXPECT_TRUE(urls.isEmpty());
}

TEST_F(UT_SemanticAdapterCov, OnSearchFinished_WithResults_EmitsUnearthedAndFinished)
{
    // Arrange
    adapter = new SemanticAdapter(QUrl::fromLocalFile("/home/test"), "k", nullptr);
    auto *child = semanticChild();
    QSignalSpy finishedSpy(adapter, &SemanticAdapter::finished);
    QSignalSpy unearthedSpy(adapter, &SemanticAdapter::unearthed);

    // Act
    emit child->searchFinished({ DFMSEARCH::SearchResult("/home/test/done.txt") });

    // Assert
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(unearthedSpy.count(), 1);
    EXPECT_EQ(adapter->takeAll().size(), 1);
}

TEST_F(UT_SemanticAdapterCov, OnSearchCancelled_EmitsFinished)
{
    // Arrange
    adapter = new SemanticAdapter(QUrl::fromLocalFile("/home/test"), "k", nullptr);
    QSignalSpy finishedSpy(adapter, &SemanticAdapter::finished);

    // Act
    emit semanticChild()->searchCancelled();

    // Assert
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_FALSE(adapter->hasItem());
}

TEST_F(UT_SemanticAdapterCov, OnErrorOccurred_EmitsFinished)
{
    // Arrange
    adapter = new SemanticAdapter(QUrl::fromLocalFile("/home/test"), "k", nullptr);
    QSignalSpy finishedSpy(adapter, &SemanticAdapter::finished);

    // Act
    emit semanticChild()->errorOccurred(DFMSEARCH::SearchError());

    // Assert
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_FALSE(adapter->hasItem());
}
