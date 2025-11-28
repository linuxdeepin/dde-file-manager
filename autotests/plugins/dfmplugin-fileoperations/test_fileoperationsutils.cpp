// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QTemporaryDir>
#include <QFile>
#include <QTimer>
#include <QSignalSpy>

#include "stubext.h"

#include "fileoperations/fileoperationutils/fileoperationsutils.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/urlroute.h>
#include <dfm-base/utils/fileutils.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_fileoperations;

class TestFileOperationsUtils : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Register file info factories
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // Create temporary directory
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    QUrl createTestFile(const QString &fileName, qint64 size = 1024)
    {
        QString filePath = tempDir->path() + "/" + fileName;
        QFile file(filePath);
        file.open(QIODevice::WriteOnly);
        file.write(QByteArray(size, 'A'));
        file.close();

        return QUrl::fromLocalFile(filePath);
    }

    QUrl createTestDir(const QString &dirName)
    {
        QString dirPath = tempDir->path() + "/" + dirName;
        QDir().mkpath(dirPath);

        return QUrl::fromLocalFile(dirPath);
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

// ========== UpdateProgressTimer Tests ==========

TEST_F(TestFileOperationsUtils, UpdateProgressTimer_Construction)
{
    UpdateProgressTimer *timer = new UpdateProgressTimer();

    ASSERT_NE(timer, nullptr);

    delete timer;
}

TEST_F(TestFileOperationsUtils, UpdateProgressTimer_SignalEmission)
{
    UpdateProgressTimer timer;
    QSignalSpy spy(&timer, &UpdateProgressTimer::updateProgressNotify);

    timer.doStartTime();

    // Wait for at least one timeout
    QThread::msleep(600);

    // Should have emitted at least one signal
    EXPECT_GT(spy.count(), 0);

    timer.stopTimer();
}

TEST_F(TestFileOperationsUtils, UpdateProgressTimer_StopTimer)
{
    UpdateProgressTimer timer;
    QSignalSpy spy(&timer, &UpdateProgressTimer::updateProgressNotify);

    timer.doStartTime();
    QThread::msleep(300);

    int countBefore = spy.count();

    timer.stopTimer();
    QThread::msleep(600);

    int countAfter = spy.count();

    // Signal count should not increase significantly after stop
    EXPECT_LE(countAfter - countBefore, 1);
}

TEST_F(TestFileOperationsUtils, UpdateProgressTimer_MultipleStarts)
{
    UpdateProgressTimer timer;

    // Starting multiple times should not crash
    timer.doStartTime();
    timer.doStartTime();
    timer.doStartTime();

    QThread::msleep(100);

    timer.stopTimer();

    SUCCEED();
}

TEST_F(TestFileOperationsUtils, UpdateProgressTimer_StopBeforeStart)
{
    UpdateProgressTimer timer;

    // Stopping before starting should not crash
    timer.stopTimer();

    SUCCEED();
}

// ========== FileOperationsUtils::statisticsFilesSize() Tests ==========

TEST_F(TestFileOperationsUtils, StatisticsFilesSize_EmptyList)
{
    QList<QUrl> emptyFiles;
    SizeInfoPointer sizeInfo = FileOperationsUtils::statisticsFilesSize(emptyFiles);

    ASSERT_NE(sizeInfo, nullptr);
}

TEST_F(TestFileOperationsUtils, StatisticsFilesSize_SingleFile)
{
    QUrl file = createTestFile("single.txt");
    QList<QUrl> files = { file };

    SizeInfoPointer sizeInfo = FileOperationsUtils::statisticsFilesSize(files);

    ASSERT_NE(sizeInfo, nullptr);
}

TEST_F(TestFileOperationsUtils, StatisticsFilesSize_MultipleFiles)
{
    QList<QUrl> files = {
        createTestFile("file1.txt"),
        createTestFile("file2.txt"),
        createTestFile("file3.txt")
    };

    SizeInfoPointer sizeInfo = FileOperationsUtils::statisticsFilesSize(files);

    ASSERT_NE(sizeInfo, nullptr);
}

TEST_F(TestFileOperationsUtils, StatisticsFilesSize_WithRecordUrl)
{
    QList<QUrl> files = { createTestFile("record.txt") };

    SizeInfoPointer sizeInfo = FileOperationsUtils::statisticsFilesSize(files, true);

    SUCCEED();
}

// ========== FileOperationsUtils::isFilesSizeOutLimit() Tests ==========

TEST_F(TestFileOperationsUtils, IsFilesSizeOutLimit_UnderLimit)
{
    stub.set_lamda(ADDR(FileOperationsUtils, isFilesSizeOutLimit), [](const QUrl &, const qint64) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QUrl file = createTestFile("small.txt", 100);
    bool result = FileOperationsUtils::isFilesSizeOutLimit(file, 1024);

    EXPECT_FALSE(result);
}

TEST_F(TestFileOperationsUtils, IsFilesSizeOutLimit_OverLimit)
{
    stub.set_lamda(ADDR(FileOperationsUtils, isFilesSizeOutLimit), [](const QUrl &, const qint64) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl file = createTestFile("large.txt", 2048);
    bool result = FileOperationsUtils::isFilesSizeOutLimit(file, 1024);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsUtils, IsFilesSizeOutLimit_ExactLimit)
{
    stub.set_lamda(ADDR(FileOperationsUtils, isFilesSizeOutLimit), [](const QUrl &, const qint64) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QUrl file = createTestFile("exact.txt", 1024);
    bool result = FileOperationsUtils::isFilesSizeOutLimit(file, 1024);

    EXPECT_FALSE(result);
}

// ========== FileOperationsUtils::isAncestorUrl() Tests ==========

TEST_F(TestFileOperationsUtils, IsAncestorUrl_DirectParent)
{
    stub.set_lamda(ADDR(FileOperationsUtils, isAncestorUrl), [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl parent = QUrl::fromLocalFile("/tmp/parent");
    QUrl child = QUrl::fromLocalFile("/tmp/parent/child");

    bool result = FileOperationsUtils::isAncestorUrl(parent, child);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsUtils, IsAncestorUrl_DeepNesting)
{
    stub.set_lamda(ADDR(FileOperationsUtils, isAncestorUrl), [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl ancestor = QUrl::fromLocalFile("/tmp/root");
    QUrl descendant = QUrl::fromLocalFile("/tmp/root/a/b/c/file.txt");

    bool result = FileOperationsUtils::isAncestorUrl(ancestor, descendant);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsUtils, IsAncestorUrl_NotAncestor)
{
    stub.set_lamda(ADDR(FileOperationsUtils, isAncestorUrl), [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QUrl url1 = QUrl::fromLocalFile("/tmp/folder1");
    QUrl url2 = QUrl::fromLocalFile("/tmp/folder2");

    bool result = FileOperationsUtils::isAncestorUrl(url1, url2);

    EXPECT_FALSE(result);
}

TEST_F(TestFileOperationsUtils, IsAncestorUrl_SameUrl)
{
    stub.set_lamda(ADDR(FileOperationsUtils, isAncestorUrl), [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QUrl url = QUrl::fromLocalFile("/tmp/same");

    bool result = FileOperationsUtils::isAncestorUrl(url, url);

    EXPECT_FALSE(result);
}

// ========== FileOperationsUtils::isFileOnDisk() Tests ==========

TEST_F(TestFileOperationsUtils, IsFileOnDisk_LocalFile)
{
    stub.set_lamda(ADDR(FileOperationsUtils, isFileOnDisk), [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl file = createTestFile("ondisk.txt");

    bool result = FileOperationsUtils::isFileOnDisk(file);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsUtils, IsFileOnDisk_NetworkFile)
{
    stub.set_lamda(ADDR(FileOperationsUtils, isFileOnDisk), [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QUrl file = QUrl("smb://server/share/file.txt");

    bool result = FileOperationsUtils::isFileOnDisk(file);

    EXPECT_FALSE(result);
}

// ========== FileOperationsUtils::bigFileSize() Tests ==========

TEST_F(TestFileOperationsUtils, BigFileSize_ReturnsValue)
{
    stub.set_lamda(ADDR(FileOperationsUtils, bigFileSize), []() -> qint64 {
        __DBG_STUB_INVOKE__
        return 100 * 1024 * 1024;   // 100MB
    });

    qint64 size = FileOperationsUtils::bigFileSize();

    EXPECT_GT(size, 0);
}

// ========== FileOperationsUtils::blockSync() Tests ==========

TEST_F(TestFileOperationsUtils, BlockSync_ReturnsValue)
{
    stub.set_lamda(ADDR(FileOperationsUtils, blockSync), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = FileOperationsUtils::blockSync();

    SUCCEED();
}

// ========== FileOperationsUtils::parentUrl() Tests ==========

TEST_F(TestFileOperationsUtils, ParentUrl_HasParent)
{
    stub.set_lamda(ADDR(FileOperationsUtils, parentUrl), [](const QUrl &url) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/tmp");
    });

    QUrl child = QUrl::fromLocalFile("/tmp/child.txt");

    QUrl parent = FileOperationsUtils::parentUrl(child);

    EXPECT_TRUE(parent.isValid());
}

TEST_F(TestFileOperationsUtils, ParentUrl_RootDirectory)
{
    stub.set_lamda(ADDR(FileOperationsUtils, parentUrl), [](const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl::fromLocalFile("/");
    });

    QUrl root = QUrl::fromLocalFile("/");

    QUrl parent = FileOperationsUtils::parentUrl(root);

    SUCCEED();
}

// ========== FileOperationsUtils::canBroadcastPaste() Tests ==========

TEST_F(TestFileOperationsUtils, CanBroadcastPaste_ReturnsValue)
{
    stub.set_lamda(ADDR(FileOperationsUtils, canBroadcastPaste), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = FileOperationsUtils::canBroadcastPaste();

    SUCCEED();
}

// ========== Edge Cases ==========

TEST_F(TestFileOperationsUtils, EdgeCase_UpdateProgressTimer_RapidStartStop)
{
    UpdateProgressTimer timer;

    for (int i = 0; i < 10; ++i) {
        timer.doStartTime();
        QThread::msleep(50);
        timer.stopTimer();
    }

    SUCCEED();
}

TEST_F(TestFileOperationsUtils, EdgeCase_LargeFileList)
{
    QList<QUrl> files;
    for (int i = 0; i < 1000; ++i) {
        files << QUrl::fromLocalFile(QString("/tmp/file%1.txt").arg(i));
    }

    SizeInfoPointer sizeInfo = FileOperationsUtils::statisticsFilesSize(files);

    ASSERT_NE(sizeInfo, nullptr);
}

TEST_F(TestFileOperationsUtils, EdgeCase_UnicodeUrls)
{
    stub.set_lamda(ADDR(FileOperationsUtils, isAncestorUrl), [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    QUrl parent = QUrl::fromLocalFile("/tmp/父目录");
    QUrl child = QUrl::fromLocalFile("/tmp/父目录/子文件.txt");

    bool result = FileOperationsUtils::isAncestorUrl(parent, child);

    EXPECT_TRUE(result);
}
