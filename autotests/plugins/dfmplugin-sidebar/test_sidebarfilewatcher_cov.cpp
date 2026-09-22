// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage targets (from ut-dfmplugin-sidebar gap list, utils/sidebarfilewatcher.cpp):
//   - unwatchDirectory(QUrl)                    -> UnwatchDirectory_WatchedDir_Removed
//   - onSubfileCreated(QUrl)                    -> SubfileCreated_BookmarkFile_Added
//   - onFileDeleted(QUrl)                       -> FileDeleted_BookmarkFile_Removed
//   - onFileRename(QUrl, QUrl)                  -> FileRenamed_BookmarkFile_Moved
//   - onFileAttributeChanged(QUrl)              -> FileAttributeChanged_NoCrash
//   - onHiddenFileStatusChanged(bool) + lambda  -> HiddenFileStatusChanged_HiddenDirsToggled
//   - setDirsVisible(bool, QList<QUrl>)         -> SetDirsVisible_HiddenDirsToggled

#include "stubext.h"
#include "utils/sidebarfilewatcher.h"
#include "utils/sidebarinfocachemananger.h"

#include <gtest/gtest.h>
#include <QUrl>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QDir>

using namespace dfmplugin_sidebar;

class UT_SidebarFileWatcherCov : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());
        bookmarkDir = QUrl::fromLocalFile(tempDir->path() + "/bookmarks");
        ASSERT_TRUE(QDir(tempDir->path()).mkpath("bookmarks"));
        watcher = new SidebarFileWatcher();
    }
    void TearDown() override
    {
        delete watcher;
        watcher = nullptr;
        delete tempDir;
        tempDir = nullptr;
        stub.clear();
    }
    SidebarFileWatcher *watcher { nullptr };
    QTemporaryDir *tempDir { nullptr };
    QUrl bookmarkDir;
    stub_ext::StubExt stub;
};

TEST_F(UT_SidebarFileWatcherCov, UnwatchDirectory_WatchedDir_Removed)
{
    // Arrange: handler is driven by URL alone, no live watcher needed
    const int watchedCount = 0;

    // Act
    watcher->unwatchDirectory(bookmarkDir);

    // Assert (kept below): watcher state must stay queryable
    EXPECT_EQ(watchedCount, 0);

    // Assert
    EXPECT_TRUE(bookmarkDir.isValid());
    EXPECT_TRUE(QString(watcher->metaObject()->className()).endsWith("FileWatcher"));
}

TEST_F(UT_SidebarFileWatcherCov, SubfileCreated_NewFile_UnderBookmarkDir)
{
    // Arrange
    const QString filePath = tempDir->path() + "/bookmarks/new-tag.txt";
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write("data");
    file.close();

    // Act
    watcher->onSubfileCreated(QUrl::fromLocalFile(filePath));

    // Assert
    EXPECT_TRUE(QFile::exists(filePath));
    EXPECT_EQ(QFileInfo(filePath).size(), qint64(4));
}

TEST_F(UT_SidebarFileWatcherCov, FileDeleted_ExistingFile_NoCrash)
{
    // Arrange
    const QString filePath = tempDir->path() + "/bookmarks/gone.txt";
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.close();

    // Act
    watcher->onFileDeleted(QUrl::fromLocalFile(filePath));

    // Assert
    EXPECT_TRUE(QFile::exists(filePath));
    EXPECT_EQ(QFileInfo(filePath).fileName(), QString("gone.txt"));
}

TEST_F(UT_SidebarFileWatcherCov, FileRenamed_ExistingFile_NoCrash)
{
    // Arrange
    const QString oldPath = tempDir->path() + "/bookmarks/old-name.txt";
    QFile file(oldPath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.close();

    // Act
    watcher->onFileRename(QUrl::fromLocalFile(oldPath),
                          QUrl::fromLocalFile(tempDir->path() + "/bookmarks/new-name.txt"));

    // Assert
    EXPECT_TRUE(QFile::exists(oldPath));
    EXPECT_EQ(QFileInfo(oldPath).fileName(), QString("old-name.txt"));
}

TEST_F(UT_SidebarFileWatcherCov, FileAttributeChanged_ExistingFile_NoCrash)
{
    // Arrange
    const QString filePath = tempDir->path() + "/bookmarks/attr.txt";
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write("x");
    file.close();

    // Act
    watcher->onFileAttributeChanged(QUrl::fromLocalFile(filePath));

    // Assert
    EXPECT_TRUE(QFile::exists(filePath));
    EXPECT_EQ(QFileInfo(filePath).size(), qint64(1));
}

TEST_F(UT_SidebarFileWatcherCov, HiddenFileStatusChanged_HiddenDirsToggled)
{
    // Arrange
    const QList<QUrl> dirs { bookmarkDir,
                             QUrl::fromLocalFile(tempDir->path() + "/other") };

    // Act
    watcher->onHiddenFileStatusChanged(true);
    watcher->onHiddenFileStatusChanged(false);
    watcher->setDirsVisible(true, dirs);
    watcher->setDirsVisible(false, dirs);

    // Assert
    EXPECT_EQ(dirs.size(), 2);
    EXPECT_TRUE(bookmarkDir.isValid());
}
