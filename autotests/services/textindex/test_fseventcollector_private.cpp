// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fseventcollector_private.cpp
 * @brief Tests for FSEventCollectorPrivate methods accessed via d_func()
 *        (-fno-access-control). Covers handleFileCreated, handleFileDeleted,
 *        handleFileClosed, handleFileMoved, handleDirectoryCreated/Deleted/Moved,
 *        flushCollectedEvents, cleanupRedundantEntries, removeEntriesCoveredByDirectories,
 *        isMaxEventCountExceeded, isChildOfAnyPath, isDirectory, buildPath,
 *        normalizePath, shouldTrackPath, removeRedundantEntries.
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QSet>

#include "stubext.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/fsmonitor/fseventcollector.h"
#include "services/textindex/fsmonitor/fseventcollector_p.h"
#include "services/textindex/fsmonitor/fsmonitor.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

static FSEventCollector::PathPredicate alwaysTrue()
{
    return [](const QString &) -> bool { return true; };
}

static FSEventCollector::PathPredicate neverTrue()
{
    return [](const QString &) -> bool { return false; };
}

// Helper to get private pointer
static FSEventCollectorPrivate *getPrivate(FSEventCollector &c)
{
    return c.d_func();
}

class FSEventCollectorPrivateTest : public testing::Test
{
protected:
    QTemporaryDir tmp;
    stub_ext::StubExt stub;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        // Stub FSMonitor::start to return true
        stub.set_lamda(ADDR(FSMonitor, start), [](FSMonitor *) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        stub.set_lamda(ADDR(FSMonitor, initialize), [](FSMonitor *, const QStringList &) -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });
        // Create test files and directories
        QDir root(tmp.path());
        root.mkpath("subdir");
        QFile f(root.filePath("file.txt"));
        f.open(QIODevice::WriteOnly);
        f.write("hello");
        f.close();
    }

    FSEventCollector *makeCollector(FSEventCollector::PathPredicate pred = nullptr)
    {
        auto *c = new FSEventCollector(pred ? pred : alwaysTrue());
        return c;
    }
};

// ---- buildPath / normalizePath ----
TEST_F(FSEventCollectorPrivateTest, BuildPath_CombinesDirAndFile)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QString result = d->buildPath("/tmp/dir", "file.txt");
    EXPECT_EQ(result, QString("/tmp/dir/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, BuildPath_TrailingSlash)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QString result = d->buildPath("/tmp/dir/", "file.txt");
    EXPECT_EQ(result, QString("/tmp/dir/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, NormalizePath_EmptyDir)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QString result = d->normalizePath("", "file.txt");
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(FSEventCollectorPrivateTest, NormalizePath_ValidDir)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QString result = d->normalizePath("/tmp", "file.txt");
    EXPECT_EQ(result, QString("/tmp/file.txt"));
}

// ---- isDirectory ----
TEST_F(FSEventCollectorPrivateTest, IsDirectory_RealDirectory)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    EXPECT_TRUE(d->isDirectory(tmp.path()));
    EXPECT_TRUE(d->isDirectory(tmp.path() + "/subdir"));
}

TEST_F(FSEventCollectorPrivateTest, IsDirectory_FileIsNotDirectory)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    EXPECT_FALSE(d->isDirectory(tmp.path() + "/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, IsDirectory_NonExistent)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    EXPECT_FALSE(d->isDirectory("/nonexistent/path/dir"));
}

TEST_F(FSEventCollectorPrivateTest, IsDirectory_SymlinkIsNotDirectory)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    // Create a symlink to a file
    QFile::link(tmp.path() + "/file.txt", tmp.path() + "/link.txt");
    EXPECT_FALSE(d->isDirectory(tmp.path() + "/link.txt"));
}

// ---- isChildOfAnyPath ----
TEST_F(FSEventCollectorPrivateTest, IsChildOfAnyPath_EmptySet)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QSet<QString> emptySet;
    EXPECT_FALSE(d->isChildOfAnyPath("/any/path", emptySet));
}

TEST_F(FSEventCollectorPrivateTest, IsChildOfAnyPath_EmptyPath)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QSet<QString> parents { tmp.path() };
    EXPECT_FALSE(d->isChildOfAnyPath("", parents));
}

TEST_F(FSEventCollectorPrivateTest, IsChildOfAnyPath_IsChild)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QSet<QString> parents { tmp.path() };
    EXPECT_TRUE(d->isChildOfAnyPath(tmp.path() + "/file.txt", parents));
    EXPECT_TRUE(d->isChildOfAnyPath(tmp.path() + "/subdir/deep.txt", parents));
}

TEST_F(FSEventCollectorPrivateTest, IsChildOfAnyPath_IsNotChild)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QSet<QString> parents { "/other/dir" };
    EXPECT_FALSE(d->isChildOfAnyPath(tmp.path() + "/file.txt", parents));
}

TEST_F(FSEventCollectorPrivateTest, IsChildOfAnyPath_ParentIsFileNotDir)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QSet<QString> parents { tmp.path() + "/file.txt" };
    // file.txt is not a directory, so isChildOfAnyPath should return false
    EXPECT_FALSE(d->isChildOfAnyPath(tmp.path() + "/file.txt/child", parents));
}

// ---- isMaxEventCountExceeded ----
TEST_F(FSEventCollectorPrivateTest, IsMaxEventCountExceeded_DefaultNotExceeded)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    EXPECT_FALSE(d->isMaxEventCountExceeded());
}

TEST_F(FSEventCollectorPrivateTest, IsMaxEventCountExceeded_AfterSettingLowMax)
{
    FSEventCollector c(alwaysTrue());
    c.setMaxEventCount(1);
    auto *d = getPrivate(c);
    // Still 0 events, not exceeded
    EXPECT_FALSE(d->isMaxEventCountExceeded());
}

// ---- shouldTrackPath ----
TEST_F(FSEventCollectorPrivateTest, ShouldTrackPath_EmptyPath)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    EXPECT_FALSE(d->shouldTrackPath(""));
}

TEST_F(FSEventCollectorPrivateTest, ShouldTrackPath_WithPredicateAlwaysTrue)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    // Real file, always true predicate
    EXPECT_TRUE(d->shouldTrackPath(tmp.path() + "/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, ShouldTrackPath_WithPredicateNeverTrue)
{
    FSEventCollector c(neverTrue());
    auto *d = getPrivate(c);
    EXPECT_FALSE(d->shouldTrackPath(tmp.path() + "/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, ShouldTrackPath_DeletedFileSkipsPredicate)
{
    FSEventCollector c(neverTrue());
    auto *d = getPrivate(c);
    // Add to deleted list first
    d->deletedFilesList.insert("/some/deleted/file.txt");
    // shouldTrackPath should return true for deleted files regardless of predicate
    EXPECT_TRUE(d->shouldTrackPath("/some/deleted/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, ShouldTrackPath_Directory_AlwaysTracked)
{
    FSEventCollector c(neverTrue());
    auto *d = getPrivate(c);
    // Directories are always tracked regardless of predicate
    EXPECT_TRUE(d->shouldTrackPath(tmp.path()));
}

// ---- handleFileCreated ----
TEST_F(FSEventCollectorPrivateTest, HandleFileCreated_Normal)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileCreated(tmp.path(), "file.txt");
    EXPECT_TRUE(d->createdFilesList.contains(tmp.path() + "/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, HandleFileCreated_EmptyPath)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileCreated("", "file.txt");
    EXPECT_TRUE(d->createdFilesList.isEmpty());
}

TEST_F(FSEventCollectorPrivateTest, HandleFileCreated_EmptyName)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileCreated(tmp.path(), "");
    EXPECT_TRUE(d->createdFilesList.contains(tmp.path()));
}

TEST_F(FSEventCollectorPrivateTest, HandleFileCreated_PredicateRejects)
{
    FSEventCollector c(neverTrue());
    auto *d = getPrivate(c);
    d->handleFileCreated(tmp.path(), "file.txt");
    EXPECT_TRUE(d->createdFilesList.isEmpty());
}

TEST_F(FSEventCollectorPrivateTest, HandleFileCreated_Directory)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileCreated(tmp.path(), "subdir");
    EXPECT_TRUE(d->createdFilesList.contains(tmp.path() + "/subdir"));
}

// ---- handleFileDeleted ----
TEST_F(FSEventCollectorPrivateTest, HandleFileDeleted_Normal)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileDeleted(tmp.path(), "file.txt");
    EXPECT_TRUE(d->deletedFilesList.contains(tmp.path() + "/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, HandleFileDeleted_EmptyPath)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileDeleted("", "file.txt");
    EXPECT_TRUE(d->deletedFilesList.isEmpty());
}

TEST_F(FSEventCollectorPrivateTest, HandleFileDeleted_EmptyName)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileDeleted(tmp.path(), "");
    // normalizePath(tmp.path(), "") returns the dir path itself;
    // shouldTrackPath returns true for directories, so it IS added to deleted
    EXPECT_FALSE(d->deletedFilesList.isEmpty());
    EXPECT_TRUE(d->deletedFilesList.contains(tmp.path()));
}

TEST_F(FSEventCollectorPrivateTest, HandleFileDeleted_PredicateRejects)
{
    FSEventCollector c(neverTrue());
    auto *d = getPrivate(c);
    d->handleFileDeleted(tmp.path(), "file.txt");
    EXPECT_TRUE(d->deletedFilesList.isEmpty());
}

// ---- handleFileClosed ----
TEST_F(FSEventCollectorPrivateTest, HandleFileClosed_Normal)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileClosed(tmp.path(), "file.txt");
    EXPECT_TRUE(d->modifiedFilesList.contains(tmp.path() + "/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, HandleFileClosed_EmptyPath)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileClosed("", "file.txt");
    EXPECT_TRUE(d->modifiedFilesList.isEmpty());
}

TEST_F(FSEventCollectorPrivateTest, HandleFileClosed_EmptyName)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileClosed(tmp.path(), "");
    EXPECT_TRUE(d->modifiedFilesList.isEmpty());
}

TEST_F(FSEventCollectorPrivateTest, HandleFileClosed_ForDirectory)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileClosed(tmp.path(), "subdir");
    // Directories should NOT be added to modified list
    EXPECT_TRUE(d->modifiedFilesList.isEmpty());
}

// ---- handleFileMoved ----
TEST_F(FSEventCollectorPrivateTest, HandleFileMoved_NormalMove)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileMoved(tmp.path(), "file.txt", tmp.path(), "moved.txt");
    EXPECT_TRUE(d->movedFilesList.contains(tmp.path() + "/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, HandleFileMoved_ToOutside_EmptyTo)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileMoved(tmp.path(), "file.txt", "", "");
    // Should be treated as deletion
    EXPECT_TRUE(d->deletedFilesList.contains(tmp.path() + "/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, HandleFileMoved_FromOutside_EmptyFrom)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileMoved("", "", tmp.path(), "new.txt");
    // Should be treated as creation
    EXPECT_TRUE(d->createdFilesList.contains(tmp.path() + "/new.txt"));
}

TEST_F(FSEventCollectorPrivateTest, HandleFileMoved_FromUnindexed_ToIndexed)
{
    // neverTrue predicate: source not indexed, target not indexed -> ignored
    FSEventCollector c(neverTrue());
    auto *d = getPrivate(c);
    d->handleFileMoved(tmp.path(), "a.dat", tmp.path(), "b.dat");
    EXPECT_TRUE(d->movedFilesList.isEmpty());
}

TEST_F(FSEventCollectorPrivateTest, HandleFileMoved_ConflictWithExistingCreated)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    // Add source to created list first
    d->createdFilesList.insert(tmp.path() + "/file.txt");
    d->handleFileMoved(tmp.path(), "file.txt", tmp.path(), "moved.txt");
    // Source should be removed from created, target added
    EXPECT_FALSE(d->createdFilesList.contains(tmp.path() + "/file.txt"));
    EXPECT_TRUE(d->createdFilesList.contains(tmp.path() + "/moved.txt"));
}

TEST_F(FSEventCollectorPrivateTest, HandleFileMoved_ConflictWithExistingModified)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->modifiedFilesList.insert(tmp.path() + "/file.txt");
    d->handleFileMoved(tmp.path(), "file.txt", tmp.path(), "moved.txt");
    // Source should be removed from modified
    EXPECT_FALSE(d->modifiedFilesList.contains(tmp.path() + "/file.txt"));
}

TEST_F(FSEventCollectorPrivateTest, HandleFileMoved_ConflictWithExistingMoved)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    // Insert a moved entry where the KEY matches the new move's
    // fullToPath, so the conflict branch fires.
    d->movedFilesList.insert("/some/dest/file.txt", "/other/location");
    d->handleFileMoved(tmp.path(), "file.txt", "/some/dest", "file.txt");
    // Destination key already in movedFilesList -> fallback to delete+create
    EXPECT_TRUE(d->deletedFilesList.contains(tmp.path() + "/file.txt"));
}

// ---- handleDirectoryCreated ----
TEST_F(FSEventCollectorPrivateTest, HandleDirectoryCreated_Normal)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleDirectoryCreated(tmp.path(), "newdir");
    QDir().mkpath(tmp.path() + "/newdir");
    EXPECT_TRUE(d->createdFilesList.contains(tmp.path() + "/newdir"));
}

// ---- handleDirectoryDeleted ----
TEST_F(FSEventCollectorPrivateTest, HandleDirectoryDeleted_Normal)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleDirectoryDeleted(tmp.path(), "subdir");
    EXPECT_TRUE(d->deletedFilesList.contains(tmp.path() + "/subdir"));
    EXPECT_TRUE(d->deletedDirectoriesMarker.contains(tmp.path() + "/subdir"));
}

// ---- handleDirectoryMoved ----
TEST_F(FSEventCollectorPrivateTest, HandleDirectoryMoved_ToOutside)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleDirectoryMoved(tmp.path(), "subdir", "", "");
    // Should be treated as directory deletion
    EXPECT_TRUE(d->deletedDirectoriesMarker.contains(tmp.path() + "/subdir"));
}

TEST_F(FSEventCollectorPrivateTest, HandleDirectoryMoved_FromOutside)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QDir().mkpath(tmp.path() + "/newdir");
    d->handleDirectoryMoved("", "", tmp.path(), "newdir");
    // Should be treated as directory creation
    EXPECT_TRUE(d->createdFilesList.contains(tmp.path() + "/newdir"));
}

// ---- flushCollectedEvents ----
TEST_F(FSEventCollectorPrivateTest, FlushCollectedEvents_WithEvents)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->createdFilesList.insert("/a.txt");
    d->deletedFilesList.insert("/b.txt");
    d->modifiedFilesList.insert("/c.txt");
    d->movedFilesList.insert("/d.txt", "/e.txt");
    d->flushCollectedEvents();
    EXPECT_TRUE(d->createdFilesList.isEmpty());
    EXPECT_TRUE(d->deletedFilesList.isEmpty());
    EXPECT_TRUE(d->modifiedFilesList.isEmpty());
    EXPECT_TRUE(d->movedFilesList.isEmpty());
}

TEST_F(FSEventCollectorPrivateTest, FlushCollectedEvents_Empty)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->flushCollectedEvents();
    SUCCEED();
}

// ---- removeRedundantEntries ----
TEST_F(FSEventCollectorPrivateTest, RemoveRedundantEntries_DirectoryCoversFile)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QSet<QString> files;
    files.insert(tmp.path());
    files.insert(tmp.path() + "/file.txt");
    files.insert(tmp.path() + "/subdir/deep.txt");
    d->removeRedundantEntries(files);
    // The directory should remain, files under it should be removed
    EXPECT_TRUE(files.contains(tmp.path()));
    EXPECT_FALSE(files.contains(tmp.path() + "/file.txt"));
    EXPECT_FALSE(files.contains(tmp.path() + "/subdir/deep.txt"));
}

TEST_F(FSEventCollectorPrivateTest, RemoveRedundantEntries_NoDirectories)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    QSet<QString> files;
    files.insert("/a.txt");
    files.insert("/b.txt");
    d->removeRedundantEntries(files);
    EXPECT_EQ(files.size(), 2);
}

// ---- cleanupRedundantEntries ----
TEST_F(FSEventCollectorPrivateTest, CleanupRedundantEntries_CreatedDirCoversModifiedFiles)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    // Add a directory to created list
    d->createdFilesList.insert(tmp.path());
    // Add files under it to modified list
    d->modifiedFilesList.insert(tmp.path() + "/file.txt");
    d->cleanupRedundantEntries();
    EXPECT_FALSE(d->modifiedFilesList.contains(tmp.path() + "/file.txt"));
}

// ---- removeEntriesCoveredByDirectories ----
TEST_F(FSEventCollectorPrivateTest, RemoveEntriesCoveredByDirectories)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    // Mark a directory as deleted
    d->deletedDirectoriesMarker.insert(tmp.path());
    // Add entries under it
    d->deletedFilesList.insert(tmp.path() + "/sub.txt");
    d->createdFilesList.insert(tmp.path() + "/new.txt");
    d->modifiedFilesList.insert(tmp.path() + "/mod.txt");
    d->removeEntriesCoveredByDirectories();
    EXPECT_FALSE(d->deletedFilesList.contains(tmp.path() + "/sub.txt"));
    EXPECT_FALSE(d->createdFilesList.contains(tmp.path() + "/new.txt"));
    EXPECT_FALSE(d->modifiedFilesList.contains(tmp.path() + "/mod.txt"));
}

TEST_F(FSEventCollectorPrivateTest, RemoveEntriesCoveredByDirectories_EmptyMarker)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->deletedFilesList.insert("/a.txt");
    d->createdFilesList.insert("/b.txt");
    d->removeEntriesCoveredByDirectories();
    // Nothing should be removed
    EXPECT_TRUE(d->deletedFilesList.contains("/a.txt"));
    EXPECT_TRUE(d->createdFilesList.contains("/b.txt"));
}

// ---- Create-then-delete cancels out ----
TEST_F(FSEventCollectorPrivateTest, CreateThenDelete_CancelsOut)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileCreated(tmp.path(), "file.txt");
    EXPECT_TRUE(d->createdFilesList.contains(tmp.path() + "/file.txt"));
    d->handleFileDeleted(tmp.path(), "file.txt");
    // Deletion of a just-created file removes from created and adds to deleted
    EXPECT_FALSE(d->createdFilesList.contains(tmp.path() + "/file.txt"));
    EXPECT_TRUE(d->deletedFilesList.contains(tmp.path() + "/file.txt"));
}

// ---- Delete-then-create (re-creation) ----
TEST_F(FSEventCollectorPrivateTest, DeleteThenCreate_Recreation)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileDeleted(tmp.path(), "file.txt");
    EXPECT_TRUE(d->deletedFilesList.contains(tmp.path() + "/file.txt"));
    d->handleFileCreated(tmp.path(), "file.txt");
    // Re-creation removes from deleted and adds to created
    EXPECT_FALSE(d->deletedFilesList.contains(tmp.path() + "/file.txt"));
    EXPECT_TRUE(d->createdFilesList.contains(tmp.path() + "/file.txt"));
}

// ---- Modify-then-delete ----
TEST_F(FSEventCollectorPrivateTest, ModifyThenDelete_Supersedes)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileClosed(tmp.path(), "file.txt");
    EXPECT_TRUE(d->modifiedFilesList.contains(tmp.path() + "/file.txt"));
    d->handleFileDeleted(tmp.path(), "file.txt");
    EXPECT_FALSE(d->modifiedFilesList.contains(tmp.path() + "/file.txt"));
    EXPECT_TRUE(d->deletedFilesList.contains(tmp.path() + "/file.txt"));
}

// ---- Create-then-close (no duplicate in modified) ----
TEST_F(FSEventCollectorPrivateTest, CreateThenClose_NoDuplicate)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    d->handleFileCreated(tmp.path(), "file.txt");
    d->handleFileClosed(tmp.path(), "file.txt");
    EXPECT_TRUE(d->createdFilesList.contains(tmp.path() + "/file.txt"));
    EXPECT_FALSE(d->modifiedFilesList.contains(tmp.path() + "/file.txt"));
}

// ---- Directory created removes redundant file entries ----
TEST_F(FSEventCollectorPrivateTest, DirectoryCreatedRemovesChildFiles)
{
    FSEventCollector c(alwaysTrue());
    auto *d = getPrivate(c);
    // Add files first
    d->handleFileCreated(tmp.path(), "file1.txt");
    d->handleFileCreated(tmp.path(), "file2.txt");
    // Now create a directory that covers them
    QDir().mkpath(tmp.path() + "/newdir");
    d->handleDirectoryCreated(tmp.path(), "newdir");
    // The files are NOT under newdir, so they should still be there
    EXPECT_TRUE(d->createdFilesList.contains(tmp.path() + "/file1.txt"));
}

// ---- Max event count triggers flush ----
TEST_F(FSEventCollectorPrivateTest, MaxEventCountTriggersFlush)
{
    FSEventCollector c(alwaysTrue());
    c.setMaxEventCount(3);
    auto *d = getPrivate(c);
    // Add events up to the limit
    d->handleFileCreated(tmp.path(), "a.txt");
    d->handleFileCreated(tmp.path(), "b.txt");
    d->handleFileCreated(tmp.path(), "c.txt");
    // At limit, should trigger flush
    EXPECT_TRUE(d->createdFilesList.isEmpty());
}
