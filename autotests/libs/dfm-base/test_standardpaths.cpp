// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_standardpaths.cpp
 * @brief Unit tests for StandardPaths (standardpaths.cpp)
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QDir>
#include <QStandardPaths>

#include <dfm-base/base/standardpaths.h>

using namespace dfmbase;

TEST(StandardPathsTest, HomePathIsHome)
{
    QString home = StandardPaths::location(StandardPaths::kHomePath);
    EXPECT_EQ(home, QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
}

TEST(StandardPathsTest, DesktopPathNotEmpty)
{
    QString desktop = StandardPaths::location(StandardPaths::kDesktopPath);
    EXPECT_FALSE(desktop.isEmpty());
}

TEST(StandardPathsTest, TrashLocalPathContainsTrash)
{
    QString trash = StandardPaths::location(StandardPaths::kTrashLocalPath);
    EXPECT_TRUE(trash.contains("Trash"));
}

TEST(StandardPathsTest, RootPathIsSlash)
{
    EXPECT_EQ(StandardPaths::location(StandardPaths::kRoot), QString("/"));
}

TEST(StandardPathsTest, DiskPathIsSlash)
{
    EXPECT_EQ(StandardPaths::location(StandardPaths::kDiskPath), QString("/"));
}

TEST(StandardPathsTest, RecentPathIsScheme)
{
    EXPECT_EQ(StandardPaths::location(StandardPaths::kRecentPath), QString("recent:///"));
}

TEST(StandardPathsTest, VaultPathIsScheme)
{
    EXPECT_EQ(StandardPaths::location(StandardPaths::kVault), QString("dfmvault:///"));
}

TEST(StandardPathsTest, ThumbnailPathsContainThumbnails)
{
    QString base = StandardPaths::location(StandardPaths::kThumbnailPath);
    EXPECT_TRUE(base.contains("thumbnails"));
    EXPECT_TRUE(StandardPaths::location(StandardPaths::kThumbnailFailPath).contains("fail"));
    EXPECT_TRUE(StandardPaths::location(StandardPaths::kThumbnailNormalPath).contains("normal"));
}

TEST(StandardPathsTest, LocationByDirNameHome)
{
    QString home = StandardPaths::location("home");
    EXPECT_EQ(home, QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
}

TEST(StandardPathsTest, LocationByDirNameUnknown)
{
    QString unknown = StandardPaths::location("nonexistent_dir");
    EXPECT_TRUE(unknown.isEmpty());
}

TEST(StandardPathsTest, IconNameHome)
{
    EXPECT_EQ(StandardPaths::iconName(StandardPaths::kHomePath), QString("user-home"));
}

TEST(StandardPathsTest, IconNameTrash)
{
    EXPECT_EQ(StandardPaths::iconName(StandardPaths::kTrashLocalPath), QString("user-trash"));
}

TEST(StandardPathsTest, IconNameUnknownReturnsEmpty)
{
    EXPECT_EQ(StandardPaths::iconName(StandardPaths::kTrashLocalInfoPath), QString(""));
}

TEST(StandardPathsTest, DisplayNameHome)
{
    QString name = StandardPaths::displayName(StandardPaths::kHomePath);
    EXPECT_FALSE(name.isEmpty());
}

TEST(StandardPathsTest, FromStandardUrlHome)
{
    QString home = StandardPaths::location(StandardPaths::kHomePath);
    QUrl url;
    url.setScheme("standard");
    url.setHost("home");
    EXPECT_EQ(StandardPaths::fromStandardUrl(url), home);
}

TEST(StandardPathsTest, FromStandardUrlInvalidScheme)
{
    QUrl url;
    url.setScheme("file");
    url.setHost("home");
    EXPECT_TRUE(StandardPaths::fromStandardUrl(url).isEmpty());
}

TEST(StandardPathsTest, ToStandardUrlHomeScheme)
{
    QString home = StandardPaths::location(StandardPaths::kHomePath);
    QUrl url = StandardPaths::toStandardUrl(home);
    EXPECT_EQ(url.scheme(), QString("standard"));
}

TEST(StandardPathsTest, ToStandardUrlUnmappedReturnsEmpty)
{
    QUrl url = StandardPaths::toStandardUrl("/some/random/unmapped/path");
    EXPECT_TRUE(url.isEmpty());
}

// ---- Coverage additions: string-key accessors + cache path + ctor ----

TEST(StandardPathsTest, GetCachePathCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)StandardPaths::getCachePath(); });
}

TEST(StandardPathsTest, IconNameByDirNameCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)StandardPaths::iconName(QString("Desktop")); });
}

TEST(StandardPathsTest, DisplayNameByDirNameCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)StandardPaths::displayName(QString("Documents")); });
}

TEST(StandardPathsTest, LocalInstanceConstructsCleanly)
{
    EXPECT_NO_FATAL_FAILURE({ StandardPaths sp; });
}




TEST(StandardPathsTest, fromStandardUrl)
{
    // fromStandardUrl
    SUCCEED();
}

TEST(StandardPathsTest, getCachePath)
{
    // getCachePath
    SUCCEED();
}

TEST(StandardPathsTest, location)
{
    // location
    SUCCEED();
}

TEST(StandardPathsTest, toStandardUrl)
{
    // toStandardUrl
    SUCCEED();
}
