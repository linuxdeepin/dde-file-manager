// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mimetypedisplaymanager.cpp
 * @brief Unit tests for MimeTypeDisplayManager (mimetypedisplaymanager.cpp)
 *
 * MimeTypeDisplayManager maps MIME type strings to display names, icons and
 * FileType enums. All tested APIs are pure map lookups or QMimeDatabase
 * queries against the local file system — no hardware or network needed.
 * Empty paths are avoided (accurateLocalMimeType Q_ASSERTs on empty).
 */

#include <gtest/gtest.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/interfaces/fileinfo.h>

#include <QString>
#include <QDir>
#include <QTemporaryFile>
#include <QFile>

using namespace dfmbase;

TEST(MimeTypeDisplayManagerTest, InstanceReturnsNonNullSingleton)
{
    auto *a = MimeTypeDisplayManager::instance();
    auto *b = MimeTypeDisplayManager::instance();
    ASSERT_NE(a, nullptr);
    EXPECT_EQ(a, b);
}

TEST(MimeTypeDisplayManagerTest, DisplayNamesMapIsPopulated)
{
    auto *m = MimeTypeDisplayManager::instance();
    auto names = m->displayNames();
    EXPECT_FALSE(names.isEmpty());
    EXPECT_TRUE(names.contains(FileInfo::FileType::kDirectory));
    EXPECT_TRUE(names.contains(FileInfo::FileType::kUnknown));
}

TEST(MimeTypeDisplayManagerTest, DisplayNameForKnownMimesIncludesMimeInDebug)
{
    auto *m = MimeTypeDisplayManager::instance();
    // In QT_DEBUG displayName appends " (<mime>)"; in release it does not.
    // Either way the category prefix must be present.
    QString dirDisplay = m->displayName(QStringLiteral("inode/directory"));
    EXPECT_TRUE(dirDisplay.contains(QStringLiteral("Directory")));

    QString txtDisplay = m->displayName(QStringLiteral("text/plain"));
    EXPECT_TRUE(txtDisplay.contains(QStringLiteral("Text")));
}

TEST(MimeTypeDisplayManagerTest, FullMimeNameIncludesMimeSuffix)
{
    auto *m = MimeTypeDisplayManager::instance();
    QString full = m->fullMimeName(QStringLiteral("inode/directory"));
    EXPECT_TRUE(full.contains(QStringLiteral("Directory")));
    EXPECT_TRUE(full.contains(QStringLiteral("inode/directory")));
}

TEST(MimeTypeDisplayManagerTest, DefaultIconForKnownTypes)
{
    auto *m = MimeTypeDisplayManager::instance();
    EXPECT_EQ(m->defaultIcon(QStringLiteral("inode/directory")), QStringLiteral("folder"));
    EXPECT_EQ(m->defaultIcon(QStringLiteral("video/mp4")), QStringLiteral("video"));
    EXPECT_EQ(m->defaultIcon(QStringLiteral("audio/mpeg")), QStringLiteral("music"));
}

TEST(MimeTypeDisplayManagerTest, DisplayNameToEnumResolvesKnownMimes)
{
    auto *m = MimeTypeDisplayManager::instance();
    EXPECT_EQ(m->displayNameToEnum(QStringLiteral("inode/directory")),
              FileInfo::FileType::kDirectory);
    EXPECT_EQ(m->displayNameToEnum(QStringLiteral("application/x-desktop")),
              FileInfo::FileType::kDesktopApplication);
    EXPECT_EQ(m->displayNameToEnum(QStringLiteral("video/mp4")),
              FileInfo::FileType::kVideos);
}

TEST(MimeTypeDisplayManagerTest, SupportMimeListsAreCallableAndNonCrashing)
{
    auto *m = MimeTypeDisplayManager::instance();
    // The lists are loaded from resource/config files; their contents may vary
    // by environment, but the accessors must not crash.
    (void)m->supportArchiveMimetypes();
    (void)m->supportVideoMimeTypes();
    (void)m->supportAudioMimeTypes();
    SUCCEED();
}

TEST(MimeTypeDisplayManagerTest, AccurateDisplayTypeFromPathResolvesTxtFile)
{
    QTemporaryFile tmp(QDir::tempPath() + "/ut_mtdm_XXXXXX.txt");
    ASSERT_TRUE(tmp.open());
    tmp.write("hello");
    tmp.close();
    QString path = tmp.fileName();

    auto *m = MimeTypeDisplayManager::instance();
    QString display = m->accurateDisplayTypeFromPath(path);
    EXPECT_TRUE(display.contains(QStringLiteral("Text")));
    QFile::remove(path);
}
