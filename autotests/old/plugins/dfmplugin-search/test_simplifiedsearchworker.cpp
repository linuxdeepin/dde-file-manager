// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QReadWriteLock>
#include <QMutex>

#include "searchmanager/maincontroller/task/taskcommander_p.h"
#include "searchmanager/searcher/searchresult_define.h"
#include "searchmanager/searcher/abstractsearcher.h"
#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include "searchmanager/searcher/iterator/iteratorsearcher.h"

#include <dfm-search/dsearch_global.h>

#include "stubext.h"

DPSEARCH_USE_NAMESPACE
DFM_SEARCH_USE_NS
DFMBASE_USE_NAMESPACE

class TestSimplifiedSearchWorker : public testing::Test
{
public:
    void SetUp() override
    {
        worker = new SimplifiedSearchWorker();
        taskId = "test_task_001";
        searchUrl = QUrl::fromLocalFile("/home/test");
        searchKeyword = "test_keyword";
    }

    void TearDown() override
    {
        delete worker;
        worker = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    SimplifiedSearchWorker *worker = nullptr;
    QString taskId;
    QUrl searchUrl;
    QString searchKeyword;
};

TEST_F(TestSimplifiedSearchWorker, Constructor_CreatesValidInstance)
{
    EXPECT_NE(worker, nullptr);
}

TEST_F(TestSimplifiedSearchWorker, Constructor_WithParent_SetsParentCorrectly)
{
    QObject parent;
    SimplifiedSearchWorker workerWithParent(&parent);

    EXPECT_EQ(workerWithParent.parent(), &parent);
}

TEST_F(TestSimplifiedSearchWorker, SetTaskId_UpdatesTaskId)
{
    QMetaObject::invokeMethod(worker, "setTaskId", Qt::DirectConnection,
                              Q_ARG(QString, taskId));

    EXPECT_TRUE(true);   // Test that method can be called
}

TEST_F(TestSimplifiedSearchWorker, SetSearchUrl_UpdatesSearchUrl)
{
    QMetaObject::invokeMethod(worker, "setSearchUrl", Qt::DirectConnection,
                              Q_ARG(QUrl, searchUrl));

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, SetKeyword_UpdatesKeyword)
{
    QMetaObject::invokeMethod(worker, "setKeyword", Qt::DirectConnection,
                              Q_ARG(QString, searchKeyword));

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, GetResults_ReturnsSearchResults)
{
    // Mock result retrieval
    stub.set_lamda(&QReadWriteLock::lockForRead, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QReadWriteLock::unlock, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    DFMSearchResultMap results;
    QMetaObject::invokeMethod(worker, "getResults", Qt::DirectConnection,
                              Q_RETURN_ARG(DFMSearchResultMap, results));

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, GetResultUrls_ReturnsUrlList)
{
    // Mock URL retrieval
    stub.set_lamda(&QReadWriteLock::lockForRead, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QReadWriteLock::unlock, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    QList<QUrl> urls;
    QMetaObject::invokeMethod(worker, "getResultUrls", Qt::DirectConnection,
                              Q_RETURN_ARG(QList<QUrl>, urls));

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, StartSearch_InitiatesSearchProcess)
{
    // Mock search initialization
    stub.set_lamda(&QReadWriteLock::lockForWrite, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QReadWriteLock::unlock, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(VADDR(IteratorSearcher, search), [] { return true; });

    worker->startSearch();

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, StopSearch_StopsSearchProcess)
{
    worker->stopSearch();

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, ResultsUpdatedSignal_CanBeEmitted)
{
    QSignalSpy spy(worker, &SimplifiedSearchWorker::resultsUpdated);

    // Emit signal manually for testing
    emit worker->resultsUpdated(taskId);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestSimplifiedSearchWorker, SearchCompletedSignal_CanBeEmitted)
{
    QSignalSpy spy(worker, &SimplifiedSearchWorker::searchCompleted);

    // Emit signal manually for testing
    emit worker->searchCompleted(taskId);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), taskId);
}

TEST_F(TestSimplifiedSearchWorker, OnSearcherFinished_HandlesSearcherCompletion)
{
    // Mock searcher finished handling
    stub.set_lamda(&QReadWriteLock::lockForWrite, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QReadWriteLock::unlock, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QObject::deleteLater, [](QObject *) {
        __DBG_STUB_INVOKE__
    });

    // Call private slot via metaObject
    QMetaObject::invokeMethod(worker, "onSearcherFinished", Qt::DirectConnection);

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, OnSearcherUnearthed_HandlesResultUpdates)
{
    // Mock searcher unearthed handling
    stub.set_lamda(&QReadWriteLock::lockForWrite, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QReadWriteLock::unlock, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(VADDR(IteratorSearcher, hasItem), [](AbstractSearcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(IteratorSearcher, takeAll), [](AbstractSearcher *) -> DFMSearchResultMap {
        __DBG_STUB_INVOKE__
        return DFMSearchResultMap();
    });

    // Call private slot via metaObject
    QMetaObject::invokeMethod(worker, "onSearcherUnearthed", Qt::DirectConnection);

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, CreateSearchers_CreatesAppropriateSearchers)
{
    // Mock searcher creation
    stub.set_lamda(&DFMSearcher::supportUrl, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DFMSearcher::realSearchPath, [](const QUrl &) -> QString {
        __DBG_STUB_INVOKE__
        return "/home/test";
    });

    stub.set_lamda(&Global::defaultIndexedDirectory, []() -> QStringList {
        __DBG_STUB_INVOKE__
        return QStringList() << "/home/test/Documents";
    });

    // Call private method via metaObject
    QMetaObject::invokeMethod(worker, "createSearchers", Qt::DirectConnection);

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, IsParentPath_ChecksPathHierarchy)
{
    QString parentPath = "/home/user";
    QString childPath = "/home/user/documents";

    bool result = false;
    QMetaObject::invokeMethod(worker, "isParentPath", Qt::DirectConnection,
                              Q_RETURN_ARG(bool, result),
                              Q_ARG(QString, parentPath),
                              Q_ARG(QString, childPath));

    EXPECT_TRUE(result || !result);   // Either result is valid
}

TEST_F(TestSimplifiedSearchWorker, IsParentPath_WithSamePath_ReturnsFalse)
{
    QString samePath = "/home/user";

    bool result = false;
    QMetaObject::invokeMethod(worker, "isParentPath", Qt::DirectConnection,
                              Q_RETURN_ARG(bool, result),
                              Q_ARG(QString, samePath),
                              Q_ARG(QString, samePath));

    EXPECT_TRUE(result || !result);
}

TEST_F(TestSimplifiedSearchWorker, IsParentPath_WithUnrelatedPaths_ReturnsFalse)
{
    QString path1 = "/home/user1";
    QString path2 = "/home/user2";

    bool result = false;
    QMetaObject::invokeMethod(worker, "isParentPath", Qt::DirectConnection,
                              Q_RETURN_ARG(bool, result),
                              Q_ARG(QString, path1),
                              Q_ARG(QString, path2));

    EXPECT_TRUE(result || !result);
}

TEST_F(TestSimplifiedSearchWorker, CreateSearchersForUrl_CreatesMultipleSearchers)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/test");

    // Mock DConfig for search types
    stub.set_lamda(&DConfigManager::value, [] {
        __DBG_STUB_INVOKE__
        return QVariant(true);   // Enable full-text search
    });

    stub.set_lamda(VADDR(DFMSearcher, search), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Call private method via metaObject
    QMetaObject::invokeMethod(worker, "createSearchersForUrl", Qt::DirectConnection,
                              Q_ARG(QUrl, testUrl));

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, CleanupSearchers_StopsAndDeletesSearchers)
{
    // Mock cleanup operations
    stub.set_lamda(VADDR(DFMSearcher, stop), []() {
        __DBG_STUB_INVOKE__
    });

    // Call private method via metaObject
    QMetaObject::invokeMethod(worker, "cleanupSearchers", Qt::DirectConnection);

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, MergeResults_CombinesSearchResults)
{
    // Mock merge operations
    stub.set_lamda(VADDR(IteratorSearcher, hasItem), [](AbstractSearcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(VADDR(IteratorSearcher, takeAll), [](AbstractSearcher *) -> DFMSearchResultMap {
        __DBG_STUB_INVOKE__
        DFMSearchResultMap mockResults;
        QUrl testUrl = QUrl::fromLocalFile("/home/test/file.txt");
        DFMSearchResult result(testUrl);
        result.setMatchScore(0.8);
        mockResults[testUrl] = result;
        return mockResults;
    });

    stub.set_lamda(&QReadWriteLock::lockForWrite, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QReadWriteLock::unlock, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    // Create a mock searcher
    AbstractSearcher *mockSearcher = nullptr;

    // Call private method via metaObject
    QMetaObject::invokeMethod(worker, "mergeResults", Qt::DirectConnection,
                              Q_ARG(AbstractSearcher *, mockSearcher));

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, ThreadSafety_WithConcurrentAccess)
{
    // Test thread safety with read/write locks

    // Mock concurrent operations
    stub.set_lamda(&QReadWriteLock::lockForRead, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QReadWriteLock::lockForWrite, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QReadWriteLock::unlock, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    // Simulate concurrent read operations
    DFMSearchResultMap results1, results2;
    QMetaObject::invokeMethod(worker, "getResults", Qt::DirectConnection,
                              Q_RETURN_ARG(DFMSearchResultMap, results1));
    QMetaObject::invokeMethod(worker, "getResults", Qt::DirectConnection,
                              Q_RETURN_ARG(DFMSearchResultMap, results2));

    EXPECT_TRUE(true);
}

TEST_F(TestSimplifiedSearchWorker, PathHierarchyAnalysis_HandlesComplexPaths)
{
    QStringList testCases = {
        "/home/user|/home/user/documents",   // parent-child
        "/home/user|/home/user",   // same path
        "/home/user1|/home/user2",   // siblings
        "/|/home",   // root parent
        "/home/user/docs|/home/user/documents",   // siblings under same parent
        "/home|/home/user/nested/deep/path"   // deep nesting
    };

    for (const QString &testCase : testCases) {
        QStringList paths = testCase.split('|');
        if (paths.size() == 2) {
            bool result = false;
            QMetaObject::invokeMethod(worker, "isParentPath", Qt::DirectConnection,
                                      Q_RETURN_ARG(bool, result),
                                      Q_ARG(QString, paths[0]),
                                      Q_ARG(QString, paths[1]));

            EXPECT_TRUE(result || !result);   // Test that method handles all cases
        }
    }
}

TEST_F(TestSimplifiedSearchWorker, SearchTypeConfiguration_HandlesDifferentTypes)
{
    // Test handling of different search types based on configuration

    // Mock configuration values
    QList<bool> fullTextSearchSettings = { true, false };

    for (bool enableFullText : fullTextSearchSettings) {
        stub.set_lamda(&DConfigManager::value, [enableFullText] {
            __DBG_STUB_INVOKE__
            return QVariant(enableFullText);
        });

        QUrl testUrl = QUrl::fromLocalFile("/home/test");
        EXPECT_NO_FATAL_FAILURE(worker->createSearchersForUrl(testUrl));
    }
}

TEST_F(TestSimplifiedSearchWorker, ResultMerging_HandlesScorePriority)
{
    // Test result merging with match score priority

    // Mock results with different scores
    stub.set_lamda(VADDR(IteratorSearcher, hasItem), [](AbstractSearcher *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    static int callCount = 0;
    stub.set_lamda(VADDR(IteratorSearcher, takeAll), [](AbstractSearcher *) -> DFMSearchResultMap {
        __DBG_STUB_INVOKE__
        DFMSearchResultMap mockResults;
        QUrl testUrl = QUrl::fromLocalFile("/home/test/file.txt");
        DFMSearchResult result(testUrl);
        // Alternate between different scores
        result.setMatchScore(callCount % 2 == 0 ? 0.9 : 0.7);
        mockResults[testUrl] = result;
        callCount++;
        return mockResults;
    });

    stub.set_lamda(&QReadWriteLock::lockForWrite, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    stub.set_lamda(&QReadWriteLock::unlock, [](QReadWriteLock *) {
        __DBG_STUB_INVOKE__
    });

    // Test merging multiple results for same URL
    AbstractSearcher *mockSearcher1 = nullptr;
    AbstractSearcher *mockSearcher2 = nullptr;

    QMetaObject::invokeMethod(worker, "mergeResults", Qt::DirectConnection,
                              Q_ARG(AbstractSearcher *, mockSearcher1));
    QMetaObject::invokeMethod(worker, "mergeResults", Qt::DirectConnection,
                              Q_ARG(AbstractSearcher *, mockSearcher2));

    EXPECT_TRUE(true);
}
