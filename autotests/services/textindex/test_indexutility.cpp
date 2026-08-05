// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexutility.cpp
 * @brief Unit tests for IndexUtility free functions (indexutility.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/utils/indexutility.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

TEST(IndexUtilityTest, NormalizeDirectoryPathAddsTrailingSlash)
{
    EXPECT_EQ(PathCalculator::normalizeDirectoryPath("/home/user"), QString("/home/user/"));
    EXPECT_EQ(PathCalculator::normalizeDirectoryPath("/home/user/"), QString("/home/user/"));
}

TEST(IndexUtilityTest, IsDirectoryMoveExistingDir)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    EXPECT_TRUE(PathCalculator::isDirectoryMove(tmp.path()));
}

TEST(IndexUtilityTest, IsDirectoryMoveExistingFile)
{
    QTemporaryDir tmp;
    QString path = tmp.path() + "/afile.txt";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();
    EXPECT_FALSE(PathCalculator::isDirectoryMove(path));
}

TEST(IndexUtilityTest, IsDirectoryMoveEmptyReturnsFalse)
{
    EXPECT_FALSE(PathCalculator::isDirectoryMove(""));
}

TEST(IndexUtilityTest, IsDirectoryMoveTrailingSlashInferred)
{
    EXPECT_TRUE(PathCalculator::isDirectoryMove("/no/such/dir/"));
    EXPECT_FALSE(PathCalculator::isDirectoryMove("/no/such/file"));
}

TEST(IndexUtilityTest, ExtractAncestorPathsBuildsList)
{
    QStringList ancestors = PathCalculator::extractAncestorPaths("/home/user/docs/file.txt");
    EXPECT_FALSE(ancestors.isEmpty());
    EXPECT_TRUE(ancestors.contains("/home/user/docs"));
}

TEST(IndexUtilityTest, ExtractAncestorPathsEmptyReturnsEmpty)
{
    EXPECT_TRUE(PathCalculator::extractAncestorPaths("").isEmpty());
}

TEST(IndexUtilityTest, CheckFileSizeWithinLimit)
{
    QTemporaryDir tmp;
    QString path = tmp.path() + "/small.txt";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("tiny");
    f.close();
    EXPECT_TRUE(IndexUtility::checkFileSize(QFileInfo(path), 50));
}

TEST(IndexUtilityTest, CheckFileSizeExceedsLimit)
{
    QTemporaryDir tmp;
    QString path = tmp.path() + "/big.txt";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write(QByteArray(10, 'x'));
    f.close();
    // 0 falls back to 50MB default, a 10-byte file fits
    EXPECT_TRUE(IndexUtility::checkFileSize(QFileInfo(path), 0));
    EXPECT_TRUE(IndexUtility::checkFileSize(QFileInfo(path), 50));
}

TEST(IndexUtilityTest, CheckFileSizeNonExistent)
{
    QFileInfo info("/no/such/file/here.txt");
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::checkFileSize(info, 50); });
}

TEST(IndexUtilityTest, IsSupportedTextFileBySuffix)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::isSupportedTextFile("/some/path/readme.txt"); });
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::isSupportedTextFile("/some/path/unknown.xyz"); });
}

TEST(IndexUtilityTest, IsSupportedOCRFileBySuffix)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::isSupportedOCRFile("/some/path/image.png"); });
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::isSupportedOCRFile("/some/path/unknown.xyz"); });
}

TEST(IndexUtilityTest, IsDefaultIndexedDirectory)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::isDefaultIndexedDirectory("/no/such/indexed/dir"); });
}

TEST(IndexUtilityTest, IsIndexWithAnything)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::isIndexWithAnything("/no/such/dir"); });
}

TEST(IndexUtilityTest, AnythingConfigWatcherInstance)
{
    EXPECT_NE(IndexUtility::AnythingConfigWatcher::instance(), nullptr);
}

TEST(IndexUtilityTest, AnythingConfigWatcherDefaultPaths)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::AnythingConfigWatcher::instance()->defaultAnythingIndexPaths(); });
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::AnythingConfigWatcher::instance()->defaultAnythingIndexPathsRealtime(); });
}

TEST(IndexUtilityTest, AnythingConfigWatcherBlacklistPaths)
{
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::AnythingConfigWatcher::instance()->defaultBlacklistPaths(); });
    EXPECT_NO_FATAL_FAILURE({ (void)IndexUtility::AnythingConfigWatcher::instance()->defaultBlacklistPathsRealtime(); });
}
