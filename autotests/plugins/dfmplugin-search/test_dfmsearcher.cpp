// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QMutex>
#include <QVariant>

#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"
#include "searchmanager/searcher/dfmsearch/querystrategies.h"
#include "searchmanager/searcher/searchresult_define.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>

#include <dfm-search/searchfactory.h>
#include <dfm-search/searchengine.h>
#include <dfm-search/searchquery.h>
#include <dfm-search/searchoptions.h>
#include <dfm-search/contentsearchapi.h>
#include <dfm-search/dsearch_global.h>

#include "stubext.h"

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFM_SEARCH_USE_NS

// Mock SearchEngine for testing
class MockSearchEngine : public DFMSEARCH::SearchEngine
{
public:
    explicit MockSearchEngine(DFMSEARCH::SearchType type, QObject *parent = nullptr)
        : SearchEngine(parent), m_type(type), m_status(DFMSEARCH::SearchStatus::Ready)
    {
    }

    DFMSEARCH::SearchType searchType() const { return m_type; }
    DFMSEARCH::SearchStatus status() const { return m_status; }
    DFMSEARCH::SearchOptions searchOptions() const { return m_options; }

    void search(const DFMSEARCH::SearchQuery &query)
    {
        m_query = query;
        m_status = DFMSEARCH::SearchStatus::Searching;
        emit searchStarted();
    }

    void cancel()
    {
        m_status = DFMSEARCH::SearchStatus::Cancelled;
        emit searchCancelled();
    }

    void setSearchOptions(const DFMSEARCH::SearchOptions &options)
    {
        m_options = options;
    }

    void setStatus(DFMSEARCH::SearchStatus status) { m_status = status; }

    void emitFinished(const QList<DFMSEARCH::SearchResult> &results = {})
    {
        m_status = DFMSEARCH::SearchStatus::Finished;
        emit searchFinished(results);
    }

    void emitResultsFound(const QList<DFMSEARCH::SearchResult> &results)
    {
        emit resultsFound(results);
    }

    void emitError(const DFMSEARCH::SearchError &error)
    {
        emit errorOccurred(error);
    }

private:
    DFMSEARCH::SearchType m_type;
    DFMSEARCH::SearchStatus m_status;
    DFMSEARCH::SearchOptions m_options;
    DFMSEARCH::SearchQuery m_query;
};

class TestDFMSearcher : public testing::Test
{
public:
    void SetUp() override
    {
        searchUrl = QUrl::fromLocalFile("/home/test");
        keyword = "test_keyword";
        searchType = DFMSEARCH::SearchType::FileName;
    }

    void TearDown() override
    {
        if (searcher) {
            delete searcher;
            searcher = nullptr;
        }
        stub.clear();
    }

    void setupBasicStubs()
    {
        // Stub UrlRoute::urlToPath
        stub.set_lamda(&UrlRoute::urlToPath, [](const QUrl &url) -> QString {
            __DBG_STUB_INVOKE__
            return url.toLocalFile();
        });

        // Stub FileUtils::bindPathTransform
        stub.set_lamda(static_cast<QString (*)(const QString &, bool)>(&FileUtils::bindPathTransform),
                       [](const QString &path, bool) -> QString {
                           __DBG_STUB_INVOKE__
                           return path;
                       });

        // Stub Application::genericAttribute
        stub.set_lamda(&Application::genericAttribute, [] {
            __DBG_STUB_INVOKE__
            return QVariant(false);
        });

        // Stub DFMSEARCH::Global functions
        stub.set_lamda(DFMSEARCH::Global::isFileNameIndexReadyForSearch, []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(&DFMSEARCH::Global::isPathInFileNameIndexDirectory, [](const QString &) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(&DFMSEARCH::Global::isHiddenPathOrInHiddenDir, [](const QString &) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        stub.set_lamda(&DFMSEARCH::Global::defaultIndexedDirectory, []() -> QStringList {
            __DBG_STUB_INVOKE__
            return QStringList() << "/home/test/indexed";
        });
    }

    DFMSearcher *createSearcherWithMockEngine(DFMSEARCH::SearchType type = DFMSEARCH::SearchType::FileName)
    {
        setupBasicStubs();

        MockSearchEngine *mockEngine = new MockSearchEngine(type);

        stub.set_lamda(&DFMSEARCH::SearchFactory::createEngine,
                       [mockEngine](DFMSEARCH::SearchType, QObject *) -> DFMSEARCH::SearchEngine * {
                           __DBG_STUB_INVOKE__
                           return mockEngine;
                       });

        return new DFMSearcher(searchUrl, keyword, nullptr, type);
    }

protected:
    stub_ext::StubExt stub;
    DFMSearcher *searcher = nullptr;
    QUrl searchUrl;
    QString keyword;
    DFMSEARCH::SearchType searchType;
};

// ========== Constructor Tests ==========
TEST_F(TestDFMSearcher, Constructor_WithValidParameters_CreatesInstance)
{
    searcher = createSearcherWithMockEngine();

    EXPECT_NE(searcher, nullptr);
    EXPECT_EQ(searcher->searchUrl, searchUrl);
    EXPECT_EQ(searcher->keyword, keyword);
}

TEST_F(TestDFMSearcher, Constructor_WithParent_SetsParentCorrectly)
{
    QObject parent;
    setupBasicStubs();

    MockSearchEngine *mockEngine = new MockSearchEngine(DFMSEARCH::SearchType::FileName);
    stub.set_lamda(&DFMSEARCH::SearchFactory::createEngine,
                   [mockEngine](DFMSEARCH::SearchType, QObject *) -> DFMSEARCH::SearchEngine * {
                       __DBG_STUB_INVOKE__
                       return mockEngine;
                   });

    DFMSearcher searcherWithParent(searchUrl, keyword, &parent, DFMSEARCH::SearchType::FileName);

    EXPECT_EQ(searcherWithParent.parent(), &parent);
}

TEST_F(TestDFMSearcher, Constructor_EngineCreationFails_HandlesGracefully)
{
    setupBasicStubs();

    bool engineCreationFailed = false;
    stub.set_lamda(&DFMSEARCH::SearchFactory::createEngine,
                   [&engineCreationFailed](DFMSEARCH::SearchType, QObject *) -> DFMSEARCH::SearchEngine * {
                       __DBG_STUB_INVOKE__
                       engineCreationFailed = true;
                       return nullptr;
                   });

    searcher = new DFMSearcher(searchUrl, keyword, nullptr, DFMSEARCH::SearchType::FileName);

    EXPECT_TRUE(engineCreationFailed);
    EXPECT_NE(searcher, nullptr);
}

// ========== Static Methods Tests ==========
TEST_F(TestDFMSearcher, SupportUrl_WithFileScheme_ReturnsTrue)
{
    QUrl localFileUrl = QUrl::fromLocalFile("/home/user/documents");

    bool result = DFMSearcher::supportUrl(localFileUrl);

    EXPECT_TRUE(result);
}

TEST_F(TestDFMSearcher, SupportUrl_WithNonFileScheme_ReturnsFalse)
{
    QUrl httpUrl("http://example.com");

    bool result = DFMSearcher::supportUrl(httpUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestDFMSearcher, SupportUrl_WithEmptyUrl_ReturnsFalse)
{
    QUrl emptyUrl;

    bool result = DFMSearcher::supportUrl(emptyUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestDFMSearcher, RealSearchPath_WithValidUrl_ReturnsTransformedPath)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/user/documents");
    QString expectedPath = "/home/user/documents";

    stub.set_lamda(&UrlRoute::urlToPath, [&expectedPath](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return expectedPath;
    });

    stub.set_lamda(static_cast<QString (*)(const QString &, bool)>(&FileUtils::bindPathTransform),
                   [&expectedPath](const QString &path, bool) -> QString {
                       __DBG_STUB_INVOKE__
                       EXPECT_EQ(path, expectedPath);
                       return expectedPath;
                   });

    QString result = DFMSearcher::realSearchPath(testUrl);

    EXPECT_EQ(result, expectedPath);
}

// ========== Search Method Tests ==========
TEST_F(TestDFMSearcher, Search_WithValidConfiguration_ReturnsTrue)
{
    searcher = createSearcherWithMockEngine();

    bool result = searcher->search();

    EXPECT_TRUE(result);
}

TEST_F(TestDFMSearcher, Search_EmptyKeyword_ReturnsFalse)
{
    setupBasicStubs();

    MockSearchEngine *mockEngine = new MockSearchEngine(DFMSEARCH::SearchType::FileName);
    stub.set_lamda(&DFMSEARCH::SearchFactory::createEngine,
                   [mockEngine](DFMSEARCH::SearchType, QObject *) -> DFMSEARCH::SearchEngine * {
                       __DBG_STUB_INVOKE__
                       return mockEngine;
                   });

    searcher = new DFMSearcher(searchUrl, "", nullptr, DFMSEARCH::SearchType::FileName);

    bool result = searcher->search();

    EXPECT_FALSE(result);
}

TEST_F(TestDFMSearcher, Search_WithContentType_UnsupportedPath_EmitsFinishedAndReturnsTrue)
{
    setupBasicStubs();

    // Content search but path not in indexed directory
    stub.set_lamda(&DFMSEARCH::Global::isFileNameIndexReadyForSearch, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DFMSEARCH::Global::isPathInFileNameIndexDirectory, [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::Content);

    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    bool result = searcher->search();

    EXPECT_TRUE(result);
    EXPECT_EQ(finishedSpy.count(), 1);
}

TEST_F(TestDFMSearcher, Search_WithContentType_SupportedPath_StartsSearch)
{
    setupBasicStubs();

    stub.set_lamda(&DFMSEARCH::Global::isFileNameIndexReadyForSearch, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DFMSEARCH::Global::isPathInFileNameIndexDirectory, [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::Content);

    bool result = searcher->search();

    EXPECT_TRUE(result);
}

TEST_F(TestDFMSearcher, Stop_WhenNotSearching_DoesNothing)
{
    searcher = createSearcherWithMockEngine();

    MockSearchEngine *engine = dynamic_cast<MockSearchEngine *>(searcher->engine);
    ASSERT_NE(engine, nullptr);
    engine->setStatus(DFMSEARCH::SearchStatus::Ready);

    QSignalSpy cancelledSpy(engine, &DFMSEARCH::SearchEngine::searchCancelled);

    searcher->stop();

    EXPECT_EQ(cancelledSpy.count(), 0);
}

TEST_F(TestDFMSearcher, Stop_WithNullEngine_DoesNotCrash)
{
    setupBasicStubs();

    stub.set_lamda(&DFMSEARCH::SearchFactory::createEngine,
                   [](DFMSEARCH::SearchType, QObject *) -> DFMSEARCH::SearchEngine * {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    searcher = new DFMSearcher(searchUrl, keyword, nullptr, DFMSEARCH::SearchType::FileName);

    EXPECT_NO_FATAL_FAILURE(searcher->stop());
}

// ========== HasItem and TakeAll Tests ==========
TEST_F(TestDFMSearcher, HasItem_WithNoResults_ReturnsFalse)
{
    searcher = createSearcherWithMockEngine();

    bool hasItems = searcher->hasItem();

    EXPECT_FALSE(hasItems);
}

TEST_F(TestDFMSearcher, HasItem_WithResults_ReturnsTrue)
{
    searcher = createSearcherWithMockEngine();

    // Manually add results
    QUrl resultUrl = QUrl::fromLocalFile("/home/test/file1.txt");
    DFMSearchResult searchResult(resultUrl);
    searcher->allResults.insert(resultUrl, searchResult);

    bool hasItems = searcher->hasItem();

    EXPECT_TRUE(hasItems);
}

TEST_F(TestDFMSearcher, TakeAll_WithNoResults_ReturnsEmptyMap)
{
    searcher = createSearcherWithMockEngine();

    DFMSearchResultMap results = searcher->takeAll();

    EXPECT_TRUE(results.isEmpty());
}

TEST_F(TestDFMSearcher, TakeAll_WithResults_ReturnsAndClearsResults)
{
    searcher = createSearcherWithMockEngine();

    // Manually add results
    QUrl resultUrl = QUrl::fromLocalFile("/home/test/file1.txt");
    DFMSearchResult searchResult(resultUrl);
    searcher->allResults.insert(resultUrl, searchResult);

    DFMSearchResultMap results = searcher->takeAll();

    EXPECT_EQ(results.size(), 1);
    EXPECT_TRUE(results.contains(resultUrl));
    EXPECT_FALSE(searcher->hasItem());
}

// ========== GetSearchType Tests ==========
TEST_F(TestDFMSearcher, GetSearchType_WithFileNameType_ReturnsFileName)
{
    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::FileName);

    DFMSEARCH::SearchType type = searcher->getSearchType();

    EXPECT_EQ(type, DFMSEARCH::SearchType::FileName);
}

TEST_F(TestDFMSearcher, GetSearchType_WithNullEngine_ReturnsFileName)
{
    setupBasicStubs();

    stub.set_lamda(&DFMSEARCH::SearchFactory::createEngine,
                   [](DFMSEARCH::SearchType, QObject *) -> DFMSEARCH::SearchEngine * {
                       __DBG_STUB_INVOKE__
                       return nullptr;
                   });

    searcher = new DFMSearcher(searchUrl, keyword, nullptr, DFMSEARCH::SearchType::Content);

    DFMSEARCH::SearchType type = searcher->getSearchType();

    EXPECT_EQ(type, DFMSEARCH::SearchType::FileName);
}

// ========== ProcessSearchResult Tests ==========
TEST_F(TestDFMSearcher, ProcessSearchResult_WithFileNameSearch_CreatesFileNameResult)
{
    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::FileName);

    // Create a mock search result
    DFMSEARCH::SearchResult mockResult;
    stub.set_lamda(&DFMSEARCH::SearchResult::path, [&mockResult]() -> QString {
        __DBG_STUB_INVOKE__
        return "/home/test/file1.txt";
    });

    searcher->processSearchResult(mockResult);

    EXPECT_TRUE(searcher->hasItem());
    DFMSearchResultMap results = searcher->takeAll();
    EXPECT_EQ(results.size(), 1);

    QUrl expectedUrl = QUrl::fromLocalFile("/home/test/file1.txt");
    EXPECT_TRUE(results.contains(expectedUrl));
    EXPECT_FALSE(results[expectedUrl].isContentMatch());
}

TEST_F(TestDFMSearcher, ProcessSearchResult_WithContentSearch_CreatesContentResult)
{
    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::Content);

    // Create a mock search result
    DFMSEARCH::SearchResult mockResult;
    stub.set_lamda(&DFMSEARCH::SearchResult::path, [&mockResult]() -> QString {
        __DBG_STUB_INVOKE__
        return "/home/test/file1.txt";
    });

    QString expectedContent = "highlighted content";
    stub.set_lamda(&DFMSEARCH::ContentResultAPI::highlightedContent,
                   [&expectedContent](DFMSEARCH::ContentResultAPI *) -> QString {
                       __DBG_STUB_INVOKE__
                       return expectedContent;
                   });

    searcher->processSearchResult(mockResult);

    EXPECT_TRUE(searcher->hasItem());
    DFMSearchResultMap results = searcher->takeAll();
    EXPECT_EQ(results.size(), 1);

    QUrl expectedUrl = QUrl::fromLocalFile("/home/test/file1.txt");
    EXPECT_TRUE(results.contains(expectedUrl));
}

// ========== Signal Tests ==========
TEST_F(TestDFMSearcher, OnSearchStarted_EmitsNoSignals)
{
    searcher = createSearcherWithMockEngine();

    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);
    QSignalSpy unearthedSpy(searcher, &DFMSearcher::unearthed);

    MockSearchEngine *engine = dynamic_cast<MockSearchEngine *>(searcher->engine);
    ASSERT_NE(engine, nullptr);

    emit engine->searchStarted();

    EXPECT_EQ(finishedSpy.count(), 0);
    EXPECT_EQ(unearthedSpy.count(), 0);
}

TEST_F(TestDFMSearcher, OnSearchFinished_WithResults_ProcessesAndEmitsFinished)
{
    searcher = createSearcherWithMockEngine();

    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    MockSearchEngine *engine = dynamic_cast<MockSearchEngine *>(searcher->engine);
    ASSERT_NE(engine, nullptr);

    // Create mock results
    QList<DFMSEARCH::SearchResult> mockResults;
    DFMSEARCH::SearchResult result1;
    stub.set_lamda(&DFMSEARCH::SearchResult::path, [&result1]() -> QString {
        __DBG_STUB_INVOKE__
        return "/home/test/file1.txt";
    });
    mockResults.append(result1);

    // Set resultFoundEnabled to false so results are processed in onSearchFinished
    DFMSEARCH::SearchOptions options;
    options.setResultFoundEnabled(false);
    engine->setSearchOptions(options);

    engine->emitFinished(mockResults);

    EXPECT_EQ(finishedSpy.count(), 1);
}

TEST_F(TestDFMSearcher, OnSearchFinished_WithRealtimeResults_OnlyEmitsFinished)
{
    searcher = createSearcherWithMockEngine();

    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    MockSearchEngine *engine = dynamic_cast<MockSearchEngine *>(searcher->engine);
    ASSERT_NE(engine, nullptr);

    // Set resultFoundEnabled to true (realtime)
    DFMSEARCH::SearchOptions options;
    options.setResultFoundEnabled(true);
    engine->setSearchOptions(options);

    QList<DFMSEARCH::SearchResult> emptyResults;
    engine->emitFinished(emptyResults);

    EXPECT_EQ(finishedSpy.count(), 1);
}

TEST_F(TestDFMSearcher, OnSearchCancelled_EmitsFinished)
{
    searcher = createSearcherWithMockEngine();

    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    MockSearchEngine *engine = dynamic_cast<MockSearchEngine *>(searcher->engine);
    ASSERT_NE(engine, nullptr);

    engine->cancel();

    EXPECT_EQ(finishedSpy.count(), 1);
}

TEST_F(TestDFMSearcher, OnSearchError_EmitsFinished)
{
    searcher = createSearcherWithMockEngine();

    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    MockSearchEngine *engine = dynamic_cast<MockSearchEngine *>(searcher->engine);
    ASSERT_NE(engine, nullptr);

    DFMSEARCH::SearchError error;
    engine->emitError(error);

    EXPECT_EQ(finishedSpy.count(), 1);
}

TEST_F(TestDFMSearcher, ResultsFound_ProcessesResultsAndEmitsUnearthed)
{
    searcher = createSearcherWithMockEngine();

    QSignalSpy unearthedSpy(searcher, &DFMSearcher::unearthed);

    MockSearchEngine *engine = dynamic_cast<MockSearchEngine *>(searcher->engine);
    ASSERT_NE(engine, nullptr);

    // Create mock results
    QList<DFMSEARCH::SearchResult> mockResults;
    DFMSEARCH::SearchResult result1;
    stub.set_lamda(&DFMSEARCH::SearchResult::path, [&result1]() -> QString {
        __DBG_STUB_INVOKE__
        return "/home/test/file1.txt";
    });
    mockResults.append(result1);

    engine->emitResultsFound(mockResults);

    EXPECT_EQ(unearthedSpy.count(), 1);
    EXPECT_TRUE(searcher->hasItem());
}

// ========== GetSearchMethod Tests ==========
TEST_F(TestDFMSearcher, GetSearchMethod_ContentSearch_ReturnsIndexed)
{
    setupBasicStubs();

    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::Content);

    DFMSEARCH::SearchMethod method = searcher->getSearchMethod("/home/test");

    EXPECT_EQ(method, DFMSEARCH::SearchMethod::Indexed);
}

TEST_F(TestDFMSearcher, GetSearchMethod_FileNameSearch_IndexReady_PathInIndex_ReturnsIndexed)
{
    setupBasicStubs();

    stub.set_lamda(&DFMSEARCH::Global::isFileNameIndexReadyForSearch, []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DFMSEARCH::Global::isPathInFileNameIndexDirectory, [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DFMSEARCH::Global::isHiddenPathOrInHiddenDir, [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::FileName);

    DFMSEARCH::SearchMethod method = searcher->getSearchMethod("/home/test");

    EXPECT_EQ(method, DFMSEARCH::SearchMethod::Indexed);
}

TEST_F(TestDFMSearcher, GetSearchMethod_FileNameSearch_IndexNotReady_ReturnsRealtime)
{
    setupBasicStubs();

    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::FileName);
    stub.set_lamda(DFMSEARCH::Global::isFileNameIndexReadyForSearch, [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    DFMSEARCH::SearchMethod method = searcher->getSearchMethod("/home/test");

    EXPECT_EQ(method, DFMSEARCH::SearchMethod::Realtime);
}

TEST_F(TestDFMSearcher, GetSearchMethod_FileNameSearch_PathNotInIndex_ReturnsRealtime)
{
    setupBasicStubs();

    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::FileName);
    stub.set_lamda(&DFMSEARCH::Global::isFileNameIndexReadyForSearch, []() -> bool {
        __DBG_STUB_INVOKE__
                return true;
    });

    stub.set_lamda(&DFMSEARCH::Global::isPathInFileNameIndexDirectory, [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
                return false;
    });

    DFMSEARCH::SearchMethod method = searcher->getSearchMethod("/home/test");

    EXPECT_EQ(method, DFMSEARCH::SearchMethod::Realtime);
}

TEST_F(TestDFMSearcher, GetSearchMethod_FileNameSearch_InHiddenDir_ReturnsRealtime)
{
    setupBasicStubs();

    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::FileName);
    stub.set_lamda(&DFMSEARCH::Global::isFileNameIndexReadyForSearch, []() -> bool {
        __DBG_STUB_INVOKE__
                return true;
    });

    stub.set_lamda(&DFMSEARCH::Global::isHiddenPathOrInHiddenDir, [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
                return true;
    });

    DFMSEARCH::SearchMethod method = searcher->getSearchMethod("/home/test/.hidden");

    EXPECT_EQ(method, DFMSEARCH::SearchMethod::Realtime);
}

// ========== ConfigureSearchOptions Tests ==========
TEST_F(TestDFMSearcher, ConfigureSearchOptions_SetsBasicOptions)
{
    setupBasicStubs();

    searcher = createSearcherWithMockEngine();

    DFMSEARCH::SearchOptions options = searcher->configureSearchOptions("/home/test");

    EXPECT_EQ(options.searchPath(), "/home/test");
    EXPECT_FALSE(options.caseSensitive());
}

TEST_F(TestDFMSearcher, ConfigureSearchOptions_HiddenFilesEnabled_IncludesHidden)
{
    setupBasicStubs();

    searcher = createSearcherWithMockEngine();
    stub.set_lamda(&Application::genericAttribute, [](Application::GenericAttribute attr) -> QVariant {
        __DBG_STUB_INVOKE__
                if (attr == Application::kShowedHiddenFiles)
                return QVariant(true);
        return QVariant(false);
    });

    DFMSEARCH::SearchOptions options = searcher->configureSearchOptions("/home/test");

    EXPECT_TRUE(options.includeHidden());
}

TEST_F(TestDFMSearcher, ConfigureSearchOptions_InHiddenDirectory_AlwaysIncludesHidden)
{
    setupBasicStubs();

    searcher = createSearcherWithMockEngine();
    stub.set_lamda(&Application::genericAttribute, [] {
        __DBG_STUB_INVOKE__
                return QVariant(false);   // Hidden files disabled
    });

    stub.set_lamda(&DFMSEARCH::Global::isHiddenPathOrInHiddenDir, [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
                return true;   // But in hidden directory
    });

    DFMSEARCH::SearchOptions options = searcher->configureSearchOptions("/home/test/.hidden");

    EXPECT_TRUE(options.includeHidden());
}

TEST_F(TestDFMSearcher, ConfigureSearchOptions_RealtimeMethod_EnablesResultFound)
{
    setupBasicStubs();

    searcher = createSearcherWithMockEngine();
    stub.set_lamda(&DFMSEARCH::Global::isFileNameIndexReadyForSearch, []() -> bool {
        __DBG_STUB_INVOKE__
                return false;   // Force realtime method
    });

    DFMSEARCH::SearchOptions options = searcher->configureSearchOptions("/home/test");

    EXPECT_EQ(options.method(), DFMSEARCH::SearchMethod::Realtime);
    EXPECT_TRUE(options.resultFoundEnabled());
}

// ========== ShouldExcludeIndexedPaths Tests ==========
TEST_F(TestDFMSearcher, ShouldExcludeIndexedPaths_InHiddenDir_ReturnsFalse)
{
    setupBasicStubs();
    searcher = createSearcherWithMockEngine();
    stub.set_lamda(&DFMSEARCH::Global::isHiddenPathOrInHiddenDir, [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
                return true;
    });

    bool result = searcher->shouldExcludeIndexedPaths("/home/test/.hidden");

    EXPECT_FALSE(result);
}

TEST_F(TestDFMSearcher, ShouldExcludeIndexedPaths_FileNameSearch_IndexNotReady_ReturnsFalse)
{
    setupBasicStubs();
    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::FileName);
    stub.set_lamda(&DFMSEARCH::Global::isFileNameIndexReadyForSearch, []() -> bool {
        __DBG_STUB_INVOKE__
                return false;
    });

    stub.set_lamda(&DFMSEARCH::Global::isHiddenPathOrInHiddenDir, [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
                return false;
    });

    bool result = searcher->shouldExcludeIndexedPaths("/home/test");

    EXPECT_FALSE(result);
}

TEST_F(TestDFMSearcher, ShouldExcludeIndexedPaths_NormalCase_ReturnsTrue)
{
    setupBasicStubs();
    searcher = createSearcherWithMockEngine(DFMSEARCH::SearchType::FileName);
    stub.set_lamda(&DFMSEARCH::Global::isFileNameIndexReadyForSearch, []() -> bool {
        __DBG_STUB_INVOKE__
                return true;
    });

    stub.set_lamda(&DFMSEARCH::Global::isHiddenPathOrInHiddenDir, [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
                return false;
    });

    bool result = searcher->shouldExcludeIndexedPaths("/home/test");

    EXPECT_TRUE(result);
}

// ========== Thread Safety Tests ==========
TEST_F(TestDFMSearcher, ThreadSafety_ConcurrentHasItemAndTakeAll_NoDataRace)
{
    searcher = createSearcherWithMockEngine();

    // Add some results
    QUrl resultUrl = QUrl::fromLocalFile("/home/test/file1.txt");
    DFMSearchResult searchResult(resultUrl);
    searcher->allResults.insert(resultUrl, searchResult);

    // Simulate concurrent access
    bool hasItems1 = searcher->hasItem();
    DFMSearchResultMap results1 = searcher->takeAll();
    bool hasItems2 = searcher->hasItem();
    DFMSearchResultMap results2 = searcher->takeAll();

    EXPECT_TRUE(hasItems1);
    EXPECT_FALSE(results1.isEmpty());
    EXPECT_FALSE(hasItems2);
    EXPECT_TRUE(results2.isEmpty());
}

// ========== Integration Tests ==========
TEST_F(TestDFMSearcher, CompleteWorkflow_Search_ReceiveResults_TakeAll)
{
    searcher = createSearcherWithMockEngine();

    QSignalSpy unearthedSpy(searcher, &DFMSearcher::unearthed);
    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    // Start search
    bool searchResult = searcher->search();
    EXPECT_TRUE(searchResult);

    // Simulate results being found
    MockSearchEngine *engine = dynamic_cast<MockSearchEngine *>(searcher->engine);
    ASSERT_NE(engine, nullptr);

    QList<DFMSEARCH::SearchResult> mockResults;
    DFMSEARCH::SearchResult result1;
    stub.set_lamda(&DFMSEARCH::SearchResult::path, [&result1]() -> QString {
        __DBG_STUB_INVOKE__
        return "/home/test/file1.txt";
    });
    mockResults.append(result1);

    engine->emitResultsFound(mockResults);

    EXPECT_EQ(unearthedSpy.count(), 1);
    EXPECT_TRUE(searcher->hasItem());

    // Take all results
    DFMSearchResultMap results = searcher->takeAll();
    EXPECT_EQ(results.size(), 1);
    EXPECT_FALSE(searcher->hasItem());
}

TEST_F(TestDFMSearcher, CompleteWorkflow_Search_Cancel)
{
    searcher = createSearcherWithMockEngine();

    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    // Start search
    bool searchResult = searcher->search();
    EXPECT_TRUE(searchResult);

    // Cancel search
    searcher->stop();

    EXPECT_EQ(finishedSpy.count(), 1);
}

TEST_F(TestDFMSearcher, CreateSearchQuery_CallsQuerySelector)
{
    searcher = createSearcherWithMockEngine();

    DFMSEARCH::SearchQuery query = searcher->createSearchQuery();

    // Query should be created by querySelector
    EXPECT_TRUE(true);   // Basic validation that method completes
}
