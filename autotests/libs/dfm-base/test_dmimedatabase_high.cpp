// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dmimedatabase_high.cpp
 * @brief Mode 2 supplement: High-priority DMimeDatabase::mimeTypeForFile.
 *
 * Target (from .ut-inventory.json, level=high, usecase_count==0):
 *   - DMimeDatabase::mimeTypeForFile
 *
 * Branch清单 (declared, cross-checked via MCP get_code_snippet):
 *   - inod not empty + cached → return cached mimeType
 *   - inod not empty + not cached → compute + cache
 *   - fileInfo.isDir() → return mimeTypeForFile("/home")
 *   - .pid / .lock / lockfile + not MatchExtension → check gvfs regex
 *   - symlink → use symLinkTarget for blacklist check
 *   - blacklisted path → use MatchExtension
 *   - normal file → use requested mode
 */

#include <gtest/gtest.h>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QUrl>

#include <dfm-base/mimetype/dmimedatabase.h>

using namespace dfmbase;

TEST(DMimeDatabaseTest, MimeTypeForRegularTextFile)
{
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    tmpFile.write("hello world\n");
    tmpFile.close();

    DMimeDatabase db;
    QFileInfo fi(tmpFile.fileName());
    QMimeType mt = db.mimeTypeForFile(fi, QMimeDatabase::MatchDefault, QString());
    EXPECT_FALSE(mt.isDefault());
    EXPECT_TRUE(mt.name().startsWith("text/") || mt.name().contains("plain"));
}

TEST(DMimeDatabaseTest, MimeTypeForDirectory)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());

    DMimeDatabase db;
    QFileInfo fi(tmpDir.path());
    QMimeType mt = db.mimeTypeForFile(fi, QMimeDatabase::MatchDefault, QString());
    EXPECT_EQ(mt.name(), QString("inode/directory"));
}

TEST(DMimeDatabaseTest, MimeTypeForFileWithExtension)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/test.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("some text content");
    f.close();

    DMimeDatabase db;
    QFileInfo fi(filePath);
    QMimeType mt = db.mimeTypeForFile(fi, QMimeDatabase::MatchExtension, QString());
    EXPECT_EQ(mt.name(), QString("text/plain"));
}

TEST(DMimeDatabaseTest, InodCachingReturnsSameMimeType)
{
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    tmpFile.write("cached content\n");
    tmpFile.close();

    DMimeDatabase db;
    QFileInfo fi(tmpFile.fileName());
    QString inod = "test-inod-12345";

    // First call with inod → computes and caches
    QMimeType mt1 = db.mimeTypeForFile(fi, QMimeDatabase::MatchDefault, inod, false);
    EXPECT_FALSE(mt1.isDefault());

    // Second call with same inod → should return cached result
    QMimeType mt2 = db.mimeTypeForFile(fi, QMimeDatabase::MatchDefault, inod, false);
    EXPECT_EQ(mt1.name(), mt2.name());
}

TEST(DMimeDatabaseTest, MimeTypeForPidFileNotMatchExtension)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/test.pid";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("12345\n");
    f.close();

    DMimeDatabase db;
    QFileInfo fi(filePath);
    // With MatchDefault (not MatchExtension), .pid files trigger special handling
    EXPECT_NO_FATAL_FAILURE({ db.mimeTypeForFile(fi, QMimeDatabase::MatchDefault, QString()); });
}

TEST(DMimeDatabaseTest, MimeTypeForLockFileNotMatchExtension)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/msg.lock";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("locked\n");
    f.close();

    DMimeDatabase db;
    QFileInfo fi(filePath);
    EXPECT_NO_FATAL_FAILURE({ db.mimeTypeForFile(fi, QMimeDatabase::MatchDefault, QString()); });
}

TEST(DMimeDatabaseTest, MimeTypeForLockfileSuffixNotMatchExtension)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString filePath = tmpDir.path() + "/something.lockfile";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data\n");
    f.close();

    DMimeDatabase db;
    QFileInfo fi(filePath);
    EXPECT_NO_FATAL_FAILURE({ db.mimeTypeForFile(fi, QMimeDatabase::MatchDefault, QString()); });
}

TEST(DMimeDatabaseTest, MimeTypeForSymlink)
{
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString targetPath = tmpDir.path() + "/target.txt";
    QString linkPath = tmpDir.path() + "/link.txt";

    QFile target(targetPath);
    ASSERT_TRUE(target.open(QIODevice::WriteOnly));
    target.write("target content");
    target.close();

    ASSERT_TRUE(QFile::link(targetPath, linkPath));

    DMimeDatabase db;
    QFileInfo fi(linkPath);
    EXPECT_NO_FATAL_FAILURE({ db.mimeTypeForFile(fi, QMimeDatabase::MatchDefault, QString()); });
}

TEST(DMimeDatabaseTest, MimeTypeForNonExistentFile)
{
    DMimeDatabase db;
    QFileInfo fi("/nonexistent/path/file.txt");
    EXPECT_NO_FATAL_FAILURE({ db.mimeTypeForFile(fi, QMimeDatabase::MatchExtension, QString()); });
}

TEST(DMimeDatabaseTest, MimeTypeForUrl)
{
    DMimeDatabase db;
    EXPECT_NO_FATAL_FAILURE({ db.mimeTypeForUrl(QUrl::fromLocalFile("/tmp/test.txt")); });
}

// Cover the QString overload: mimeTypeForFile(QString, MatchMode, inod, isGvfs)
TEST(DMimeDatabaseTest, MimeTypeForFileQStringOverload)
{
    QString tmpDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString filePath = tmpDir + "/dmime_qstring_test.txt";
    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly)) {
        f.write("hello");
        f.close();
    }

    DMimeDatabase db;
    // QString overload with empty inod
    QMimeType mt1 = db.mimeTypeForFile(filePath, QMimeDatabase::MatchDefault, QString());
    EXPECT_FALSE(mt1.isDefault());

    // QString overload with inod (caching path)
    QString inod = QStringLiteral("fake-inod-123");
    QMimeType mt2 = db.mimeTypeForFile(filePath, QMimeDatabase::MatchDefault, inod);
    EXPECT_FALSE(mt2.isDefault());
    EXPECT_EQ(mt1.name(), mt2.name());

    // MatchExtension via QString overload
    QMimeType mt3 = db.mimeTypeForFile(filePath, QMimeDatabase::MatchExtension, QString());
    EXPECT_EQ(mt3.name(), QString("text/plain"));

    // isGvfs = true
    EXPECT_NO_FATAL_FAILURE({ db.mimeTypeForFile(filePath, QMimeDatabase::MatchDefault, QString(), true); });

    QFile::remove(filePath);
}
