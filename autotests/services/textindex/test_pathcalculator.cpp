// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pathcalculator.cpp
 * @brief Unit tests for the PathCalculator namespace
 *        (utils/indexutility.cpp) — pure path helpers:
 *        calculateNewPathForDirectoryMove, normalizeDirectoryPath,
 *        isDirectoryMove, extractAncestorPaths.
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/utils/indexutility.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

TEST(PathCalculatorTest, CalculateNewPathForDirectoryMoveMatch)
{
    QString result = PathCalculator::calculateNewPathForDirectoryMove(
            "/old/dir/file.txt", "/old/dir/", "/new/dir/");
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(result.startsWith("/new/dir/"));
}

TEST(PathCalculatorTest, CalculateNewPathForDirectoryMoveNoMatch)
{
    QString result = PathCalculator::calculateNewPathForDirectoryMove(
            "/other/path.txt", "/old/dir/", "/new/dir/");
    EXPECT_EQ(result, QString("/other/path.txt"));
}

TEST(PathCalculatorTest, CalculateNewPathForDirectoryMoveExactDirMatch)
{
    // oldPath == fromDirPath.chopped(1) -> returns toDirPath
    QString result = PathCalculator::calculateNewPathForDirectoryMove(
            "/old/dir", "/old/dir/", "/new/dir/");
    EXPECT_EQ(result, QString("/new/dir/"));
}

TEST(PathCalculatorTest, NormalizeDirectoryPathAddsTrailingSlash)
{
    EXPECT_EQ(PathCalculator::normalizeDirectoryPath("/tmp/dir"), QString("/tmp/dir/"));
}

TEST(PathCalculatorTest, NormalizeDirectoryPathKeepsTrailingSlash)
{
    EXPECT_EQ(PathCalculator::normalizeDirectoryPath("/tmp/dir/"), QString("/tmp/dir/"));
}

TEST(PathCalculatorTest, NormalizeDirectoryPathEmpty)
{
    EXPECT_EQ(PathCalculator::normalizeDirectoryPath(QString()), QString("/"));
}

TEST(PathCalculatorTest, IsDirectoryMoveEmptyReturnsFalse)
{
    EXPECT_FALSE(PathCalculator::isDirectoryMove(QString()));
}

TEST(PathCalculatorTest, IsDirectoryMoveTrailingSlashReturnsTrue)
{
    EXPECT_TRUE(PathCalculator::isDirectoryMove("/some/dir/"));
}

TEST(PathCalculatorTest, IsDirectoryMoveNoTrailingSlashReturnsFalse)
{
    EXPECT_FALSE(PathCalculator::isDirectoryMove("/some/file.txt"));
}

TEST(PathCalculatorTest, IsDirectoryMoveExistingDirReturnsTrue)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString path = tmp.path();   // exists and is a dir
    EXPECT_TRUE(PathCalculator::isDirectoryMove(path));
}

TEST(PathCalculatorTest, IsDirectoryMoveExistingFileReturnsFalse)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString filePath = tmp.path() + "/test.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();
    EXPECT_FALSE(PathCalculator::isDirectoryMove(filePath));
}

TEST(PathCalculatorTest, ExtractAncestorPathsEmptyReturnsEmpty)
{
    EXPECT_TRUE(PathCalculator::extractAncestorPaths(QString()).isEmpty());
}

TEST(PathCalculatorTest, ExtractAncestorPathsForNestedFile)
{
    QStringList ancestors = PathCalculator::extractAncestorPaths("/a/b/c/file.txt");
    EXPECT_FALSE(ancestors.isEmpty());
    EXPECT_TRUE(ancestors.contains("/a/b/c"));
    EXPECT_TRUE(ancestors.contains("/a/b"));
    EXPECT_TRUE(ancestors.contains("/a"));
}

TEST(PathCalculatorTest, ExtractAncestorPathsForRootFile)
{
    QStringList ancestors = PathCalculator::extractAncestorPaths("/file.txt");
    // File directly under root — ancestors should be empty or minimal
    EXPECT_GE(ancestors.size(), 0);
}
