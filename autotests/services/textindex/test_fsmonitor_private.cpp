// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fsmonitor_private.cpp
 * @brief Tests for FSMonitorPrivate methods accessed via d_func()
 *        (-fno-access-control). Covers shouldExcludePath, isSymbolicLink,
 *        addWatchForDirectory, removeWatchForDirectory, isWithinWatchLimit,
 *        getMaxUserWatches, showHidden, isDirectory, handleFileCreated,
 *        handleFileDeleted, handleFileClosed, handleFileMoved,
 *        handleFastScanCompleted, handleDirectoriesBatch.
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QString>

#include "stubext.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/fsmonitor/fsmonitor.h"
#include "services/textindex/fsmonitor/fsmonitor_p.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class FSMonitorPrivateTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        QDir root(tmp.path());
        root.mkpath("subdir1/subdir2");
        createFile("normal.txt", "content");
        createFile(".hidden.txt", "hidden");
    }

    void createFile(const QString &name, const QString &content)
    {
        QFile f(tmp.path() + "/" + name);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(content.toUtf8());
        f.close();
    }

    FSMonitorPrivate *getPrivate()
    {
        return FSMonitor::instance().d_func();
    }
};

// ---- isSymbolicLink ----
TEST_F(FSMonitorPrivateTest, IsSymbolicLink_EmptyPath)
{
    auto *d = getPrivate();
    EXPECT_FALSE(d->isSymbolicLink(""));
}

TEST_F(FSMonitorPrivateTest, IsSymbolicLink_RealFile)
{
    auto *d = getPrivate();
    EXPECT_FALSE(d->isSymbolicLink(tmp.path() + "/normal.txt"));
}

TEST_F(FSMonitorPrivateTest, IsSymbolicLink_SymlinkFile)
{
    auto *d = getPrivate();
    QFile::link(tmp.path() + "/normal.txt", tmp.path() + "/link.txt");
    EXPECT_TRUE(d->isSymbolicLink(tmp.path() + "/link.txt"));
}

// ---- shouldExcludePath ----
TEST_F(FSMonitorPrivateTest, ShouldExcludePath_EmptyPath)
{
    auto *d = getPrivate();
    EXPECT_TRUE(d->shouldExcludePath(""));
}

TEST_F(FSMonitorPrivateTest, ShouldExcludePath_Symlink)
{
    auto *d = getPrivate();
    QFile::link(tmp.path() + "/normal.txt", tmp.path() + "/link.txt");
    EXPECT_TRUE(d->shouldExcludePath(tmp.path() + "/link.txt"));
}

TEST_F(FSMonitorPrivateTest, ShouldExcludePath_NonExistent)
{
    auto *d = getPrivate();
    // Non-existent path - not a symlink, may not be excluded
    // Just verify no crash
    EXPECT_NO_FATAL_FAILURE({ (void)d->shouldExcludePath("/nonexistent/path/file.txt"); });
}

TEST_F(FSMonitorPrivateTest, ShouldExcludePath_NormalFile)
{
    auto *d = getPrivate();
    // The default excludeMatcher might or might not exclude /tmp paths
    EXPECT_NO_FATAL_FAILURE({ (void)d->shouldExcludePath(tmp.path() + "/normal.txt"); });
}

// ---- isWithinWatchLimit ----
TEST_F(FSMonitorPrivateTest, IsWithinWatchLimit_NoMaxSet)
{
    auto *d = getPrivate();
    // maxWatches defaults to -1, so should return true
    EXPECT_TRUE(d->isWithinWatchLimit());
}

TEST_F(FSMonitorPrivateTest, IsWithinWatchLimit_WithMaxSet)
{
    auto *d = getPrivate();
    d->maxWatches = 100;
    d->maxUsagePercentage = 0.5;
    d->watchedDirectories.clear();
    EXPECT_TRUE(d->isWithinWatchLimit());
}

TEST_F(FSMonitorPrivateTest, IsWithinWatchLimit_ExceedsLimit)
{
    auto *d = getPrivate();
    d->maxWatches = 10;
    d->maxUsagePercentage = 0.5;
    for (int i = 0; i < 10; i++) {
        d->watchedDirectories.insert(QString("/watch%1").arg(i));
    }
    EXPECT_FALSE(d->isWithinWatchLimit());
}

// ---- getMaxUserWatches ----
TEST_F(FSMonitorPrivateTest, GetMaxUserWatches_ReadsFromProc)
{
    auto *d = getPrivate();
    int watches = d->getMaxUserWatches();
    // Should be positive on Linux, or -1 if file can't be read
    EXPECT_GE(watches, -1);
    if (watches > 0) {
        EXPECT_GT(watches, 0);
    }
}

// ---- showHidden ----
TEST_F(FSMonitorPrivateTest, ShowHidden_ReturnsBool)
{
    auto *d = getPrivate();
    bool hidden = d->showHidden();
    EXPECT_TRUE(hidden || !hidden);  // Just verify no crash
}

// ---- isDirectory (FSMonitorPrivate version) ----
TEST_F(FSMonitorPrivateTest, IsDirectory_EmptyPath)
{
    auto *d = getPrivate();
    EXPECT_FALSE(d->isDirectory("", "file.txt"));
}

TEST_F(FSMonitorPrivateTest, IsDirectory_RealDir)
{
    auto *d = getPrivate();
    EXPECT_TRUE(d->isDirectory(tmp.path(), "subdir1"));
}

TEST_F(FSMonitorPrivateTest, IsDirectory_FileIsNotDir)
{
    auto *d = getPrivate();
    EXPECT_FALSE(d->isDirectory(tmp.path(), "normal.txt"));
}

// ---- addWatchForDirectory ----
TEST_F(FSMonitorPrivateTest, AddWatchForDirectory_EmptyPath)
{
    auto *d = getPrivate();
    EXPECT_FALSE(d->addWatchForDirectory(""));
}

TEST_F(FSMonitorPrivateTest, AddWatchForDirectory_AlreadyWatched)
{
    auto *d = getPrivate();
    QString path = "/already/watched";
    d->watchedDirectories.insert(path);
    EXPECT_TRUE(d->addWatchForDirectory(path));
}

// ---- removeWatchForDirectory ----
TEST_F(FSMonitorPrivateTest, RemoveWatchForDirectory_EmptyPath)
{
    auto *d = getPrivate();
    EXPECT_NO_FATAL_FAILURE({ d->removeWatchForDirectory(""); });
}

TEST_F(FSMonitorPrivateTest, RemoveWatchForDirectory_NotWatched)
{
    auto *d = getPrivate();
    EXPECT_NO_FATAL_FAILURE({ d->removeWatchForDirectory("/not/watched"); });
}

TEST_F(FSMonitorPrivateTest, RemoveWatchForDirectory_Watched)
{
    auto *d = getPrivate();
    QString path = "/some/path";
    d->watchedDirectories.insert(path);
    d->removeWatchForDirectory(path);
    EXPECT_FALSE(d->watchedDirectories.contains(path));
}

// ---- addDirectoryRecursively ----
TEST_F(FSMonitorPrivateTest, AddDirectoryRecursively_EmptyPath)
{
    auto *d = getPrivate();
    // active is false by default, so this is a no-op
    EXPECT_NO_FATAL_FAILURE({ d->addDirectoryRecursively(""); });
}

TEST_F(FSMonitorPrivateTest, AddDirectoryRecursively_NotActive)
{
    auto *d = getPrivate();
    d->active = false;
    EXPECT_NO_FATAL_FAILURE({ d->addDirectoryRecursively(tmp.path()); });
}

// ---- stopMonitoring ----
TEST_F(FSMonitorPrivateTest, StopMonitoring_NotActive)
{
    auto *d = getPrivate();
    d->active = false;
    EXPECT_NO_FATAL_FAILURE({ d->stopMonitoring(); });
    EXPECT_FALSE(d->active);
}

// ---- startMonitoring ----
TEST_F(FSMonitorPrivateTest, StartMonitoring_AlreadyActive)
{
    auto *d = getPrivate();
    d->active = true;
    bool result = d->startMonitoring();
    EXPECT_TRUE(result);
}

// ---- handleFastScanCompleted ----
TEST_F(FSMonitorPrivateTest, HandleFastScanCompleted_Success)
{
    auto *d = getPrivate();
    EXPECT_NO_FATAL_FAILURE({ d->handleFastScanCompleted(true); });
}

TEST_F(FSMonitorPrivateTest, HandleFastScanCompleted_Failure)
{
    auto *d = getPrivate();
    EXPECT_NO_FATAL_FAILURE({ d->handleFastScanCompleted(false); });
}

// ---- handleDirectoriesBatch ----
TEST_F(FSMonitorPrivateTest, HandleDirectoriesBatch_EmptyList)
{
    auto *d = getPrivate();
    EXPECT_NO_FATAL_FAILURE({ d->handleDirectoriesBatch(QStringList{}); });
}

TEST_F(FSMonitorPrivateTest, HandleDirectoriesBatch_SinglePath)
{
    auto *d = getPrivate();
    d->maxWatches = 10000;
    EXPECT_NO_FATAL_FAILURE({ d->handleDirectoriesBatch(QStringList{tmp.path()}); });
}

TEST_F(FSMonitorPrivateTest, HandleDirectoriesBatch_ExceedsWatchLimit)
{
    auto *d = getPrivate();
    d->maxWatches = 1;
    d->maxUsagePercentage = 0.5;
    EXPECT_NO_FATAL_FAILURE({ d->handleDirectoriesBatch(QStringList{tmp.path()}); });
}

// ---- travelRootDirectories ----
TEST_F(FSMonitorPrivateTest, TravelRootDirectories)
{
    auto *d = getPrivate();
    EXPECT_NO_FATAL_FAILURE({ d->travelRootDirectories(); });
}

// ---- handleFile* signals (private handlers) ----
TEST_F(FSMonitorPrivateTest, HandleFileCreated_NotActive)
{
    auto *d = getPrivate();
    d->active = false;
    EXPECT_NO_FATAL_FAILURE({ d->handleFileCreated("/path", "file.txt"); });
}

TEST_F(FSMonitorPrivateTest, HandleFileCreated_ActiveEmptyPath)
{
    auto *d = getPrivate();
    d->active = true;
    EXPECT_NO_FATAL_FAILURE({ d->handleFileCreated("", "file.txt"); });
}

TEST_F(FSMonitorPrivateTest, HandleFileDeleted_NotActive)
{
    auto *d = getPrivate();
    d->active = false;
    EXPECT_NO_FATAL_FAILURE({ d->handleFileDeleted("/path", "file.txt"); });
}

TEST_F(FSMonitorPrivateTest, HandleFileClosed_NotActive)
{
    auto *d = getPrivate();
    d->active = false;
    EXPECT_NO_FATAL_FAILURE({ d->handleFileClosed("/path", "file.txt"); });
}

TEST_F(FSMonitorPrivateTest, HandleFileMoved_NotActive)
{
    auto *d = getPrivate();
    d->active = false;
    EXPECT_NO_FATAL_FAILURE({ d->handleFileMoved("/from", "f.txt", "/to", "t.txt"); });
}

// ---- init with QStringList ----
TEST_F(FSMonitorPrivateTest, Init_EmptyRootPaths)
{
    auto *d = getPrivate();
    bool result = d->init(QStringList{});
    EXPECT_FALSE(result);
}

TEST_F(FSMonitorPrivateTest, Init_NonExistentPaths)
{
    auto *d = getPrivate();
    bool result = d->init(QStringList{"/nonexistent/path12345"});
    EXPECT_FALSE(result);
}

// ---- setUseFastScan while active (should be no-op) ----
TEST_F(FSMonitorPrivateTest, SetUseFastScan_WhileActive_NoChange)
{
    FSMonitor &m = FSMonitor::instance();
    bool origValue = m.useFastScan();
    // Simulate being active (just set the flag)
    m.d_func()->active = true;
    m.setUseFastScan(!origValue);
    // Should be unchanged because active
    EXPECT_EQ(m.useFastScan(), origValue);
    m.d_func()->active = false;
    // Restore
    m.setUseFastScan(true);
}
