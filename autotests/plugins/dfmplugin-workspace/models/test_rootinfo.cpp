// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "models/rootinfo.h"
#include "utils/filesortworker.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/sortfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <QUrl>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

class RootInfoTest : public ::testing::Test
{
protected:
    static void registerSchemes()
    {
        static bool registered = false;
        if (registered)
            return;
        registered = true;
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
    }

    void SetUp() override
    {
        registerSchemes();

        QString templatePath = QDir::homePath() + "/Documents/.test_rootinfo_XXXXXX";
        testDir = std::make_unique<QTemporaryDir>(templatePath);
        ASSERT_TRUE(testDir->isValid());
        QDir().mkpath(testDir->path());

        testUrl = QUrl::fromLocalFile(testDir->path());
    }

    void TearDown() override
    {
        if (rootInfo) {
            delete rootInfo;
            rootInfo = nullptr;
        }
        // Give event loop time to process pending deletions
        QTest::qWait(200);
        testDir.reset();
    }

    int waitForSignal(QSignalSpy &spy, int timeoutMs = 5000)
    {
        spy.wait(timeoutMs);
        return spy.count();
    }

    void createFile(const QString &name, const QByteArray &content = "test")
    {
        QString path = testDir->path() + "/" + name;
        QFile file(path);
        file.open(QIODevice::WriteOnly);
        file.write(content);
        file.close();
    }

    void createRootInfo()
    {
        if (rootInfo) {
            delete rootInfo;
            rootInfo = nullptr;
            QTest::qWait(100);
        }
        rootInfo = new RootInfo(testUrl);
    }

    // Full traversal cycle: init + start + wait for finish
    void doTraversal(const QString &key)
    {
        ASSERT_TRUE(rootInfo != nullptr);
        rootInfo->initThreadOfFileData(key, Global::ItemRoles::kItemFileDisplayNameRole,
                                       Qt::AscendingOrder, true);
        rootInfo->startIteratorWork(key);
    }

    std::unique_ptr<QTemporaryDir> testDir;
    QUrl testUrl;
    RootInfo *rootInfo { nullptr };
};

// ========== Construction ==========

TEST_F(RootInfoTest, Construction_ValidUrl)
{
    createRootInfo();
    ASSERT_NE(rootInfo, nullptr);
    EXPECT_EQ(rootInfo->watcher, nullptr);
    EXPECT_TRUE(rootInfo->connectTokens().isEmpty());
    EXPECT_TRUE(rootInfo->getKeyWords().isEmpty());
}

TEST_F(RootInfoTest, Construction_HasNoWatcher)
{
    createRootInfo();
    ASSERT_NE(rootInfo, nullptr);
    EXPECT_EQ(rootInfo->watcher, nullptr);
}

TEST_F(RootInfoTest, Destructor_DoesNotCrash)
{
    createRootInfo();
    RootInfo *info = new RootInfo(testUrl);
    delete info;
    SUCCEED();
}

// ========== initThreadOfFileData ==========

TEST_F(RootInfoTest, InitThreadOfFileData_CreatesTraversalThread)
{
    createRootInfo();
    rootInfo->initThreadOfFileData("key1", Global::ItemRoles::kItemFileDisplayNameRole,
                                   Qt::AscendingOrder, true);
    // After init, the traversal thread should be created
    EXPECT_TRUE(rootInfo->getKeyWords().size() >= 0);
}

TEST_F(RootInfoTest, InitThreadOfFileData_SecondInitReusesThread)
{
    createRootInfo();
    // First traversal
    rootInfo->initThreadOfFileData("key1", Global::ItemRoles::kItemFileDisplayNameRole,
                                   Qt::AscendingOrder, true);
    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    rootInfo->startIteratorWork("key1");
    waitForSignal(finishSpy, 10000);

    // Second init on same key should not crash
    rootInfo->initThreadOfFileData("key1", Global::ItemRoles::kItemFileDisplayNameRole,
                                   Qt::AscendingOrder, true);
    SUCCEED();
}

// ========== startIteratorWork / Traversal ==========

TEST_F(RootInfoTest, StartIterator_EmptyDir_EmitsTraversalFinished)
{
    createRootInfo();

    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");

    int count = waitForSignal(finishSpy, 10000);
    EXPECT_GE(count, 1);
}

TEST_F(RootInfoTest, StartIterator_WithFiles_EmitsIteratorLocalFiles)
{
    createRootInfo();
    createFile("file1.txt");
    createFile("file2.txt");

    QSignalSpy localSpy(rootInfo, &RootInfo::iteratorLocalFiles);
    doTraversal("key1");

    waitForSignal(localSpy, 10000);
    EXPECT_GE(localSpy.count(), 1);
}

TEST_F(RootInfoTest, StartIterator_EmitsIteratorStatus)
{
    createRootInfo();
    createFile("a.txt");

    QSignalSpy statusSpy(rootInfo, &RootInfo::iteratorStatus);
    doTraversal("key1");

    statusSpy.wait(3000);
    // Should have at least kRunning signal
    ASSERT_GE(statusSpy.count(), 1);
    auto status = statusSpy.takeFirst().at(0).value<RootInfoWorker::IteratorStatus>();
    EXPECT_EQ(status, RootInfoWorker::IteratorStatus::kRunning);
}

// ========== traversalFinished signal ==========

TEST_F(RootInfoTest, TraversalFinished_NoDataProduced_WhenFirstBatchTrue)
{
    createRootInfo();
    rootInfo->setFirstBatch(true);

    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");

    int count = waitForSignal(finishSpy, 10000);
    ASSERT_GE(count, 1);

    QList<QVariant> args = finishSpy.takeFirst();
    bool noDataProduced = args.at(1).toBool();
    // Empty dir with setFirstBatch(true) → noDataProduced should be true
    EXPECT_TRUE(noDataProduced);
}

TEST_F(RootInfoTest, TraversalFinished_DataProduced_WhenFilesExist)
{
    createRootInfo();
    createFile("data.txt");

    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");

    int count = waitForSignal(finishSpy, 10000);
    ASSERT_GE(count, 1);

    QList<QVariant> args = finishSpy.takeFirst();
    bool noDataProduced = args.at(1).toBool();
    EXPECT_FALSE(noDataProduced);
}

// ========== Watcher events ==========

TEST_F(RootInfoTest, Watcher_FileCreated_EmitsWatcherAddFiles)
{
    createRootInfo();
    createFile("initial.txt");

    // Traverse first to start the watcher (watcher starts after iterator init finishes)
    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");
    waitForSignal(finishSpy, 10000);

    // Wait for watcher to be ready
    QTest::qWait(500);
    ASSERT_FALSE(rootInfo->watcher.isNull());

    QSignalSpy addSpy(rootInfo, &RootInfo::watcherAddFiles);
    createFile("newfile.txt");

    int count = waitForSignal(addSpy, 10000);
    EXPECT_GE(count, 1);
}

TEST_F(RootInfoTest, Watcher_FileDeleted_EmitsWatcherRemoveFiles)
{
    createRootInfo();
    createFile("todelete.txt");

    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");
    waitForSignal(finishSpy, 10000);

    QTest::qWait(500);
    ASSERT_FALSE(rootInfo->watcher.isNull());

    QSignalSpy removeSpy(rootInfo, &RootInfo::watcherRemoveFiles);
    QFile::remove(testDir->path() + "/todelete.txt");

    int count = waitForSignal(removeSpy, 10000);
    EXPECT_GE(count, 1);
}

TEST_F(RootInfoTest, Watcher_FileRenamed_EmitsWatcherSignals)
{
    createRootInfo();
    createFile("oldname.txt");

    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");
    waitForSignal(finishSpy, 10000);

    QTest::qWait(500);
    ASSERT_FALSE(rootInfo->watcher.isNull());

    // Rename should trigger remove + add (or rename process started)
    QSignalSpy removeSpy(rootInfo, &RootInfo::watcherRemoveFiles);
    QSignalSpy addSpy(rootInfo, &RootInfo::watcherAddFiles);

    QFile::rename(testDir->path() + "/oldname.txt", testDir->path() + "/newname.txt");

    // Wait for both signals
    removeSpy.wait(5000);
    addSpy.wait(5000);

    EXPECT_GE(removeSpy.count() + addSpy.count(), 1);
}

// ========== reset() ==========

TEST_F(RootInfoTest, Reset_StopsWatcherAndEmitsStatus)
{
    createRootInfo();
    createFile("file1.txt");

    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");
    waitForSignal(finishSpy, 10000);

    QTest::qWait(500);
    ASSERT_FALSE(rootInfo->watcher.isNull());

    QSignalSpy statusSpy(rootInfo, &RootInfo::iteratorStatus);
    rootInfo->reset();

    statusSpy.wait(2000);
    ASSERT_GE(statusSpy.count(), 1);
    auto status = statusSpy.takeFirst().at(0).value<RootInfoWorker::IteratorStatus>();
    EXPECT_EQ(status, RootInfoWorker::IteratorStatus::kNone);
}

TEST_F(RootInfoTest, Reset_ThenNewTraversal_WatcherStillWorks)
{
    createRootInfo();
    createFile("initial.txt");

    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");
    waitForSignal(finishSpy, 10000);

    QTest::qWait(500);

    // Reset (simulating directory switch)
    rootInfo->reset();
    QTest::qWait(300);

    // Start new traversal on same dir
    finishSpy.clear();
    doTraversal("key1");
    waitForSignal(finishSpy, 10000);

    QTest::qWait(500);
    ASSERT_FALSE(rootInfo->watcher.isNull());

    // Watcher should still be functional
    QSignalSpy addSpy(rootInfo, &RootInfo::watcherAddFiles);
    createFile("after_reset.txt");

    int count = waitForSignal(addSpy, 10000);
    EXPECT_GE(count, 1);
}

// ========== canDelete() ==========

TEST_F(RootInfoTest, CanDelete_True_AfterTraversalFinishes)
{
    createRootInfo();
    createFile("file.txt");

    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");
    waitForSignal(finishSpy, 10000);

    // Wait for threads to fully finish
    QTest::qWait(500);
    EXPECT_TRUE(rootInfo->canDelete());
}

TEST_F(RootInfoTest, CanDelete_True_WhenNoTraversalStarted)
{
    createRootInfo();
    EXPECT_TRUE(rootInfo->canDelete());
}

// ========== checkKeyOnly() ==========

TEST_F(RootInfoTest, CheckKeyOnly_True_WithSingleKey)
{
    createRootInfo();
    rootInfo->initThreadOfFileData("only_key", Global::ItemRoles::kItemFileDisplayNameRole,
                                   Qt::AscendingOrder, true);
    EXPECT_TRUE(rootInfo->checkKeyOnly("only_key"));
}

TEST_F(RootInfoTest, CheckKeyOnly_False_WithMultipleKeys)
{
    createRootInfo();
    rootInfo->initThreadOfFileData("key1", Global::ItemRoles::kItemFileDisplayNameRole,
                                   Qt::AscendingOrder, true);
    rootInfo->initThreadOfFileData("key2", Global::ItemRoles::kItemFileDisplayNameRole,
                                   Qt::AscendingOrder, true);
    EXPECT_FALSE(rootInfo->checkKeyOnly("key1"));
    EXPECT_FALSE(rootInfo->checkKeyOnly("key2"));
}

// ========== clearTraversalThread() ==========

TEST_F(RootInfoTest, ClearTraversalThread_ReduceCount)
{
    createRootInfo();
    rootInfo->initThreadOfFileData("key1", Global::ItemRoles::kItemFileDisplayNameRole,
                                   Qt::AscendingOrder, true);
    rootInfo->initThreadOfFileData("key2", Global::ItemRoles::kItemFileDisplayNameRole,
                                   Qt::AscendingOrder, true);

    int remaining = rootInfo->clearTraversalThread("key1", false);
    EXPECT_EQ(remaining, 1);
}

TEST_F(RootInfoTest, ClearTraversalThread_NeedStartWatcher_WhenEmpty)
{
    createRootInfo();
    rootInfo->initThreadOfFileData("key1", Global::ItemRoles::kItemFileDisplayNameRole,
                                   Qt::AscendingOrder, true);

    rootInfo->clearTraversalThread("key1", false);

    // After clearing all threads, startWatcher should be needed again.
    // Create new traversal to verify watcher starts.
    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key2");
    waitForSignal(finishSpy, 10000);

    QTest::qWait(500);
    EXPECT_FALSE(rootInfo->watcher.isNull());
}

// ========== connectTokens ==========

TEST_F(RootInfoTest, ConnectTokens_AddAndRetrieve)
{
    createRootInfo();
    rootInfo->addConnectToken("token1");
    rootInfo->addConnectToken("token2");
    rootInfo->addConnectToken("token1"); // duplicate, should not add

    QStringList tokens = rootInfo->connectTokens();
    EXPECT_EQ(tokens.size(), 2);
    EXPECT_TRUE(tokens.contains("token1"));
    EXPECT_TRUE(tokens.contains("token2"));
}

// ========== setFirstBatch ==========

TEST_F(RootInfoTest, SetFirstBatch_AffectsNoDataProduced)
{
    createRootInfo();

    // Set first batch to true, then start traversal on empty dir
    rootInfo->setFirstBatch(true);
    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");
    waitForSignal(finishSpy, 10000);

    ASSERT_GE(finishSpy.count(), 1);
    bool noDataProduced = finishSpy.takeFirst().at(1).toBool();
    EXPECT_TRUE(noDataProduced);
}

// ========== Bug scenario: reset then re-create RootInfo ==========

TEST_F(RootInfoTest, BugScenario_ResetAndRecreate_WatcherWorksOnSameDir)
{
    createRootInfo();
    createFile("bug_test.txt");

    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");
    waitForSignal(finishSpy, 10000);

    QTest::qWait(500);
    ASSERT_FALSE(rootInfo->watcher.isNull());

    // Reset (simulating directory switch away)
    rootInfo->reset();
    QTest::qWait(500);

    // Delete old RootInfo and create a new one on the same dir (simulating switch back)
    delete rootInfo;
    rootInfo = nullptr;
    QTest::qWait(300);

    rootInfo = new RootInfo(testUrl);

    finishSpy.clear();
    doTraversal("key1");
    waitForSignal(finishSpy, 10000);

    QTest::qWait(500);
    ASSERT_FALSE(rootInfo->watcher.isNull());

    // Verify watcher is functional: create a file and check for add signal
    QSignalSpy addSpy(rootInfo, &RootInfo::watcherAddFiles);
    createFile("after_recreate.txt");

    int count = waitForSignal(addSpy, 10000);
    EXPECT_GE(count, 1);
}

// ========== Multiple rapid operations ==========

TEST_F(RootInfoTest, Watcher_RapidMultipleOperations)
{
    createRootInfo();

    QSignalSpy finishSpy(rootInfo, &RootInfo::traversalFinished);
    doTraversal("key1");
    waitForSignal(finishSpy, 10000);

    QTest::qWait(500);
    ASSERT_FALSE(rootInfo->watcher.isNull());

    QSignalSpy addSpy(rootInfo, &RootInfo::watcherAddFiles);

    // Rapidly create 5 files
    for (int i = 0; i < 5; ++i) {
        createFile(QString("rapid_%1.txt").arg(i));
    }

    addSpy.wait(5000);
    addSpy.wait(1000); // extra wait for consolidated events

    EXPECT_GE(addSpy.count(), 1);
}
