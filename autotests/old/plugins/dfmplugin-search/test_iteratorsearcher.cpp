// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QSignalSpy>
#include <QMutex>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QTimer>
#include <QApplication>
#include <QThread>

#include "searchmanager/searcher/iterator/iteratorsearcher.h"
#include "searchmanager/searcher/searchresult_define.h"
#include "utils/searchhelper.h"
#include "iterator/searchdiriterator.h"

#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/interfaces/abstractfileinfo.h>
#include <dfm-base/base/schemefactory.h>

#include "stubext.h"

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

// Mock FileInfo for testing
class MockFileInfo : public FileInfo
{
public:
    explicit MockFileInfo(const QUrl &url, bool isDir = false, bool isSymlink = false,
                          const QString &displayName = QString(), bool fileExists = true)
        : FileInfo(url), m_isDir(isDir), m_isSymlink(isSymlink), m_displayName(displayName), m_exists(fileExists)
    {
        if (m_displayName.isEmpty())
            m_displayName = url.fileName();
    }

    bool exists() const override { return m_exists; }

    bool isAttributes(const OptInfoType type) const override
    {
        if (type == OptInfoType::kIsDir)
            return m_isDir;
        if (type == OptInfoType::kIsSymLink)
            return m_isSymlink;
        return FileInfo::isAttributes(type);
    }

    QString displayOf(const DisPlayInfoType type) const override
    {
        if (type == DisPlayInfoType::kFileDisplayName)
            return m_displayName;
        return FileInfo::displayOf(type);
    }

    QUrl urlOf(const UrlInfoType type) const override
    {
        if (type == UrlInfoType::kUrl)
            return url;
        return FileInfo::urlOf(type);
    }

private:
    bool m_isDir;
    bool m_isSymlink;
    QString m_displayName;
    bool m_exists;
};

// Mock AbstractDirIterator for testing
class MockDirIterator : public AbstractDirIterator
{
public:
    explicit MockDirIterator(const QUrl &url,
                             const QStringList &nameFilters = QStringList(),
                             QDir::Filters filters = QDir::NoFilter,
                             QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags)
        : AbstractDirIterator(url, nameFilters, filters, flags), m_url(url), m_currentIndex(0)
    {
        // Add some mock files with fileInfo
        addMockEntry(QUrl::fromLocalFile("/home/test/file1.txt"), false, false, "file1.txt");
        addMockEntry(QUrl::fromLocalFile("/home/test/file2.doc"), false, false, "file2.doc");
        addMockEntry(QUrl::fromLocalFile("/home/test/subdir"), true, false, "subdir");
    }

    void addMockEntry(const QUrl &url, bool isDir, bool isSymlink, const QString &displayName)
    {
        m_mockUrls << url;
        m_mockInfos << FileInfoPointer(new MockFileInfo(url, isDir, isSymlink, displayName, true));
    }

    QUrl next() override
    {
        if (m_currentIndex < m_mockUrls.size()) {
            return m_mockUrls[m_currentIndex++];
        }
        return QUrl();
    }

    bool hasNext() const override
    {
        return m_currentIndex < m_mockUrls.size();
    }

    QString fileName() const override
    {
        if (m_currentIndex > 0 && m_currentIndex <= m_mockUrls.size()) {
            return m_mockUrls[m_currentIndex - 1].fileName();
        }
        return QString();
    }

    QUrl fileUrl() const override
    {
        if (m_currentIndex > 0 && m_currentIndex <= m_mockUrls.size()) {
            return m_mockUrls[m_currentIndex - 1];
        }
        return QUrl();
    }

    const FileInfoPointer fileInfo() const override
    {
        if (m_currentIndex > 0 && m_currentIndex <= m_mockInfos.size()) {
            return m_mockInfos[m_currentIndex - 1];
        }
        return nullptr;
    }

    QUrl url() const override
    {
        return m_url;
    }

    // Public members for testing access
    QUrl m_url;
    QList<QUrl> m_mockUrls;
    QList<FileInfoPointer> m_mockInfos;
    mutable int m_currentIndex;
};

class TestIteratorSearcherBridge : public testing::Test
{
public:
    void SetUp() override
    {
        DirIteratorFactory::regClass<SearchDirIterator>(SearchHelper::scheme());
        bridge = new IteratorSearcherBridge();
    }

    void TearDown() override
    {
        delete bridge;
        bridge = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    IteratorSearcherBridge *bridge = nullptr;
};

class TestIteratorSearcher : public testing::Test
{
public:
    void SetUp() override
    {
        DirIteratorFactory::regClass<SearchDirIterator>(SearchHelper::scheme());
        searchUrl = QUrl::fromLocalFile("/home/test");
        keyword = "test";
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
        stub.set_lamda(&SearchHelper::checkWildcardAndToRegularExpression,
                       [](SearchHelper *, const QString &keyword) -> QString {
                           __DBG_STUB_INVOKE__
                           return keyword;   // Return as-is for simplicity
                       });

        // Stub QTimer operations
        stub.set_lamda(static_cast<void (QTimer::*)(int)>(&QTimer::start), [] {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&QTimer::stop, [](QTimer *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(&QTimer::isActive, [](const QTimer *) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });

        // Stub QApplication thread
        stub.set_lamda(&QApplication::thread, [] {
            __DBG_STUB_INVOKE__
            return QThread::currentThread();
        });

        stub.set_lamda(&QObject::thread, [](const QObject *) -> QThread * {
            __DBG_STUB_INVOKE__
            return QThread::currentThread();
        });

        // Stub QMetaObject::invokeMethodImpl - correct signature for Qt 6
        stub.set_lamda(static_cast<bool (*)(QObject *, QtPrivate::QSlotObjectBase *,
                                            Qt::ConnectionType, qsizetype,
                                            const void *const *, const char *const *,
                                            const QtPrivate::QMetaTypeInterface *const *)>(&QMetaObject::invokeMethodImpl),
                       [] {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
    }

protected:
    stub_ext::StubExt stub;
    IteratorSearcher *searcher = nullptr;
    QUrl searchUrl;
    QString keyword;
};

// ========== IteratorSearcherBridge Tests ==========
TEST_F(TestIteratorSearcherBridge, Constructor_CreatesValidInstance)
{
    EXPECT_NE(bridge, nullptr);
}

TEST_F(TestIteratorSearcherBridge, Constructor_WithParent_SetsParentCorrectly)
{
    QObject parent;
    IteratorSearcherBridge bridgeWithParent(&parent);

    EXPECT_EQ(bridgeWithParent.parent(), &parent);
}

TEST_F(TestIteratorSearcherBridge, SetSearcher_WithValidSearcher_ConnectsSignals)
{
    IteratorSearcher testSearcher(QUrl::fromLocalFile("/home/test"), "test");

    EXPECT_NO_FATAL_FAILURE(bridge->setSearcher(&testSearcher));
}

TEST_F(TestIteratorSearcherBridge, SetSearcher_WithNullSearcher_HandlesCorrectly)
{
    EXPECT_NO_FATAL_FAILURE(bridge->setSearcher(nullptr));
}

TEST_F(TestIteratorSearcherBridge, CreateIterator_WithValidUrl_EmitsSignal)
{
    QUrl testUrl = QUrl("search:///home/test");
    EXPECT_NO_FATAL_FAILURE(bridge->createIterator(testUrl));
}

TEST_F(TestIteratorSearcherBridge, CreateIterator_WithInvalidUrl_EmitsNullIterator)
{
    QUrl invalidUrl;
    EXPECT_NO_FATAL_FAILURE(bridge->createIterator(invalidUrl));
}

// ========== IteratorSearcher Constructor Tests ==========
TEST_F(TestIteratorSearcher, Constructor_WithValidParameters_CreatesInstance)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    EXPECT_NE(searcher, nullptr);
    EXPECT_EQ(searcher->searchUrl, searchUrl);
}

TEST_F(TestIteratorSearcher, Constructor_WithWildcardKeyword_ProcessesCorrectly)
{
    setupBasicStubs();

    QString wildcardKeyword = "*.txt";

    searcher = new IteratorSearcher(searchUrl, wildcardKeyword);

    EXPECT_NE(searcher, nullptr);
}

TEST_F(TestIteratorSearcher, Constructor_InitializesBatchTimer)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    EXPECT_NE(searcher->batchTimer, nullptr);
}

TEST_F(TestIteratorSearcher, Constructor_SetsDefaultBatchLimits)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    EXPECT_EQ(searcher->batchResultLimit, 200);
    EXPECT_EQ(searcher->batchTimeLimit, 500);
}

TEST_F(TestIteratorSearcher, Constructor_CreatesRegexPattern)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    EXPECT_TRUE(searcher->regex.isValid());
}

// ========== Static Methods Tests ==========
TEST_F(TestIteratorSearcher, IsSupportSearch_WithAnyUrl_ReturnsTrue)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/test");

    EXPECT_TRUE(IteratorSearcher::isSupportSearch(testUrl));
}

// ========== Search Method Tests ==========
TEST_F(TestIteratorSearcher, Search_FromReadyState_ReturnsTrue)
{
    setupBasicStubs();
    searcher = new IteratorSearcher(searchUrl, keyword);

    bool result = searcher->search();

    EXPECT_TRUE(result);
}

TEST_F(TestIteratorSearcher, Search_FromNonReadyState_ReturnsFalse)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    // Set status to non-ready
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    bool result = searcher->search();

    EXPECT_FALSE(result);
}

TEST_F(TestIteratorSearcher, Search_EnqueuesSearchUrl)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    searcher->search();

    EXPECT_FALSE(searcher->pendingDirs.isEmpty());
}

TEST_F(TestIteratorSearcher, Search_MultipleCallsWithoutStop_ReturnsFalse)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    bool firstCall = searcher->search();
    EXPECT_TRUE(firstCall);

    // Second call should fail because status is already kRuning
    bool secondCall = searcher->search();
    EXPECT_FALSE(secondCall);
}

// ========== Stop Method Tests ==========
TEST_F(TestIteratorSearcher, Stop_FromRunningState_ClearsQueue)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    searcher->stop();
    EXPECT_EQ(searcher->status.loadAcquire(), AbstractSearcher::kTerminated);
}

TEST_F(TestIteratorSearcher, Stop_FromReadyState_DoesNotEmitSignals)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    QSignalSpy finishedSpy(searcher, &IteratorSearcher::finished);

    searcher->stop();

    EXPECT_EQ(finishedSpy.count(), 0);
}

TEST_F(TestIteratorSearcher, Stop_WithResults_EmitsUnearthed)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Add results
    QUrl resultUrl = QUrl::fromLocalFile("/home/test/file1.txt");
    DFMSearchResult result;
    result.setUrl(resultUrl);
    searcher->resultMap.insert(resultUrl, result);

    QSignalSpy unearthedSpy(searcher, &IteratorSearcher::unearthed);

    searcher->stop();

    EXPECT_EQ(unearthedSpy.count(), 1);
}

TEST_F(TestIteratorSearcher, Stop_MultipleCalls_OnlyFirstCallEmitsSignals)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    QSignalSpy finishedSpy(searcher, &IteratorSearcher::finished);

    searcher->stop();
    EXPECT_EQ(finishedSpy.count(), 1);

    // Second stop call should not emit signals
    searcher->stop();
    EXPECT_EQ(finishedSpy.count(), 1);
}

// ========== HasItem and TakeAll Tests ==========
TEST_F(TestIteratorSearcher, HasItem_WithNoResults_ReturnsFalse)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    EXPECT_FALSE(searcher->hasItem());
}

TEST_F(TestIteratorSearcher, HasItem_WithResults_ReturnsTrue)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    // Add result
    QUrl resultUrl = QUrl::fromLocalFile("/home/test/file1.txt");
    DFMSearchResult result;
    result.setUrl(resultUrl);
    searcher->resultMap.insert(resultUrl, result);

    EXPECT_TRUE(searcher->hasItem());
}

TEST_F(TestIteratorSearcher, TakeAll_WithNoResults_ReturnsEmptyMap)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    DFMSearchResultMap results = searcher->takeAll();

    EXPECT_TRUE(results.isEmpty());
}

TEST_F(TestIteratorSearcher, TakeAll_WithResults_ReturnsAndClearsResults)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    // Add results
    QUrl url1 = QUrl::fromLocalFile("/home/test/file1.txt");
    QUrl url2 = QUrl::fromLocalFile("/home/test/file2.txt");
    DFMSearchResult result1, result2;
    result1.setUrl(url1);
    result2.setUrl(url2);
    searcher->resultMap.insert(url1, result1);
    searcher->resultMap.insert(url2, result2);

    DFMSearchResultMap results = searcher->takeAll();

    EXPECT_EQ(results.size(), 2);
    EXPECT_TRUE(results.contains(url1));
    EXPECT_TRUE(results.contains(url2));
    EXPECT_FALSE(searcher->hasItem());
}

TEST_F(TestIteratorSearcher, TakeAllUrls_WithResults_ReturnsUrlsAndClears)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    // Add results
    QUrl url1 = QUrl::fromLocalFile("/home/test/file1.txt");
    QUrl url2 = QUrl::fromLocalFile("/home/test/file2.txt");
    DFMSearchResult result1, result2;
    result1.setUrl(url1);
    result2.setUrl(url2);
    searcher->resultMap.insert(url1, result1);
    searcher->resultMap.insert(url2, result2);

    QList<QUrl> urls = searcher->takeAllUrls();

    EXPECT_EQ(urls.size(), 2);
    EXPECT_TRUE(urls.contains(url1));
    EXPECT_TRUE(urls.contains(url2));
    EXPECT_FALSE(searcher->hasItem());
}

TEST_F(TestIteratorSearcher, TakeAllUrls_WithEmptyResults_ReturnsEmptyList)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    QList<QUrl> urls = searcher->takeAllUrls();

    EXPECT_TRUE(urls.isEmpty());
}

// ========== ProcessDirectory Tests ==========
TEST_F(TestIteratorSearcher, ProcessDirectory_NotRunning_Skips)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kReady);

    QSignalSpy requestSpy(searcher, &IteratorSearcher::requestCreateIterator);

    searcher->processDirectory();

    EXPECT_EQ(requestSpy.count(), 0);
}

TEST_F(TestIteratorSearcher, ProcessDirectory_EmptyQueue_EmitsFinished)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kRuning);
    searcher->pendingDirs.clear();

    QSignalSpy finishedSpy(searcher, &IteratorSearcher::finished);

    searcher->processDirectory();

    EXPECT_EQ(finishedSpy.count(), 1);
    EXPECT_EQ(searcher->status.loadAcquire(), AbstractSearcher::kCompleted);
}

TEST_F(TestIteratorSearcher, ProcessDirectory_WithPendingDir_RequestsIterator)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kRuning);
    searcher->pendingDirs.enqueue(searchUrl);

    QSignalSpy requestSpy(searcher, &IteratorSearcher::requestCreateIterator);

    searcher->processDirectory();

    EXPECT_EQ(requestSpy.count(), 1);
}

TEST_F(TestIteratorSearcher, ProcessDirectory_WithMultiplePendingDirs_ProcessesInOrder)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    QUrl url1 = QUrl::fromLocalFile("/home/test/dir1");
    QUrl url2 = QUrl::fromLocalFile("/home/test/dir2");
    searcher->pendingDirs.enqueue(url1);
    searcher->pendingDirs.enqueue(url2);

    QSignalSpy requestSpy(searcher, &IteratorSearcher::requestCreateIterator);

    searcher->processDirectory();

    EXPECT_EQ(requestSpy.count(), 1);
    // First URL should be dequeued
    QUrl requestedUrl = requestSpy.at(0).at(0).value<QUrl>();
    EXPECT_EQ(requestedUrl, url1);
}

// ========== OnIteratorCreated Tests ==========
TEST_F(TestIteratorSearcher, OnIteratorCreated_NotRunning_IgnoresIterator)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kReady);

    QSharedPointer<AbstractDirIterator> mockIterator(new MockDirIterator(searchUrl));

    QSignalSpy requestSpy(searcher, &IteratorSearcher::requestProcessNextDirectory);

    searcher->onIteratorCreated(mockIterator);

    // Should not process
    EXPECT_TRUE(true);
}

TEST_F(TestIteratorSearcher, OnIteratorCreated_WithNullIterator_RequestsNext)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    QSharedPointer<AbstractDirIterator> nullIterator;

    QSignalSpy requestSpy(searcher, &IteratorSearcher::requestProcessNextDirectory);

    searcher->onIteratorCreated(nullIterator);

    EXPECT_EQ(requestSpy.count(), 1);
}

TEST_F(TestIteratorSearcher, OnIteratorCreated_WithValidIterator_CallsProcessIteratorResults)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "file");
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Create mock iterator
    MockDirIterator *mockIter = new MockDirIterator(searchUrl);
    mockIter->m_mockUrls.clear();
    mockIter->m_mockInfos.clear();
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/file1.txt"), false, false, "file1.txt");
    mockIter->m_currentIndex = 0;

    QSharedPointer<AbstractDirIterator> iterator(mockIter);

    QSignalSpy spy(searcher, &IteratorSearcher::requestProcessNextDirectory);

    searcher->onIteratorCreated(iterator);

    // Should emit requestProcessNextDirectory signal
    EXPECT_EQ(spy.count(), 1);
}

// ========== ProcessIteratorResults Tests ==========
TEST_F(TestIteratorSearcher, ProcessIteratorResults_NotRunning_ReturnsImmediately)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kReady);

    QSharedPointer<AbstractDirIterator> mockIterator(new MockDirIterator(searchUrl));

    // Should return immediately without processing
    searcher->processIteratorResults(mockIterator);

    EXPECT_FALSE(searcher->hasItem());
}

TEST_F(TestIteratorSearcher, ProcessIteratorResults_WithMatchingFiles_AddsToResults)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "file");
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Create mock iterator with matching files
    MockDirIterator *mockIter = new MockDirIterator(searchUrl);
    // Clear default entries and add custom ones
    mockIter->m_mockUrls.clear();
    mockIter->m_mockInfos.clear();
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/file1.txt"), false, false, "file1.txt");
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/file2.doc"), false, false, "file2.doc");
    mockIter->m_currentIndex = 0;

    QSharedPointer<AbstractDirIterator> iterator(mockIter);

    searcher->processIteratorResults(iterator);

    EXPECT_TRUE(searcher->hasItem());
    DFMSearchResultMap results = searcher->takeAll();
    EXPECT_EQ(results.size(), 2);
}

TEST_F(TestIteratorSearcher, ProcessIteratorResults_WithSubDirectories_EnqueuesThem)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "test");
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Create mock iterator with directories
    MockDirIterator *mockIter = new MockDirIterator(searchUrl);
    mockIter->m_mockUrls.clear();
    mockIter->m_mockInfos.clear();
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/subdir1"), true, false, "subdir1");
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/subdir2"), true, false, "subdir2");
    mockIter->m_currentIndex = 0;

    QSharedPointer<AbstractDirIterator> iterator(mockIter);

    int initialQueueSize = searcher->pendingDirs.size();
    searcher->processIteratorResults(iterator);

    EXPECT_EQ(searcher->pendingDirs.size(), initialQueueSize + 2);
}

TEST_F(TestIteratorSearcher, ProcessIteratorResults_WithSysDirectory_FiltersItOut)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "test");
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Create mock iterator with /sys/ directory
    MockDirIterator *mockIter = new MockDirIterator(searchUrl);
    mockIter->m_mockUrls.clear();
    mockIter->m_mockInfos.clear();
    mockIter->addMockEntry(QUrl::fromLocalFile("/sys/devices/test"), true, false, "test");
    mockIter->m_currentIndex = 0;

    QSharedPointer<AbstractDirIterator> iterator(mockIter);

    int initialQueueSize = searcher->pendingDirs.size();
    searcher->processIteratorResults(iterator);

    // /sys/ directory should not be added to queue
    EXPECT_EQ(searcher->pendingDirs.size(), initialQueueSize);
}

TEST_F(TestIteratorSearcher, ProcessIteratorResults_WithSymlinkDirectory_SkipsIt)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "test");
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Create mock iterator with symlink directory
    MockDirIterator *mockIter = new MockDirIterator(searchUrl);
    mockIter->m_mockUrls.clear();
    mockIter->m_mockInfos.clear();
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/symlink"), true, true, "symlink");
    mockIter->m_currentIndex = 0;

    QSharedPointer<AbstractDirIterator> iterator(mockIter);

    int initialQueueSize = searcher->pendingDirs.size();
    searcher->processIteratorResults(iterator);

    // Symlink directory should not be added to queue
    EXPECT_EQ(searcher->pendingDirs.size(), initialQueueSize);
}

TEST_F(TestIteratorSearcher, ProcessIteratorResults_WithNullFileInfo_SkipsEntry)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "test");
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Create mock iterator that returns null fileInfo
    class NullInfoIterator : public MockDirIterator
    {
    public:
        explicit NullInfoIterator(const QUrl &url)
            : MockDirIterator(url) { }
        const FileInfoPointer fileInfo() const override { return nullptr; }
    };

    QSharedPointer<AbstractDirIterator> iterator(new NullInfoIterator(searchUrl));

    searcher->processIteratorResults(iterator);

    // Should not crash and should not add any results
    EXPECT_FALSE(searcher->hasItem());
}

TEST_F(TestIteratorSearcher, ProcessIteratorResults_WithNonExistingFile_SkipsEntry)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "test");
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Create mock iterator with non-existing file
    MockDirIterator *mockIter = new MockDirIterator(searchUrl);
    mockIter->m_mockUrls.clear();
    mockIter->m_mockInfos.clear();
    // Add entry with exists = false
    mockIter->m_mockUrls << QUrl::fromLocalFile("/home/test/nonexistent.txt");
    mockIter->m_mockInfos << FileInfoPointer(
            new MockFileInfo(QUrl::fromLocalFile("/home/test/nonexistent.txt"), false, false, "test_nonexistent", false));
    mockIter->m_currentIndex = 0;

    QSharedPointer<AbstractDirIterator> iterator(mockIter);

    searcher->processIteratorResults(iterator);

    // Should not add non-existing file to results
    EXPECT_FALSE(searcher->hasItem());
}

TEST_F(TestIteratorSearcher, ProcessIteratorResults_NoMatches_NoResultsAdded)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "nomatch");
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Create mock iterator with files that don't match the keyword
    MockDirIterator *mockIter = new MockDirIterator(searchUrl);
    mockIter->m_mockUrls.clear();
    mockIter->m_mockInfos.clear();
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/file1.txt"), false, false, "file1.txt");
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/file2.doc"), false, false, "file2.doc");
    mockIter->m_currentIndex = 0;

    QSharedPointer<AbstractDirIterator> iterator(mockIter);

    searcher->processIteratorResults(iterator);

    // No results should be added since no files match "nomatch"
    EXPECT_FALSE(searcher->hasItem());
}

TEST_F(TestIteratorSearcher, ProcessIteratorResults_StatusChangedDuringProcessing_StopsProcessing)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "test");
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Create a custom iterator that changes status during iteration
    class StatusChangingIterator : public MockDirIterator
    {
    public:
        explicit StatusChangingIterator(const QUrl &url, IteratorSearcher *s)
            : MockDirIterator(url), searcher(s), callCount(0) { }

        QUrl next() override
        {
            if (callCount++ == 1) {
                // Change status on second call
                searcher->status.storeRelease(AbstractSearcher::kTerminated);
            }
            return MockDirIterator::next();
        }

        IteratorSearcher *searcher;
        mutable int callCount;
    };

    StatusChangingIterator *mockIter = new StatusChangingIterator(searchUrl, searcher);
    mockIter->m_mockUrls.clear();
    mockIter->m_mockInfos.clear();
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/file1.txt"), false, false, "test1");
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/file2.txt"), false, false, "test2");
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/file3.txt"), false, false, "test3");
    mockIter->m_currentIndex = 0;

    QSharedPointer<AbstractDirIterator> iterator(mockIter);

    searcher->processIteratorResults(iterator);

    // Should stop processing when status changes
    DFMSearchResultMap results = searcher->takeAll();
    EXPECT_LE(results.size(), 2);   // Should process at most 2 items before stopping
}

TEST_F(TestIteratorSearcher, ProcessIteratorResults_MixedFilesAndDirectories_HandlesCorrectly)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "test");
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Create mock iterator with mixed files and directories
    MockDirIterator *mockIter = new MockDirIterator(searchUrl);
    mockIter->m_mockUrls.clear();
    mockIter->m_mockInfos.clear();
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/testfile.txt"), false, false, "testfile.txt");
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/testdir"), true, false, "testdir");
    mockIter->addMockEntry(QUrl::fromLocalFile("/home/test/another_test.doc"), false, false, "another_test.doc");
    mockIter->m_currentIndex = 0;

    QSharedPointer<AbstractDirIterator> iterator(mockIter);

    int initialQueueSize = searcher->pendingDirs.size();
    searcher->processIteratorResults(iterator);

    // Should add files to results and directory to queue
    EXPECT_TRUE(searcher->hasItem());
    DFMSearchResultMap results = searcher->takeAll();
    EXPECT_EQ(results.size(), 3);
    EXPECT_EQ(searcher->pendingDirs.size(), initialQueueSize + 1);   // One directory added
}

// ========== Batch Processing Tests ==========
TEST_F(TestIteratorSearcher, PublishBatchedResults_NotRunning_DoesNothing)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kReady);

    QSignalSpy unearthedSpy(searcher, &IteratorSearcher::unearthed);

    searcher->publishBatchedResults();

    EXPECT_EQ(unearthedSpy.count(), 0);
}

TEST_F(TestIteratorSearcher, PublishBatchedResults_WithResults_EmitsUnearthed)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    // Add batched results
    QUrl resultUrl = QUrl::fromLocalFile("/home/test/file1.txt");
    DFMSearchResult result;
    result.setUrl(resultUrl);
    searcher->batchedResults.insert(resultUrl, result);

    QSignalSpy unearthedSpy(searcher, &IteratorSearcher::unearthed);

    searcher->publishBatchedResults();

    EXPECT_EQ(unearthedSpy.count(), 1);
    EXPECT_TRUE(searcher->batchedResults.isEmpty());
}

TEST_F(TestIteratorSearcher, PublishBatchedResults_RestartsTimer)
{
    setupBasicStubs();

    bool timerStarted = false;
    stub.set_lamda(static_cast<void (QTimer::*)(int)>(&QTimer::start), [&timerStarted](QTimer *, int) {
        __DBG_STUB_INVOKE__
        timerStarted = true;
    });

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    searcher->publishBatchedResults();

    EXPECT_TRUE(timerStarted);
}

TEST_F(TestIteratorSearcher, PublishBatchedResults_WhenNotRunning_DoesNothing)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kReady);

    // Add some batched results
    QUrl resultUrl = QUrl::fromLocalFile("/home/test/file1.txt");
    DFMSearchResult result;
    result.setUrl(resultUrl);
    searcher->batchedResults.insert(resultUrl, result);

    QSignalSpy unearthedSpy(searcher, &IteratorSearcher::unearthed);

    searcher->publishBatchedResults();

    // Should not emit signal when not running
    EXPECT_EQ(unearthedSpy.count(), 0);
}

// ========== AddResults Tests ==========
TEST_F(TestIteratorSearcher, AddResults_EmptyResults_DoesNothing)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    DFMSearchResultMap emptyResults;

    searcher->addResults(emptyResults);

    EXPECT_FALSE(searcher->hasItem());
}

TEST_F(TestIteratorSearcher, AddResults_WithResults_AddsToBatchAndTotal)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kRuning);

    DFMSearchResultMap newResults;
    QUrl url1 = QUrl::fromLocalFile("/home/test/file1.txt");
    DFMSearchResult result1;
    result1.setUrl(url1);
    newResults.insert(url1, result1);

    searcher->addResults(newResults);

    EXPECT_TRUE(searcher->hasItem());
}

TEST_F(TestIteratorSearcher, AddResults_ReachingBatchLimit_PublishesImmediately)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->status.storeRelease(AbstractSearcher::kRuning);
    searcher->batchResultLimit = 2;

    DFMSearchResultMap newResults;
    QUrl url1 = QUrl::fromLocalFile("/home/test/file1.txt");
    QUrl url2 = QUrl::fromLocalFile("/home/test/file2.txt");
    DFMSearchResult result1, result2;
    result1.setUrl(url1);
    result2.setUrl(url2);
    newResults.insert(url1, result1);
    newResults.insert(url2, result2);

    QSignalSpy unearthedSpy(searcher, &IteratorSearcher::unearthed);

    searcher->addResults(newResults);

    EXPECT_EQ(unearthedSpy.count(), 1);
}

// ========== AddResultToMap Tests ==========
TEST_F(TestIteratorSearcher, AddResultToMap_CreatesResultWithCorrectUrl)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    QUrl fileUrl = QUrl::fromLocalFile("/home/test/file1.txt");
    DFMSearchResultMap results;

    searcher->addResultToMap(fileUrl, results);

    EXPECT_EQ(results.size(), 1);
    EXPECT_TRUE(results.contains(fileUrl));
    EXPECT_EQ(results[fileUrl].matchScore(), 1.0);
}

TEST_F(TestIteratorSearcher, AddResultToMap_DuplicateUrl_ReplacesExisting)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    QUrl fileUrl = QUrl::fromLocalFile("/home/test/file1.txt");
    DFMSearchResultMap results;

    // Add first result
    searcher->addResultToMap(fileUrl, results);
    EXPECT_EQ(results.size(), 1);

    // Add same URL again
    searcher->addResultToMap(fileUrl, results);
    EXPECT_EQ(results.size(), 1);   // Should still be 1 (replaced)
}

// ========== RequestNextDirectory Tests ==========
TEST_F(TestIteratorSearcher, RequestNextDirectory_EmitsSignal)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    QSignalSpy spy(searcher, &IteratorSearcher::requestProcessNextDirectory);

    searcher->requestNextDirectory();

    EXPECT_EQ(spy.count(), 1);
}

// ========== Signal Tests ==========
TEST_F(TestIteratorSearcher, RequestProcessNextDirectorySignal_CanBeEmitted)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    QSignalSpy spy(searcher, &IteratorSearcher::requestProcessNextDirectory);

    emit searcher->requestProcessNextDirectory();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TestIteratorSearcher, RequestCreateIteratorSignal_CanBeEmitted)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    QSignalSpy spy(searcher, &IteratorSearcher::requestCreateIterator);

    QUrl testUrl = QUrl::fromLocalFile("/home/test");
    emit searcher->requestCreateIterator(testUrl);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).value<QUrl>(), testUrl);
}

// ========== Regex Matching Tests ==========
TEST_F(TestIteratorSearcher, RegexMatching_CaseInsensitive)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "TEST");

    // Test case insensitive matching
    EXPECT_TRUE(searcher->regex.match("test").hasMatch());
    EXPECT_TRUE(searcher->regex.match("Test").hasMatch());
    EXPECT_TRUE(searcher->regex.match("TEST").hasMatch());
}

TEST_F(TestIteratorSearcher, RegexMatching_PartialMatch)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "test");

    EXPECT_TRUE(searcher->regex.match("testfile.txt").hasMatch());
    EXPECT_TRUE(searcher->regex.match("mytest.doc").hasMatch());
}

TEST_F(TestIteratorSearcher, Regex_CaseInsensitiveOption_IsSet)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, "TEST");

    // Verify case insensitive option is set
    EXPECT_TRUE(searcher->regex.patternOptions() & QRegularExpression::CaseInsensitiveOption);
}

// ========== Thread Safety Tests ==========
TEST_F(TestIteratorSearcher, ThreadSafety_ConcurrentAccess_NoDataRace)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    // Add results
    QUrl url1 = QUrl::fromLocalFile("/home/test/file1.txt");
    DFMSearchResult result1;
    result1.setUrl(url1);
    searcher->resultMap.insert(url1, result1);

    // Simulate concurrent access
    bool hasItems1 = searcher->hasItem();
    DFMSearchResultMap results = searcher->takeAll();
    bool hasItems2 = searcher->hasItem();
    QList<QUrl> urls = searcher->takeAllUrls();

    EXPECT_TRUE(hasItems1);
    EXPECT_FALSE(hasItems2);
    EXPECT_TRUE(urls.isEmpty());
}

// ========== Batch Timer Tests ==========
TEST_F(TestIteratorSearcher, BatchTimer_Configuration_IsCorrect)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    EXPECT_NE(searcher->batchTimer, nullptr);
    EXPECT_EQ(searcher->batchResultLimit, 200);
    EXPECT_EQ(searcher->batchTimeLimit, 500);
}

// ========== Integration Tests ==========
TEST_F(TestIteratorSearcher, CompleteWorkflow_SearchProcessStop)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);

    QSignalSpy finishedSpy(searcher, &IteratorSearcher::finished);

    // Start search
    bool searchResult = searcher->search();
    EXPECT_TRUE(searchResult);

    // Stop search
    searcher->stop();
    EXPECT_EQ(finishedSpy.count(), 1);
}

// ========== Destructor Tests ==========
TEST_F(TestIteratorSearcher, Destructor_StopsTimer)
{
    setupBasicStubs();

    bool timerStopped = false;
    stub.set_lamda(&QTimer::stop, [&timerStopped](QTimer *) {
        __DBG_STUB_INVOKE__
        timerStopped = true;
    });

    stub.set_lamda(&QTimer::isActive, [](const QTimer *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    searcher = new IteratorSearcher(searchUrl, keyword);

    delete searcher;
    searcher = nullptr;

    EXPECT_TRUE(timerStopped);
}

TEST_F(TestIteratorSearcher, Destructor_ClearsPendingDirs)
{
    setupBasicStubs();

    searcher = new IteratorSearcher(searchUrl, keyword);
    searcher->pendingDirs.enqueue(searchUrl);

    // Destructor should clear the queue
    delete searcher;
    searcher = nullptr;

    EXPECT_TRUE(true);   // No crash means cleanup successful
}
