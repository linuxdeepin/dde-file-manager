// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indextraverseutils.cpp
 * @brief Unit tests for IndexTraverseUtils (utils/indextraverseutils.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QSet>
#include <QString>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/utils/indextraverseutils.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class IndexTraverseUtilsTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        QDir root(tmp.path());
        root.mkpath("subdir1/subsub");
        root.mkpath("subdir2");
        QFile f(root.filePath("file.txt"));
        f.open(QIODevice::WriteOnly);
        f.write("hello");
        f.close();
        QFile f2(root.filePath(".hidden"));
        f2.open(QIODevice::WriteOnly);
        f2.write("hidden");
        f2.close();
    }
};

// ---- isHiddenFile ----
TEST_F(IndexTraverseUtilsTest, IsHiddenFile_DotPrefix)
{
    EXPECT_TRUE(IndexTraverseUtils::isHiddenFile(".bashrc"));
    EXPECT_TRUE(IndexTraverseUtils::isHiddenFile(".config"));
}

TEST_F(IndexTraverseUtilsTest, IsHiddenFile_NoDotPrefix)
{
    EXPECT_FALSE(IndexTraverseUtils::isHiddenFile("file.txt"));
    EXPECT_FALSE(IndexTraverseUtils::isHiddenFile("README"));
}

// ---- isSpecialDir ----
TEST_F(IndexTraverseUtilsTest, IsSpecialDir_Dot)
{
    EXPECT_TRUE(IndexTraverseUtils::isSpecialDir("."));
}

TEST_F(IndexTraverseUtilsTest, IsSpecialDir_DotDot)
{
    EXPECT_TRUE(IndexTraverseUtils::isSpecialDir(".."));
}

TEST_F(IndexTraverseUtilsTest, IsSpecialDir_RegularName)
{
    EXPECT_FALSE(IndexTraverseUtils::isSpecialDir("file.txt"));
    EXPECT_FALSE(IndexTraverseUtils::isSpecialDir("subdir"));
}

// ---- isValidFile ----
TEST_F(IndexTraverseUtilsTest, IsValidFile_ExistingFile)
{
    EXPECT_TRUE(IndexTraverseUtils::isValidFile(tmp.path() + "/file.txt"));
}

TEST_F(IndexTraverseUtilsTest, IsValidFile_NonExistent)
{
    EXPECT_FALSE(IndexTraverseUtils::isValidFile(tmp.path() + "/nonexistent.txt"));
}

// ---- isValidDirectory ----
TEST_F(IndexTraverseUtilsTest, IsValidDirectory_ValidDir)
{
    QSet<QString> visited;
    EXPECT_TRUE(IndexTraverseUtils::isValidDirectory(tmp.path(), visited));
    EXPECT_TRUE(visited.contains(QDir(tmp.path()).canonicalPath()));
}

TEST_F(IndexTraverseUtilsTest, IsValidDirectory_DuplicateDir)
{
    QSet<QString> visited;
    EXPECT_TRUE(IndexTraverseUtils::isValidDirectory(tmp.path(), visited));
    // Second call with same dir returns false (already visited)
    EXPECT_FALSE(IndexTraverseUtils::isValidDirectory(tmp.path(), visited));
}

TEST_F(IndexTraverseUtilsTest, IsValidDirectory_Symlink)
{
    QString linkPath = tmp.path() + "/symlink_to_subdir";
    QFile::link(tmp.path() + "/subdir1", linkPath);
    QSet<QString> visited;
    EXPECT_FALSE(IndexTraverseUtils::isValidDirectory(linkPath, visited));
}

TEST_F(IndexTraverseUtilsTest, IsValidDirectory_NonExistent)
{
    QSet<QString> visited;
    EXPECT_FALSE(IndexTraverseUtils::isValidDirectory("/nonexistent/path/12345", visited));
}

// ---- shouldSkipDirectory ----
TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Proc)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/proc"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/proc/something"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Sys)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/sys"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/sys/class/net"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Dev)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/dev"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/dev/sda1"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Boot)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/boot"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/boot/grub"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Tmp)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/tmp"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/tmp/something"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_VarTmp)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/var/tmp"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_VarCache)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/var/cache"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_VarLog)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/var/log"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Run)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/run"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/run/media"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Mnt)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/mnt"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/mnt/data"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Opt)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/opt"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Srv)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/srv"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_UsrLib)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/usr/lib"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/usr/lib32"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/usr/lib64"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/usr/libx32"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_UsrShare)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/usr/share"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Data)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/data"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Ostree)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/ostree"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/persistent/ostree"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_LostFound)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/lost+found"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_Media)
{
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/media"));
}

TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_NormalPath)
{
    // /home/user is not in the exclusion list
    EXPECT_FALSE(IndexTraverseUtils::shouldSkipDirectory("/home/user/docs"));
}

// Edge case: startsWith-based matching (implementation detail)
TEST_F(IndexTraverseUtilsTest, ShouldSkipDirectory_StartsWithBehavior)
{
    // The implementation uses startsWith, so /processing matches /proc prefix
    // This test documents the actual behavior
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/procinfo"));
    EXPECT_TRUE(IndexTraverseUtils::shouldSkipDirectory("/sysfs"));
}

// ---- fstabBindInfo ----
TEST_F(IndexTraverseUtilsTest, FstabBindInfo_ReturnsMap)
{
    // /etc/fstab always exists on Linux, so this should work.
    // Even if empty, it should return a valid QMap.
    QMap<QString, QString> info = IndexTraverseUtils::fstabBindInfo();
    // Just verify it doesn't crash and returns something
    SUCCEED();
}

TEST_F(IndexTraverseUtilsTest, FstabBindInfo_ConsistentAcrossCalls)
{
    QMap<QString, QString> first = IndexTraverseUtils::fstabBindInfo();
    QMap<QString, QString> second = IndexTraverseUtils::fstabBindInfo();
    // Same file, same mtime -> should return identical results
    EXPECT_EQ(first, second);
}
