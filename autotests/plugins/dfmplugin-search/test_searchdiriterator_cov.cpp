// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// Coverage map for src/plugins/filemanager/dfmplugin-search/iterator/searchdiriterator.cpp
// (uncovered functions -> test case)
//   SearchResultBuffer::updateResults(map) ................ Buffer_UpdateThenGet_SwitchesActiveBuffer
//   SearchResultBuffer::getResults() const ................. Buffer_UpdateThenGet_SwitchesActiveBuffer / Consume
//   SearchDirIteratorPrivate::onMatched(id) ................ OnMatched_MatchingTaskId_UpdatesBuffer / OnMatched_OtherId_Ignored
//   SearchDirIteratorPrivate::onSearchCompleted(id) ........ OnSearchCompleted_MatchingId_MarksFinished
//   SearchDirIteratorPrivate::onSearchStoped(id) ........... OnSearchStoped_MatchingId_EmitsStopAndFlags
//   SearchDirIteratorPrivate::doSearch(){lambda(QUrl)#1} ... covered indirectly through iterator construction (doSearch -> search)
//   SearchDirIterator::doCompleteSortInfo(SortInfoPointer) . DoCompleteSortInfo_CompletesGivenSortInfo
// Private members are reached through the private->public include switch
// (searchdiriterator_p.h declares the slots public anyway).
// ============================================================================

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>
#include <QVariant>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include "stubext.h"

#include <dfm-base/interfaces/sortfileinfo.h>

#include "utils/searchhelper.h"
#include "searchmanager/searchmanager.h"
#include "searchmanager/searcher/searchresult_define.h"

#include "iterator/searchdiriterator.h"
#include "iterator/searchdiriterator_p.h"

using namespace dfmplugin_search;
DFMBASE_USE_NAMESPACE

class UT_SearchDirIteratorCov : public testing::Test
{
protected:
    void SetUp() override
    {
        // Keep iterator construction away from the real search pipeline.
        stub.set_lamda(&SearchManager::search,
                       [](SearchManager *, quint64, const QString &, const QUrl &, const QString &) -> bool {
                           return true;
                       });

        searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home/cov"), "keyword", "42");
        iterator = new SearchDirIterator(searchUrl);
    }

    void TearDown() override
    {
        stub.clear();
        delete iterator;
        iterator = nullptr;
    }

    static DFMSearchResultMap makeMap(int count, const QString &prefix)
    {
        DFMSearchResultMap map;
        for (int i = 0; i < count; ++i)
            map.insert(QUrl::fromLocalFile(QString("%1/%2.txt").arg(prefix).arg(i)), DFMSearchResult());
        return map;
    }

    stub_ext::StubExt stub;
    QUrl searchUrl;
    SearchDirIterator *iterator = nullptr;
};

// ---------- SearchResultBuffer double buffering ----------

TEST_F(UT_SearchDirIteratorCov, Buffer_UpdateThenGet_SwitchesActiveBuffer)
{
    // Arrange
    SearchResultBuffer buffer;
    DFMSearchResultMap first = makeMap(2, "/tmp/one");

    // Act
    buffer.updateResults(first);
    const DFMSearchResultMap gotFirst = buffer.getResults();
    DFMSearchResultMap second = makeMap(3, "/tmp/two");
    buffer.updateResults(second);
    const DFMSearchResultMap gotSecond = buffer.getResults();

    // Assert
    EXPECT_EQ(gotFirst.size(), 2);
    EXPECT_EQ(gotSecond.size(), 3);   // active buffer switched to the newest write
}

TEST_F(UT_SearchDirIteratorCov, Buffer_Consume_DrainsActiveBuffer)
{
    // Arrange
    SearchResultBuffer buffer;
    buffer.updateResults(makeMap(2, "/tmp/three"));

    // Act
    const DFMSearchResultMap consumed = buffer.consumeResults();
    const bool emptyAfterConsume = buffer.isEmpty();

    // Assert
    EXPECT_EQ(consumed.size(), 2);
    EXPECT_TRUE(emptyAfterConsume);
    EXPECT_TRUE(buffer.getResults().isEmpty());
}

// ---------- private slots ----------

TEST_F(UT_SearchDirIteratorCov, OnMatched_MatchingTaskId_UpdatesBuffer)
{
    // Arrange
    SearchDirIteratorPrivate *d = iterator->d;
    ASSERT_NE(d, nullptr);
    stub.set_lamda(&SearchManager::matchedResults,
                   [](SearchManager *, const QString &) -> DFMSearchResultMap {
                       return makeMap(2, "/tmp/matched");
                   });

    // Act
    d->onMatched(d->taskId);

    // Assert
    EXPECT_EQ(d->resultBuffer.getResults().size(), 2);
    EXPECT_FALSE(d->hasConsumedResults.load());
}

TEST_F(UT_SearchDirIteratorCov, OnMatched_OtherId_Ignored)
{
    // Arrange
    SearchDirIteratorPrivate *d = iterator->d;
    stub.set_lamda(&SearchManager::matchedResults,
                   [](SearchManager *, const QString &) -> DFMSearchResultMap {
                       return makeMap(1, "/tmp/other");
                   });

    // Act
    d->onMatched(QStringLiteral("not-my-task"));

    // Assert
    EXPECT_TRUE(d->resultBuffer.isEmpty());
    EXPECT_EQ(d->resultBuffer.getResults().size(), 0);
}

TEST_F(UT_SearchDirIteratorCov, OnSearchCompleted_MatchingId_MarksFinished)
{
    // Arrange
    SearchDirIteratorPrivate *d = iterator->d;
    EXPECT_FALSE(d->searchFinished.load());

    // Act
    d->onSearchCompleted(QStringLiteral("foreign-id"));
    const bool afterForeign = d->searchFinished.load();
    d->onSearchCompleted(d->taskId);

    // Assert
    EXPECT_FALSE(afterForeign);
    EXPECT_TRUE(d->searchFinished.load());
    EXPECT_EQ(d->resultBuffer.getResults().size(), 0);
}

TEST_F(UT_SearchDirIteratorCov, OnSearchStoped_MatchingId_EmitsStopAndFlags)
{
    // Arrange
    SearchDirIteratorPrivate *d = iterator->d;
    QSignalSpy spy(iterator, &SearchDirIterator::sigStopSearch);

    // Act
    d->onSearchStoped(d->taskId);

    // Assert
    EXPECT_EQ(spy.count(), 1);
    EXPECT_TRUE(d->searchStoped.load());
}

// ---------- doCompleteSortInfo ----------

TEST_F(UT_SearchDirIteratorCov, DoCompleteSortInfo_CompletesGivenSortInfo)
{
    // Arrange
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QTemporaryFile realFile(dir.filePath("file.txt"));
    ASSERT_TRUE(realFile.open());
    realFile.write("hello");
    realFile.close();

    SortInfoPointer sortInfo = SortInfoPointer::create();
    sortInfo->setUrl(QUrl::fromLocalFile(realFile.fileName()));
    EXPECT_FALSE(sortInfo->isInfoCompleted());

    // Act (private non-slot method: reachable through the access switch above)
    iterator->doCompleteSortInfo(sortInfo);

    // Assert
    EXPECT_TRUE(sortInfo->isInfoCompleted());
    EXPECT_TRUE(sortInfo->isFile());
    EXPECT_FALSE(sortInfo->isDir());
    EXPECT_EQ(sortInfo->fileSize(), qint64(5));
}
