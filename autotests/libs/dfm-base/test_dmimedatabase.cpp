// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dmimedatabase.cpp
 * @brief Unit tests for DMimeDatabase (dmimedatabase.cpp)
 *
 * DMimeDatabase wraps QMimeDatabase with file-manager specific safeguards
 * (block-listed kernel files, .lock/.pid shortcuts, an inode mime cache).
 * Tests exercise the pure, hardware-free paths using local temp files and
 * the non-local URL branch of mimeTypeForUrl (no InfoFactory scheme setup
 * required).
 */

#include <gtest/gtest.h>
#include <dfm-base/mimetype/dmimedatabase.h>

#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QFileInfo>
#include <QMimeDatabase>

using namespace dfmbase;

namespace {
// Create a temp file with the given suffix+content, returning its absolute path.
QString utMakeTempFile(const QString &suffix, const QByteArray &content = "hello")
{
    QTemporaryFile tmp(QDir::tempPath() + "/ut_dmimedb_XXXXXX" + suffix);
    tmp.setAutoRemove(false);
    if (!tmp.open()) {
        ADD_FAILURE() << "failed to create temp file";
        return {};
    }
    tmp.write(content);
    tmp.close();
    return tmp.fileName();
}

void utCleanup(const QString &path)
{
    if (!path.isEmpty())
        QFile::remove(path);
}
}   // namespace

TEST(DMimeDatabaseTest, MimeTypeForUrlNonLocalUrlDelegatesToQt)
{
    DMimeDatabase db;
    QUrl url(QStringLiteral("http://example.com/index.html"));
    ASSERT_FALSE(url.isLocalFile());

    QMimeType mime = db.mimeTypeForUrl(url);
    EXPECT_TRUE(mime.isValid());
}

TEST(DMimeDatabaseTest, MimeTypeForFileTxtByExtensionReturnsTextPlain)
{
    QString path = utMakeTempFile(".txt");
    ASSERT_FALSE(path.isEmpty());

    DMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension, QString(), false);
    EXPECT_TRUE(mime.isValid());
    EXPECT_EQ(mime.name(), QStringLiteral("text/plain"));

    utCleanup(path);
}

TEST(DMimeDatabaseTest, MimeTypeForFileDefaultModeReturnsValidForExistingTxt)
{
    QString path = utMakeTempFile(".txt", "plain text content\n");
    ASSERT_FALSE(path.isEmpty());

    DMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(path, QMimeDatabase::MatchDefault, QString(), false);
    EXPECT_TRUE(mime.isValid());

    utCleanup(path);
}

TEST(DMimeDatabaseTest, MimeTypeForFileWithInodeCachesResult)
{
    QString path = utMakeTempFile(".txt");
    ASSERT_FALSE(path.isEmpty());

    DMimeDatabase db;
    const QString inode = QStringLiteral("ut-inode-42");

    // First call: cache miss -> computes and stores under the inode key.
    QMimeType first = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension, inode, false);
    ASSERT_TRUE(first.isValid());

    // Second call with the same inode: served from the inode cache; same result.
    QMimeType second = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension, inode, false);
    EXPECT_TRUE(second.isValid());
    EXPECT_EQ(second.name(), first.name());

    utCleanup(path);
}

TEST(DMimeDatabaseTest, MimeTypeForFileDirectoryReturnsValidMime)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());

    DMimeDatabase db;
    // A directory path goes through the "isDir -> mimeTypeForFile(QFileInfo('/home'))" branch.
    QMimeType mime = db.mimeTypeForFile(dir.path(), QMimeDatabase::MatchExtension, QString(), false);
    EXPECT_TRUE(mime.isValid());

    // The directory branch resolves against "/home" regardless of input dir,
    // so the result should be the inode/directory mime, not a file mime.
    QMimeDatabase qdb;
    EXPECT_EQ(mime.name(), qdb.mimeTypeForFile(QFileInfo(QStringLiteral("/home"))).name());
}

TEST(DMimeDatabaseTest, MimeTypeForFileNonExistentPathByExtensionStillResolvesSuffix)
{
    // A path that does not exist on disk but has a clear suffix: MatchExtension
    // resolves purely from the name, so it should still return a valid mime.
    QString ghost = QDir::tempPath() + "/ut_dmimedb_does_not_exist_12345.txt";

    DMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(ghost, QMimeDatabase::MatchExtension, QString(), false);
    EXPECT_TRUE(mime.isValid());
    EXPECT_EQ(mime.name(), QStringLiteral("text/plain"));
}
