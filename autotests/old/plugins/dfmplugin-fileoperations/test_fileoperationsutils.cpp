// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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
    EXPECT_EQ(sizeInfo->fileCount, 0);
}

TEST_F(TestFileOperationsUtils, StatisticsFilesSize_SingleFile)
{
    QUrl file = createTestFile("single.txt", 2048);
    QList<QUrl> files = { file };

    SizeInfoPointer sizeInfo = FileOperationsUtils::statisticsFilesSize(files);

    ASSERT_NE(sizeInfo, nullptr);
    EXPECT_EQ(sizeInfo->fileCount, 1);
    EXPECT_GT(sizeInfo->totalSize, 0);
}

TEST_F(TestFileOperationsUtils, StatisticsFilesSize_MultipleFiles)
{
    QList<QUrl> files = {
        createTestFile("file1.txt", 1024),
        createTestFile("file2.txt", 2048),
        createTestFile("file3.txt", 512)
    };

    SizeInfoPointer sizeInfo = FileOperationsUtils::statisticsFilesSize(files);

    ASSERT_NE(sizeInfo, nullptr);
    EXPECT_EQ(sizeInfo->fileCount, 3);
    EXPECT_GT(sizeInfo->totalSize, 3584);  // At least sum of file sizes
}

TEST_F(TestFileOperationsUtils, StatisticsFilesSize_WithRecordUrl)
{
    QList<QUrl> files = { createTestFile("record.txt") };

    SizeInfoPointer sizeInfo = FileOperationsUtils::statisticsFilesSize(files, true);

    ASSERT_NE(sizeInfo, nullptr);
    EXPECT_GT(sizeInfo->allFiles.size(), 0);  // Should record URLs
}

TEST_F(TestFileOperationsUtils, StatisticsFilesSize_Directory)
{
    QUrl dir = createTestDir("testdir");
    createTestFile("testdir/file1.txt", 1024);
    createTestFile("testdir/file2.txt", 2048);

    QList<QUrl> files = { dir };
    SizeInfoPointer sizeInfo = FileOperationsUtils::statisticsFilesSize(files);

    ASSERT_NE(sizeInfo, nullptr);
    EXPECT_GE(sizeInfo->fileCount, 2);  // At least 2 files
}

// ========== FileOperationsUtils::isFilesSizeOutLimit() Tests ==========

TEST_F(TestFileOperationsUtils, IsFilesSizeOutLimit_UnderLimit)
{
    QUrl file = createTestFile("small.txt", 512);
    bool result = FileOperationsUtils::isFilesSizeOutLimit(file, 10240);

    EXPECT_FALSE(result);
}

TEST_F(TestFileOperationsUtils, IsFilesSizeOutLimit_OverLimit)
{
    QUrl file = createTestFile("large.txt", 2048);
    bool result = FileOperationsUtils::isFilesSizeOutLimit(file, 1024);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsUtils, IsFilesSizeOutLimit_ExactLimit)
{
    QUrl file = createTestFile("exact.txt", 1024);
    bool result = FileOperationsUtils::isFilesSizeOutLimit(file, 1024);

    EXPECT_FALSE(result);  // Exact limit should not be over
}

TEST_F(TestFileOperationsUtils, IsFilesSizeOutLimit_Directory)
{
    QUrl dir = createTestDir("sizedir");
    createTestFile("sizedir/file1.txt", 1024);
    createTestFile("sizedir/file2.txt", 2048);

    bool result = FileOperationsUtils::isFilesSizeOutLimit(dir, 1024);
    EXPECT_TRUE(result);  // Total size > 1024
}

// ========== FileOperationsUtils::isAncestorUrl() Tests ==========

TEST_F(TestFileOperationsUtils, IsAncestorUrl_DirectParent)
{
    QUrl parent = QUrl::fromLocalFile("/tmp/parent");
    QUrl child = QUrl::fromLocalFile("/tmp/parent/child");

    bool result = FileOperationsUtils::isAncestorUrl(parent, child);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsUtils, IsAncestorUrl_NotAncestor)
{
    QUrl url1 = QUrl::fromLocalFile("/tmp/folder1");
    QUrl url2 = QUrl::fromLocalFile("/tmp/folder2");

    bool result = FileOperationsUtils::isAncestorUrl(url1, url2);

    EXPECT_FALSE(result);
}

TEST_F(TestFileOperationsUtils, IsAncestorUrl_DeepNesting)
{
    QUrl ancestor = QUrl::fromLocalFile("/tmp/root/a");
    QUrl descendant = QUrl::fromLocalFile("/tmp/root/a/b");

    bool result = FileOperationsUtils::isAncestorUrl(ancestor, descendant);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsUtils, IsAncestorUrl_SameUrl)
{
    QUrl url = QUrl::fromLocalFile("/tmp/same");

    bool result = FileOperationsUtils::isAncestorUrl(url, url);

    EXPECT_FALSE(result);
}

// ========== FileOperationsUtils::isFileOnDisk() Tests ==========

TEST_F(TestFileOperationsUtils, IsFileOnDisk_LocalFile)
{
    QUrl file = createTestFile("ondisk.txt");

    bool result = FileOperationsUtils::isFileOnDisk(file);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsUtils, IsFileOnDisk_InvalidUrl)
{
    QUrl invalidUrl;

    bool result = FileOperationsUtils::isFileOnDisk(invalidUrl);

    EXPECT_FALSE(result);
}

TEST_F(TestFileOperationsUtils, IsFileOnDisk_NetworkFile)
{
    QUrl file = QUrl("smb://server/share/file.txt");

    bool result = FileOperationsUtils::isFileOnDisk(file);

    // Network files may or may not be on disk depending on system configuration
    SUCCEED();
}

// ========== FileOperationsUtils::bigFileSize() Tests ==========

TEST_F(TestFileOperationsUtils, BigFileSize_ReturnsValue)
{
    qint64 size = FileOperationsUtils::bigFileSize();

    EXPECT_GT(size, 0);  // Should return a positive size
}

// ========== FileOperationsUtils::blockSync() Tests ==========

TEST_F(TestFileOperationsUtils, BlockSync_ReturnsValue)
{
    bool result = FileOperationsUtils::blockSync();

    // Result depends on configuration, just verify it doesn't crash
    SUCCEED();
}

// ========== FileOperationsUtils::parentUrl() Tests ==========

TEST_F(TestFileOperationsUtils, ParentUrl_HasParent)
{
    QUrl child = QUrl::fromLocalFile("/tmp/subdir/child.txt");

    QUrl parent = FileOperationsUtils::parentUrl(child);

    EXPECT_TRUE(parent.isValid());
    EXPECT_EQ(parent.path(), "/tmp/subdir");
}

TEST_F(TestFileOperationsUtils, ParentUrl_RootDirectory)
{
    QUrl root = QUrl::fromLocalFile("/");

    QUrl parent = FileOperationsUtils::parentUrl(root);

    // Root has no parent
    EXPECT_FALSE(parent.isValid());
}

TEST_F(TestFileOperationsUtils, ParentUrl_DeepPath)
{
    QUrl child = QUrl::fromLocalFile("/a/b/c/d/file.txt");

    QUrl parent = FileOperationsUtils::parentUrl(child);

    EXPECT_TRUE(parent.isValid());
    EXPECT_EQ(parent.path(), "/a/b/c/d");
}

TEST_F(TestFileOperationsUtils, ParentUrl_TrailingSlash)
{
    QUrl child = QUrl::fromLocalFile("/tmp/dir/");

    QUrl parent = FileOperationsUtils::parentUrl(child);

    EXPECT_TRUE(parent.isValid());
}

// ========== FileOperationsUtils::canBroadcastPaste() Tests ==========

TEST_F(TestFileOperationsUtils, CanBroadcastPaste_ReturnsValue)
{
    bool result = FileOperationsUtils::canBroadcastPaste();

    // Result depends on configuration
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
    for (int i = 0; i < 100; ++i) {
        files << createTestFile(QString("file%1.txt").arg(i), 100);
    }

    SizeInfoPointer sizeInfo = FileOperationsUtils::statisticsFilesSize(files);

    ASSERT_NE(sizeInfo, nullptr);
    EXPECT_EQ(sizeInfo->fileCount, 100);
}

TEST_F(TestFileOperationsUtils, EdgeCase_UnicodeUrls)
{
    QUrl parent = QUrl::fromLocalFile(tempDir->path() + "/父目录");
    QDir().mkpath(parent.path());

    QUrl child = QUrl::fromLocalFile(parent.path() + "/子文件.txt");
    QFile file(child.path());
    file.open(QIODevice::WriteOnly);
    file.write("test");
    file.close();

    bool result = FileOperationsUtils::isAncestorUrl(parent, child);

    EXPECT_TRUE(result);
}

TEST_F(TestFileOperationsUtils, EdgeCase_StatisticFilesSize_NonExistentFile)
{
    QUrl nonExistent = QUrl::fromLocalFile("/tmp/does_not_exist_12345.txt");
    QList<QUrl> files = { nonExistent };

    SizeInfoPointer sizeInfo = FileOperationsUtils::statisticsFilesSize(files);

    ASSERT_NE(sizeInfo, nullptr);
    // Should handle gracefully even if file doesn't exist
}

TEST_F(TestFileOperationsUtils, EdgeCase_IsFilesSizeOutLimit_InvalidUrl)
{
    QUrl invalid = QUrl::fromLocalFile("/tmp/nonexistent_file_xyz.txt");

    bool result = FileOperationsUtils::isFilesSizeOutLimit(invalid, 1024);

    // Should handle invalid URLs gracefully
    SUCCEED();
}

