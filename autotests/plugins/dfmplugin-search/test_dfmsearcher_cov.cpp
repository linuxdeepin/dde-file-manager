// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map for src/plugins/filemanager/dfmplugin-search/searchmanager/searcher/dfmsearch/dfmsearcher.cpp
// (functions from the uncovered list -> test case that calls them)
//   DFMSearcher::DFMSearcher(QUrl,QString,QObject*,SearchType) .......... Constructor_EngineCreated_*,
//                                                                           Constructor_NullEngine_*
//   DFMSearcher::{lambda(QList<SearchResult>)#1} (resultsFound) ......... Constructor_ResultsFoundSignal_ProcessesResultsAndEmitsUnearthed
//   DFMSearcher::~DFMSearcher .......................................... Constructor_NullEngine_* (stack + fixture deletion paths)
//   DFMSearcher::createSearchQuery() const ............................. Search_ReadyEngine_* (via executeSearch)
//   DFMSearcher::search() .............................................. Search_* / GetSearchMethod_* cases
//   DFMSearcher::stop() ................................................ Stop_WhileSearching_CancelsEngine / Stop_WhenReady_DoesNotCancel
//   DFMSearcher::hasItem() const ....................................... HasItem_* / TakeAll_*
//   DFMSearcher::takeAll() ............................................. TakeAll_WithResults_ReturnsAndClears
//   DFMSearcher::getSearchType() const ................................. GetSearchType_* (used by many cases)
//   DFMSearcher::isIndexOnlyContentSearch() const ...................... GetSearchType_* / ProcessSearchResult_*
//   DFMSearcher::searchTypeDisplayName() const ........................ SearchTypeDisplayName_AllEngineTypes_Mapped
//   DFMSearcher::processSearchResult(SearchResult const&) ............. ProcessSearchResult_* / handleRemainingResults cases
//   DFMSearcher::isEngineReady() const ................................. Search_EngineNotReady_ReturnsFalse
//   DFMSearcher::isValidSearchParameters() const ....................... Search_EmptyKeyword_ReturnsFalse
//   DFMSearcher::validateSearchType(QString,SearchOptions&) ........... Search_ContentType*_*/Search_OcrType_*
//   DFMSearcher::executeSearch() ....................................... Search_ReadyEngine_ValidParams_ReturnsTrue
//   DFMSearcher::configureSearchOptions(QString) const ................. Search_* (all method-selection cases)
//   DFMSearcher::configureHiddenFilesOption(SearchOptions&,QString) .... Search_* hidden/normal cases
//   DFMSearcher::configureRealtimeSearchOptions(SearchOptions&,QString)  GetSearchMethod_* realtime cases
//   DFMSearcher::shouldExcludeIndexedPaths(QString) const .............. GetSearchMethod_* (hidden/unavailable/external/default)
//   DFMSearcher::setExcludedPathsForRealtime(SearchOptions&) const ..... GetSearchMethod_PathOutsideIndexDir_* / ExcludesEmpty_*
//   DFMSearcher::setExcludedPathsForRealtime::{lambda#1..#3} ........... same two cases (bindPathTransform/remove_if/copy_if lambdas)
//   DFMSearcher::handleRemainingResults(QList<SearchResult> const&) .... Constructor_ResultsFoundSignal_* / SearchFinished_*
//   DFMSearcher::getSearchMethod(QString) const ........................ GetSearchMethod_* (branch list below)
//   DFMSearcher::onSearchStarted() ..................................... Search_ReadyEngine_* (stubbed search emits searchStarted)
//   DFMSearcher::onSearchFinished(QList<SearchResult> const&) .......... SearchFinished_WithResults_ProcessesAndFinishes
//   DFMSearcher::onSearchCancelled() ................................... Stop_WhileSearching_CancelsEngine / SearchCancelled_*
//   DFMSearcher::onSearchError(SearchError const&) ..................... SearchError_EmitsFinished
//   (bonus) AbstractSearcher::takeAllUrls() ............................ TakeAllUrls_WithResults_ReturnsUrls
//
// getSearchMethod branch checklist covered:
//   [1] non-FileName type -> Indexed          (GetSearchMethod_NonFileNameEngine_*)
//   [2] FileName + index not ready -> Realtime (GetSearchMethod_IndexNotReady_*)
//   [3] FileName + ready + not in index dir -> Realtime (GetSearchMethod_PathOutsideIndexDir_*)
//   [4] FileName + ready + in dir + hidden dir -> Realtime (GetSearchMethod_HiddenDir_*)
//   [5] FileName + ready + in dir + external mount -> Realtime (GetSearchMethod_ExternalMount_*)
//   [6] FileName + ready + symlink resolves outside index -> Realtime (GetSearchMethod_SymlinkOutsideIndex_*)
//   [7] fallback -> Indexed (canonical path empty) (GetSearchMethod_InIndexDirNoSymlink_*)
// DFMSEARCH::SearchEngine methods are non-virtual, so the engine is isolated by
// stubbing SearchEngine::search/status/searchType/searchOptions/setSearchOptions/cancel.
// ============================================================================

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>
#include <QVariant>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QList>
#include <memory>

#include "stubext.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <dfm-search/searchfactory.h>
#include <dfm-search/searchengine.h>
#include <dfm-search/searchquery.h>
#include <dfm-search/searchoptions.h>
#include <dfm-search/dsearch_global.h>

#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"
#include "searchmanager/searcher/abstractsearcher.h"
#include "searchmanager/searcher/searchresult_define.h"

using namespace dfmplugin_search;

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE
DFM_SEARCH_USE_NS

class UT_DFMSearcherCov : public testing::Test
{
protected:
    void SetUp() override
    {
        searchUrl = QUrl::fromLocalFile("/home/test");
        keyword = "test_keyword";
        engineType = DFMSEARCH::SearchType::FileName;
        engineStatus = DFMSEARCH::SearchStatus::Ready;
    }

    void TearDown() override
    {
        // Remove global patches before destroying the engine objects.
        stub.clear();
        qDeleteAll(engines);
        engines.clear();
    }

    void setupBasicStubs(bool indexReady = true,
                         bool pathInIndexDir = true,
                         bool pathHidden = false,
                         bool externalMount = false)
    {
        stub.set_lamda(&UrlRoute::urlToPath, [](const QUrl &url) -> QString {
            return url.toLocalFile();
        });

        stub.set_lamda(static_cast<QString (*)(const QString &, bool)>(&FileUtils::bindPathTransform),
                       [](const QString &path, bool) -> QString {
                           return path;
                       });

        stub.set_lamda(static_cast<QVariant (*)(Application::GenericAttribute)>(&Application::genericAttribute),
                       [](Application::GenericAttribute) -> QVariant {
                           return QVariant(false);
                       });

        stub.set_lamda(&DFMSEARCH::Global::isFileNameIndexReadyForSearch, [indexReady]() -> bool {
            return indexReady;
        });

        stub.set_lamda(&DFMSEARCH::Global::isPathInFileNameIndexDirectory, [pathInIndexDir](const QString &) -> bool {
            return pathInIndexDir;
        });

        stub.set_lamda(&DFMSEARCH::Global::isHiddenPathOrInHiddenDir, [pathHidden](const QString &) -> bool {
            return pathHidden;
        });

        stub.set_lamda(&DFMSEARCH::Global::isOcrTextIndexAvailable, []() -> bool {
            return true;
        });

        stub.set_lamda(&DFMSEARCH::Global::isPathInOcrTextIndexDirectory, [](const QString &) -> bool {
            return true;
        });

        stub.set_lamda(&DFMSEARCH::Global::defaultIndexedDirectory, []() -> QStringList {
            return QStringList() << "/home/test/indexed";
        });

        stub.set_lamda(static_cast<bool (DeviceProxyManager::*)(const QString &)>(&DeviceProxyManager::isFileOfExternalMounts),
                       [externalMount](DeviceProxyManager *, const QString &) -> bool {
                           return externalMount;
                       });
    }

    // Patch the (non-virtual) SearchEngine facade so no real index is touched.
    void stubEngineMethods()
    {
        stub.set_lamda(static_cast<DFMSEARCH::SearchType (DFMSEARCH::SearchEngine::*)() const>(&DFMSEARCH::SearchEngine::searchType),
                       [this](DFMSEARCH::SearchEngine *) -> DFMSEARCH::SearchType {
                           return engineType;
                       });

        stub.set_lamda(static_cast<DFMSEARCH::SearchStatus (DFMSEARCH::SearchEngine::*)() const>(&DFMSEARCH::SearchEngine::status),
                       [this](DFMSEARCH::SearchEngine *) -> DFMSEARCH::SearchStatus {
                           return engineStatus;
                       });

        stub.set_lamda(static_cast<DFMSEARCH::SearchOptions (DFMSEARCH::SearchEngine::*)() const>(&DFMSEARCH::SearchEngine::searchOptions),
                       [this](DFMSEARCH::SearchEngine *) -> DFMSEARCH::SearchOptions {
                           return capturedOptions;
                       });

        stub.set_lamda(static_cast<void (DFMSEARCH::SearchEngine::*)(const DFMSEARCH::SearchOptions &)>(&DFMSEARCH::SearchEngine::setSearchOptions),
                       [this](DFMSEARCH::SearchEngine *, const DFMSEARCH::SearchOptions &options) {
                           capturedOptions = options;
                           ++setOptionsCalls;
                       });

        stub.set_lamda(static_cast<void (DFMSEARCH::SearchEngine::*)(const DFMSEARCH::SearchQuery &)>(&DFMSEARCH::SearchEngine::search),
                       [this](DFMSEARCH::SearchEngine *self, const DFMSEARCH::SearchQuery &query) {
                           lastQuery = query;
                           ++searchCalls;
                           engineStatus = DFMSEARCH::SearchStatus::Searching;
                           emit self->searchStarted();
                       });

        stub.set_lamda(static_cast<void (DFMSEARCH::SearchEngine::*)()>(&DFMSEARCH::SearchEngine::cancel),
                       [this](DFMSEARCH::SearchEngine *self) {
                           ++cancelCalls;
                           engineStatus = DFMSEARCH::SearchStatus::Cancelled;
                           emit self->searchCancelled();
                       });
    }

    DFMSearcher *createSearcher(DFMSEARCH::SearchType type = DFMSEARCH::SearchType::FileName,
                                bool indexReady = true,
                                bool pathInIndexDir = true,
                                bool pathHidden = false,
                                bool externalMount = false)
    {
        setupBasicStubs(indexReady, pathInIndexDir, pathHidden, externalMount);
        engineType = type;
        stubEngineMethods();

        auto *engine = new DFMSEARCH::SearchEngine();
        engines.append(engine);
        stub.set_lamda(&DFMSEARCH::SearchFactory::createEngine,
                       [engine](DFMSEARCH::SearchType, QObject *) -> DFMSEARCH::SearchEngine * {
                           return engine;
                       });
        return new DFMSearcher(searchUrl, keyword, nullptr, type);
    }

    stub_ext::StubExt stub;
    QList<DFMSEARCH::SearchEngine *> engines;
    QUrl searchUrl;
    QString keyword;

    // engine facade state
    DFMSEARCH::SearchType engineType = DFMSEARCH::SearchType::FileName;
    DFMSEARCH::SearchStatus engineStatus = DFMSEARCH::SearchStatus::Ready;
    DFMSEARCH::SearchOptions capturedOptions;
    DFMSEARCH::SearchQuery lastQuery;
    int setOptionsCalls = 0;
    int searchCalls = 0;
    int cancelCalls = 0;
};

// ---------- Constructor / destructor ----------

TEST_F(UT_DFMSearcherCov, Constructor_EngineCreated_StoresContextAndEngine)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();

    // Act
    DFMSEARCH::SearchEngine *engine = searcher->engine;

    // Assert
    ASSERT_NE(engine, nullptr);
    EXPECT_EQ(searcher->searchUrl, searchUrl);
    EXPECT_EQ(searcher->keyword, keyword);
    EXPECT_EQ(searcher->parent(), nullptr);
}

TEST_F(UT_DFMSearcherCov, Constructor_NullEngine_FactoryFailureHandled)
{
    // Arrange
    setupBasicStubs();
    stubEngineMethods();
    stub.set_lamda(&DFMSEARCH::SearchFactory::createEngine,
                   [](DFMSEARCH::SearchType, QObject *) -> DFMSEARCH::SearchEngine * {
                       return nullptr;
                   });

    // Act
    DFMSearcher searcher(searchUrl, keyword, nullptr, DFMSEARCH::SearchType::Content);

    // Assert
    EXPECT_EQ(searcher.engine, nullptr);
    EXPECT_EQ(searcher.getSearchType(), DFMSEARCH::SearchType::FileName);   // null-engine fallback
}

TEST_F(UT_DFMSearcherCov, Constructor_ResultsFoundSignal_ProcessesResultsAndEmitsUnearthed)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();
    auto *engine = searcher->engine;
    ASSERT_NE(engine, nullptr);
    QSignalSpy unearthedSpy(searcher, &DFMSearcher::unearthed);

    // Act
    emit engine->resultsFound({ DFMSEARCH::SearchResult("/home/test/file1.txt") });

    // Assert
    EXPECT_EQ(unearthedSpy.count(), 1);
    EXPECT_TRUE(searcher->hasItem());
    DFMSearchResultMap results = searcher->takeAll();
    EXPECT_EQ(results.size(), 1);
    EXPECT_TRUE(results.contains(QUrl::fromLocalFile("/home/test/file1.txt")));
}

// ---------- search() ----------

TEST_F(UT_DFMSearcherCov, Search_ReadyEngine_ValidParams_ReturnsTrueAndStartsSearch)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();
    auto *engine = searcher->engine;
    QSignalSpy startedSpy(engine, &DFMSEARCH::SearchEngine::searchStarted);

    // Act
    bool result = searcher->search();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_GE(startedSpy.count(), 1);   // onSearchStarted executed through the signal
    EXPECT_EQ(searchCalls, 1);
    EXPECT_EQ(capturedOptions.searchPath(), "/home/test");
    EXPECT_FALSE(capturedOptions.caseSensitive());
}

TEST_F(UT_DFMSearcherCov, Search_EngineNotReady_ReturnsFalse)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();
    engineStatus = DFMSEARCH::SearchStatus::Finished;

    // Act
    bool result = searcher->search();

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(searchCalls, 0);
}

TEST_F(UT_DFMSearcherCov, Search_EmptyKeyword_ReturnsFalse)
{
    // Arrange
    setupBasicStubs();
    stubEngineMethods();
    auto *engine = new DFMSEARCH::SearchEngine();
    engines.append(engine);
    stub.set_lamda(&DFMSEARCH::SearchFactory::createEngine,
                   [engine](DFMSEARCH::SearchType, QObject *) -> DFMSEARCH::SearchEngine * {
                       return engine;
                   });
    DFMSearcher searcher(searchUrl, QString(), nullptr, DFMSEARCH::SearchType::FileName);

    // Act
    bool result = searcher.search();

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(searchCalls, 0);
}

TEST_F(UT_DFMSearcherCov, Search_ContentTypeNotIndexed_EmitsFinishedEarly)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::Content, true, false);
    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    // Act
    bool result = searcher->search();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(searchCalls, 0);   // never reached the engine
}

TEST_F(UT_DFMSearcherCov, Search_ContentTypeIndexed_ExecutesContentSearch)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::Content, true, true);

    // Act
    bool result = searcher->search();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(searchCalls, 1);
    EXPECT_EQ(engineStatus, DFMSEARCH::SearchStatus::Searching);
}

TEST_F(UT_DFMSearcherCov, Search_OcrTypeUnavailable_EmitsFinishedEarly)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::Ocr);
    stub.set_lamda(&DFMSEARCH::Global::isOcrTextIndexAvailable, []() -> bool {
        return false;
    });
    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    // Act
    bool result = searcher->search();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(searchCalls, 0);
}

TEST_F(UT_DFMSearcherCov, Search_OcrTypeNotInIndexDir_EmitsFinishedEarly)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::Ocr);
    stub.set_lamda(&DFMSEARCH::Global::isPathInOcrTextIndexDirectory, [](const QString &) -> bool {
        return false;
    });
    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    // Act
    bool result = searcher->search();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(searchCalls, 0);
}

TEST_F(UT_DFMSearcherCov, Search_OcrTypeIndexed_ExecutesOcrSearch)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::Ocr, true, true, false, false);

    // Act
    bool result = searcher->search();

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(searchCalls, 1);
}

// ---------- stop() ----------

TEST_F(UT_DFMSearcherCov, Stop_WhileSearching_CancelsEngine)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();
    auto *engine = searcher->engine;
    searcher->search();   // engine now Searching
    QSignalSpy cancelledSpy(engine, &DFMSEARCH::SearchEngine::searchCancelled);
    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    // Act
    searcher->stop();

    // Assert
    EXPECT_EQ(cancelledSpy.count(), 1);
    EXPECT_EQ(cancelCalls, 1);
    EXPECT_EQ(finishedSpy.count(), 1);   // onSearchCancelled fired
}

TEST_F(UT_DFMSearcherCov, Stop_WhenReady_DoesNotCancel)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();
    auto *engine = searcher->engine;
    QSignalSpy cancelledSpy(engine, &DFMSEARCH::SearchEngine::searchCancelled);

    // Act
    searcher->stop();

    // Assert
    EXPECT_EQ(cancelledSpy.count(), 0);
    EXPECT_EQ(cancelCalls, 0);
}

// ---------- results access ----------

TEST_F(UT_DFMSearcherCov, TakeAll_WithResults_ReturnsAndClears)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();
    emit searcher->engine->resultsFound({ DFMSEARCH::SearchResult("/home/test/a.txt"),
                                          DFMSEARCH::SearchResult("/home/test/b.txt") });

    // Act
    DFMSearchResultMap results = searcher->takeAll();

    // Assert
    EXPECT_EQ(results.size(), 2);
    EXPECT_FALSE(searcher->hasItem());   // drained
}

TEST_F(UT_DFMSearcherCov, HasItem_Empty_FalseThenTrueAfterResult)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();
    EXPECT_FALSE(searcher->hasItem());

    // Act
    emit searcher->engine->resultsFound({ DFMSEARCH::SearchResult("/home/test/c.txt") });

    // Assert
    EXPECT_TRUE(searcher->hasItem());
    EXPECT_EQ(searcher->takeAll().size(), 1);
}

TEST_F(UT_DFMSearcherCov, TakeAllUrls_WithResults_ReturnsUrlList)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();
    emit searcher->engine->resultsFound({ DFMSEARCH::SearchResult("/home/test/d.txt") });

    // Act
    QList<QUrl> urls = searcher->takeAllUrls();

    // Assert
    EXPECT_EQ(urls.size(), 1);
    EXPECT_TRUE(urls.contains(QUrl::fromLocalFile("/home/test/d.txt")));
}

// ---------- type helpers ----------

TEST_F(UT_DFMSearcherCov, GetSearchType_ReflectsEngineType)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::Content);

    // Act
    auto type = searcher->getSearchType();

    // Assert
    EXPECT_EQ(type, DFMSEARCH::SearchType::Content);
    EXPECT_TRUE(searcher->isIndexOnlyContentSearch());
}

TEST_F(UT_DFMSearcherCov, IsIndexOnlyContentSearch_FileNameType_False)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::FileName);

    // Act
    bool contentOnly = searcher->isIndexOnlyContentSearch();

    // Assert
    EXPECT_FALSE(contentOnly);
    EXPECT_EQ(searcher->getSearchType(), DFMSEARCH::SearchType::FileName);
}

TEST_F(UT_DFMSearcherCov, IsIndexOnlyContentSearch_OcrType_True)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::Ocr);

    // Act
    bool contentOnly = searcher->isIndexOnlyContentSearch();

    // Assert
    EXPECT_TRUE(contentOnly);
    EXPECT_EQ(searcher->getSearchType(), DFMSEARCH::SearchType::Ocr);
}

TEST_F(UT_DFMSearcherCov, SearchTypeDisplayName_AllEngineTypes_Mapped)
{
    // Arrange
    std::unique_ptr<DFMSearcher> nameSearcher(createSearcher(DFMSEARCH::SearchType::FileName));

    // Act
    const QString nameText = nameSearcher->searchTypeDisplayName();

    // Assert
    EXPECT_EQ(nameText, QStringLiteral("File name"));
    EXPECT_EQ(nameSearcher->searchTypeDisplayName(), QStringLiteral("File name"));

    std::unique_ptr<DFMSearcher> contentSearcher(createSearcher(DFMSEARCH::SearchType::Content));
    EXPECT_EQ(contentSearcher->searchTypeDisplayName(), QStringLiteral("Content"));

    std::unique_ptr<DFMSearcher> ocrSearcher(createSearcher(DFMSEARCH::SearchType::Ocr));
    EXPECT_EQ(ocrSearcher->searchTypeDisplayName(), QStringLiteral("OCR"));

    std::unique_ptr<DFMSearcher> unknownSearcher(createSearcher(DFMSEARCH::SearchType::Semantic));
    EXPECT_EQ(unknownSearcher->searchTypeDisplayName(), QStringLiteral("Unknown"));
}

// ---------- processSearchResult ----------

TEST_F(UT_DFMSearcherCov, ProcessSearchResult_FileNameType_LowMatchScore)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::FileName);

    // Act
    searcher->processSearchResult(DFMSEARCH::SearchResult("/home/test/e.txt"));

    // Assert
    DFMSearchResultMap results = searcher->takeAll();
    ASSERT_EQ(results.size(), 1);
    const DFMSearchResult &item = results.begin().value();
    EXPECT_EQ(item.keyword(), keyword);
    EXPECT_DOUBLE_EQ(item.matchScore(), 0.5);
}

TEST_F(UT_DFMSearcherCov, ProcessSearchResult_ContentType_HighMatchScore)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::Content);

    // Act
    searcher->processSearchResult(DFMSEARCH::SearchResult("/home/test/f.txt"));

    // Assert
    DFMSearchResultMap results = searcher->takeAll();
    ASSERT_EQ(results.size(), 1);
    const DFMSearchResult &item = results.begin().value();
    EXPECT_EQ(item.searchType(), DFMSEARCH::SearchType::Content);
    EXPECT_DOUBLE_EQ(item.matchScore(), 1.0);
}

// ---------- engine signal slots ----------

TEST_F(UT_DFMSearcherCov, SearchFinished_WithResults_ProcessesAndFinishes)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();
    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    // Act
    emit searcher->engine->searchFinished({ DFMSEARCH::SearchResult("/home/test/g.txt") });

    // Assert
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(searcher->takeAll().size(), 1);   // resultFound disabled -> handleRemainingResults ran
}

TEST_F(UT_DFMSearcherCov, SearchCancelled_EmitsFinished)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();
    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    // Act
    emit searcher->engine->searchCancelled();

    // Assert
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(searcher->takeAll().size(), 0);
}

TEST_F(UT_DFMSearcherCov, SearchError_EmitsFinished)
{
    // Arrange
    DFMSearcher *searcher = createSearcher();
    QSignalSpy finishedSpy(searcher, &DFMSearcher::finished);

    // Act
    emit searcher->engine->errorOccurred(DFMSEARCH::SearchError());

    // Assert
    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(searcher->takeAll().size(), 0);
}

// ---------- getSearchMethod / configureSearchOptions branches ----------

TEST_F(UT_DFMSearcherCov, GetSearchMethod_NonFileNameEngine_UsesIndexed)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::Content, false, false);

    // Act
    bool ok = searcher->search();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(capturedOptions.method(), DFMSEARCH::SearchMethod::Indexed);
}

TEST_F(UT_DFMSearcherCov, GetSearchMethod_IndexNotReady_FallsBackRealtime)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::FileName, false, true);

    // Act
    bool ok = searcher->search();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(capturedOptions.method(), DFMSEARCH::SearchMethod::Realtime);
}

TEST_F(UT_DFMSearcherCov, GetSearchMethod_PathOutsideIndexDir_UsesRealtimeWithExcludes)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::FileName, true, false);

    // Act
    bool ok = searcher->search();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(capturedOptions.method(), DFMSEARCH::SearchMethod::Realtime);
    EXPECT_TRUE(capturedOptions.resultFoundEnabled());
    EXPECT_TRUE(capturedOptions.searchExcludedPaths().contains("/home/test/indexed"));   // lambdas #1..#3 executed
}

TEST_F(UT_DFMSearcherCov, GetSearchMethod_HiddenDir_UsesRealtimeWithoutExcludes)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::FileName, true, true, true);

    // Act
    bool ok = searcher->search();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(capturedOptions.method(), DFMSEARCH::SearchMethod::Realtime);
    EXPECT_TRUE(capturedOptions.includeHidden());   // hidden dir forces includeHidden
}

TEST_F(UT_DFMSearcherCov, GetSearchMethod_ExternalMount_UsesRealtime)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::FileName, true, true, false, true);

    // Act
    bool ok = searcher->search();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(capturedOptions.method(), DFMSEARCH::SearchMethod::Realtime);
}

TEST_F(UT_DFMSearcherCov, GetSearchMethod_SymlinkOutsideIndex_UsesRealtime)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::FileName, true, true);
    // Point the URL at an existing directory so canonicalFilePath() is non empty,
    // while the canonical path is (per stub) outside the index dir.
    stub.set_lamda(&UrlRoute::urlToPath, [&dir](const QUrl &) -> QString {
        return dir.path();
    });
    stub.set_lamda(&DFMSEARCH::Global::isPathInFileNameIndexDirectory, [](const QString &) -> bool {
        return false;
    });

    // Act
    bool ok = searcher->search();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(capturedOptions.method(), DFMSEARCH::SearchMethod::Realtime);
}

TEST_F(UT_DFMSearcherCov, GetSearchMethod_InIndexDirNoSymlink_UsesIndexed)
{
    // Arrange
    stub.set_lamda(&UrlRoute::urlToPath, [](const QUrl &) -> QString {
        return "/home/test/not-exist-path";   // canonicalFilePath() empty -> symlink check skipped
    });
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::FileName, true, true);

    // Act
    bool ok = searcher->search();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(capturedOptions.method(), DFMSEARCH::SearchMethod::Indexed);
}

TEST_F(UT_DFMSearcherCov, GetSearchMethod_ExcludesEmpty_WhenIndexedDirOutsideSearchPath)
{
    // Arrange
    DFMSearcher *searcher = createSearcher(DFMSEARCH::SearchType::FileName, true, false);
    // Re-stub AFTER createSearcher so it is not overwritten by setupBasicStubs().
    stub.set_lamda(&DFMSEARCH::Global::defaultIndexedDirectory, []() -> QStringList {
        return QStringList() << "/opt/other/index";   // not under /home/test/
    });

    // Act
    bool ok = searcher->search();

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(capturedOptions.method(), DFMSEARCH::SearchMethod::Realtime);
    EXPECT_TRUE(capturedOptions.searchExcludedPaths().isEmpty());
}
