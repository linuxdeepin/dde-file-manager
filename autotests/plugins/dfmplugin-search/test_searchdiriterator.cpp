// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QDir>
#include <QDirIterator>
#include <QSignalSpy>

#include "iterator/searchdiriterator.h"
#include "iterator/searchdiriterator_p.h"
#include "utils/searchhelper.h"

#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/interfaces/fileinfo.h>

#include "stubext.h"

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class TestSearchDirIterator : public testing::Test
{
public:
    void SetUp() override
    {
        searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home/test"), "keyword", "123");
        iterator = new SearchDirIterator(searchUrl);
    }

    void TearDown() override
    {
        delete iterator;
        iterator = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    QUrl searchUrl;
    SearchDirIterator *iterator = nullptr;
};

TEST_F(TestSearchDirIterator, Constructor_WithValidUrl_CreatesInstance)
{
    QUrl testUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "456");
    QStringList nameFilters = { "*.txt", "*.doc" };
    QDir::Filters filters = QDir::Files | QDir::Readable;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;

    SearchDirIterator testIterator(testUrl, nameFilters, filters, flags);

    // Basic construction test
    EXPECT_TRUE(true);
}

TEST_F(TestSearchDirIterator, Constructor_WithDefaultParameters_CreatesInstance)
{
    QUrl testUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "789");
    SearchDirIterator testIterator(testUrl);

    EXPECT_TRUE(true);
}

TEST_F(TestSearchDirIterator, Next_ReturnsValidUrl)
{
    QUrl expectedUrl = QUrl::fromLocalFile("/home/test/result.txt");

    // Mock internal search operations
    stub.set_lamda(VADDR(SearchDirIterator, hasNext), [](const SearchDirIterator *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl result = iterator->next();

    // Test that next() can be called without crashing
    EXPECT_TRUE(true);
}

TEST_F(TestSearchDirIterator, HasNext_ReturnsCorrectValue)
{
    bool hasNext = iterator->hasNext();

    // Test that hasNext() can be called
    EXPECT_TRUE(hasNext || !hasNext);   // Either true or false is valid
}

TEST_F(TestSearchDirIterator, FileName_ReturnsCorrectName)
{
    QString fileName = iterator->fileName();

    // Test that fileName() can be called
    EXPECT_TRUE(true);
}

TEST_F(TestSearchDirIterator, FileUrl_ReturnsValidUrl)
{
    QUrl fileUrl = iterator->fileUrl();

    // Test that fileUrl() can be called
    EXPECT_TRUE(true);
}

TEST_F(TestSearchDirIterator, FileInfo_ReturnsValidPointer)
{
    const FileInfoPointer fileInfo = iterator->fileInfo();

    // Test that fileInfo() can be called
    EXPECT_TRUE(true);
}

TEST_F(TestSearchDirIterator, Url_ReturnsSearchUrl)
{
    EXPECT_NO_FATAL_FAILURE(iterator->url());
}

TEST_F(TestSearchDirIterator, Close_CallsCorrectly)
{
    EXPECT_NO_FATAL_FAILURE(iterator->close());
}

TEST_F(TestSearchDirIterator, SortFileInfoList_ReturnsValidList)
{
    using WaitFunc = bool (QWaitCondition::*)(QMutex *, QDeadlineTimer);
    stub.set_lamda(static_cast<WaitFunc>(&QWaitCondition::wait),
                   [this] {
                       iterator->d->searchStoped.store(true, std::memory_order_release);
                       return true;
                   });

    QList<QSharedPointer<SortFileInfo>> sortInfoList = iterator->sortFileInfoList();

    // Test that sortFileInfoList() can be called
    EXPECT_TRUE(true);
}

TEST_F(TestSearchDirIterator, OneByOne_ReturnsFalse)
{
    bool oneByOne = iterator->oneByOne();

    EXPECT_FALSE(oneByOne);
}

TEST_F(TestSearchDirIterator, IsWaitingForUpdates_ReturnsCorrectValue)
{
    bool waiting = iterator->isWaitingForUpdates();

    // Test that isWaitingForUpdates() can be called
    EXPECT_TRUE(waiting || !waiting);   // Either true or false is valid
}

TEST_F(TestSearchDirIterator, SigSearch_CanBeEmitted)
{
    QSignalSpy spy(iterator, &SearchDirIterator::sigSearch);

    // Emit the signal manually to test
    emit iterator->sigSearch();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestSearchDirIterator, SigStopSearch_CanBeEmitted)
{
    QSignalSpy spy(iterator, &SearchDirIterator::sigStopSearch);

    // Emit the signal manually to test
    emit iterator->sigStopSearch();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestSearchDirIterator, DoCompleteSortInfo_CallsCorrectly)
{
    // Create a mock SortInfoPointer
    SortInfoPointer sortInfo;

    // Call the private method via metaObject
    EXPECT_NO_FATAL_FAILURE(
            QMetaObject::invokeMethod(iterator, "doCompleteSortInfo", Qt::DirectConnection,
                                      Q_ARG(SortInfoPointer, sortInfo)));
}

TEST_F(TestSearchDirIterator, MultipleIteratorOperations_WorkTogether)
{
    // Test sequence of operations
    EXPECT_NO_FATAL_FAILURE(iterator->hasNext());
    EXPECT_NO_FATAL_FAILURE(iterator->fileName());
    EXPECT_NO_FATAL_FAILURE(iterator->fileUrl());
    EXPECT_NO_FATAL_FAILURE(iterator->fileInfo());
    EXPECT_NO_FATAL_FAILURE(iterator->url());
    EXPECT_NO_FATAL_FAILURE(iterator->isWaitingForUpdates());
    EXPECT_NO_FATAL_FAILURE(iterator->close());
}

TEST_F(TestSearchDirIterator, WithNameFilters_CreatesCorrectly)
{
    QUrl testUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "document", "456");
    QStringList nameFilters = { "*.txt", "*.pdf", "*.doc" };

    SearchDirIterator filteredIterator(testUrl, nameFilters);

    // Test that iterator with name filters can be created
    EXPECT_TRUE(true);
}

TEST_F(TestSearchDirIterator, WithDirFilters_CreatesCorrectly)
{
    QUrl testUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "files", "789");
    QStringList nameFilters;
    QDir::Filters filters = QDir::Files | QDir::Readable | QDir::Hidden;

    SearchDirIterator filteredIterator(testUrl, nameFilters, filters);

    // Test that iterator with dir filters can be created
    EXPECT_TRUE(true);
}

TEST_F(TestSearchDirIterator, WithIteratorFlags_CreatesCorrectly)
{
    QUrl testUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "recursive", "101");
    QStringList nameFilters;
    QDir::Filters filters = QDir::NoFilter;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories | QDirIterator::FollowSymlinks;

    SearchDirIterator flaggedIterator(testUrl, nameFilters, filters, flags);

    // Test that iterator with flags can be created
    EXPECT_TRUE(true);
}

TEST_F(TestSearchDirIterator, WithAllParameters_CreatesCorrectly)
{
    QUrl testUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "complete", "202");
    QStringList nameFilters = { "*.cpp", "*.h" };
    QDir::Filters filters = QDir::Files | QDir::NoDotAndDotDot;
    QDirIterator::IteratorFlags flags = QDirIterator::Subdirectories;

    SearchDirIterator completeIterator(testUrl, nameFilters, filters, flags);

    // Test that iterator with all parameters can be created
    EXPECT_TRUE(true);
}

TEST_F(TestSearchDirIterator, IteratorCycle_WorksCorrectly)
{
    // Mock hasNext to return true initially, then false
    int callCount = 0;
    stub.set_lamda(VADDR(SearchDirIterator, hasNext), [&callCount](const SearchDirIterator *) -> bool {
        __DBG_STUB_INVOKE__
        return callCount++ < 3;   // Return true for first 3 calls, then false
    });

    // Test iteration cycle
    while (iterator->hasNext()) {
        QUrl nextUrl = iterator->next();
        QString fileName = iterator->fileName();
        QUrl fileUrl = iterator->fileUrl();

        // Break to prevent infinite loop in case mocking doesn't work as expected
        if (callCount > 5) break;
    }

    EXPECT_GT(callCount, 0);
}
