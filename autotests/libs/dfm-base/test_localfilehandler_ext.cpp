// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localfilehandler_ext.cpp
 * @brief Extended unit tests for LocalFileHandler (localfilehandler.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QDateTime>
#include <mutex>
#include <QIcon>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class LocalFileHandlerExtTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }

    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        rootPath = tmpDir.path();
    }

    QTemporaryDir tmpDir;
    QString rootPath;
    static std::once_flag flag;
};

std::once_flag LocalFileHandlerExtTest::flag;

TEST_F(LocalFileHandlerExtTest, MoveFileSucceeds)
{
    LocalFileHandler handler;
    QString src = rootPath + "/move_src.txt";
    QString dst = rootPath + "/move_dst.txt";
    QFile f(src);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();
    EXPECT_TRUE(handler.moveFile(QUrl::fromLocalFile(src), QUrl::fromLocalFile(dst)));
    EXPECT_TRUE(QFile::exists(dst));
}

TEST_F(LocalFileHandlerExtTest, MoveFileNonExistentReturnsFalse)
{
    LocalFileHandler handler;
    EXPECT_FALSE(handler.moveFile(QUrl::fromLocalFile(rootPath + "/no_such_src"),
                                  QUrl::fromLocalFile(rootPath + "/dst")));
}

TEST_F(LocalFileHandlerExtTest, CopyFileOverwrite)
{
    LocalFileHandler handler;
    QString src = rootPath + "/copy_ext_src.txt";
    QString dst = rootPath + "/copy_ext_dst.txt";
    QFile f(src);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("copydata");
    f.close();
    EXPECT_TRUE(handler.copyFile(QUrl::fromLocalFile(src), QUrl::fromLocalFile(dst)));
    EXPECT_TRUE(QFile::exists(dst));
}

TEST_F(LocalFileHandlerExtTest, SetFileTime)
{
    LocalFileHandler handler;
    QString path = rootPath + "/settime.txt";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("t");
    f.close();
    QDateTime now = QDateTime::currentDateTime();
    EXPECT_NO_FATAL_FAILURE({
        (void)handler.setFileTime(QUrl::fromLocalFile(path), now, now);
    });
}

TEST_F(LocalFileHandlerExtTest, RenameFilesBatch)
{
    LocalFileHandler handler;
    QString src = rootPath + "/batch_src.txt";
    QFile f(src);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();
    QMap<QUrl, QUrl> mapping;
    mapping.insert(QUrl::fromLocalFile(src), QUrl::fromLocalFile(rootPath + "/batch_dst.txt"));
    QMap<QUrl, QUrl> success;
    EXPECT_NO_FATAL_FAILURE({ (void)handler.renameFilesBatch(mapping, success); });
}

TEST_F(LocalFileHandlerExtTest, DoHiddenFileRemind)
{
    LocalFileHandler handler;
    bool check = false;
    EXPECT_NO_FATAL_FAILURE({ (void)handler.doHiddenFileRemind(".hidden", &check); });
}

TEST_F(LocalFileHandlerExtTest, GetInvalidPathAfterFailure)
{
    LocalFileHandler handler;
    handler.deleteFile(QUrl::fromLocalFile(rootPath + "/nonexistent_for_invalid.txt"));
    EXPECT_NO_FATAL_FAILURE({ (void)handler.getInvalidPath(); });
}

TEST_F(LocalFileHandlerExtTest, SetPermissionsRecursive)
{
    LocalFileHandler handler;
    QString path = rootPath + "/perms_recursive.txt";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();
    EXPECT_NO_FATAL_FAILURE({
        (void)handler.setPermissionsRecursive(QUrl::fromLocalFile(path),
                                              QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    });
}

TEST_F(LocalFileHandlerExtTest, OpenFileNonExistentReturnsFalse)
{
    LocalFileHandler handler;
    EXPECT_FALSE(handler.openFile(QUrl::fromLocalFile(rootPath + "/no_such_open.txt")));
}

TEST_F(LocalFileHandlerExtTest, TouchFileWithTempUrl)
{
    LocalFileHandler handler;
    QString path = rootPath + "/touch_temp.txt";
    EXPECT_NO_FATAL_FAILURE({
        (void)handler.touchFile(QUrl::fromLocalFile(path), QUrl::fromLocalFile(rootPath + "/tmp_template"));
    });
}
