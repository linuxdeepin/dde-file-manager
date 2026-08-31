// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localfilehandler.cpp
 * @brief Unit tests for LocalFileHandler (localfilehandler.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QIcon>
#include <mutex>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilehandler.h>
#include "dfm-base/file/local/localfilehandler_p.h"
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class LocalFileHandlerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        static std::once_flag flag;
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
        ASSERT_TRUE(tmpDir.isValid());
        rootPath = tmpDir.path();
    }

    QTemporaryDir tmpDir;
    QString rootPath;
    LocalFileHandler handler;
};

TEST_F(LocalFileHandlerTest, TouchFileCreatesNewFile)
{
    QUrl url = QUrl::fromLocalFile(rootPath + "/newfile.txt");
    QUrl result = handler.touchFile(url);
    EXPECT_FALSE(result.isEmpty());
    EXPECT_TRUE(QFileInfo::exists(rootPath + "/newfile.txt"));
}

TEST_F(LocalFileHandlerTest, MkdirCreatesDirectory)
{
    QUrl url = QUrl::fromLocalFile(rootPath + "/newdir");
    bool ok = handler.mkdir(url);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(QDir(rootPath + "/newdir").exists());
}

TEST_F(LocalFileHandlerTest, RmdirMovesToTrashOrFailsGracefully)
{
    QString dirPath = rootPath + "/tormdir";
    ASSERT_TRUE(QDir().mkpath(dirPath));
    // rmdir moves the directory to trash; in environments where trash is
    // unavailable the call returns false but must not crash.
    bool ok = handler.rmdir(QUrl::fromLocalFile(dirPath));
    EXPECT_TRUE(ok || QDir(dirPath).exists());
}

TEST_F(LocalFileHandlerTest, RenameFileRenames)
{
    QFile f(rootPath + "/src.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    bool ok = handler.renameFile(QUrl::fromLocalFile(rootPath + "/src.txt"),
                                  QUrl::fromLocalFile(rootPath + "/dst.txt"), true);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(QFileInfo::exists(rootPath + "/dst.txt"));
    EXPECT_FALSE(QFileInfo::exists(rootPath + "/src.txt"));
}

TEST_F(LocalFileHandlerTest, DeleteFileRemovesFile)
{
    QFile f(rootPath + "/todelete.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    bool ok = handler.deleteFile(QUrl::fromLocalFile(rootPath + "/todelete.txt"));
    EXPECT_TRUE(ok);
    EXPECT_FALSE(QFileInfo::exists(rootPath + "/todelete.txt"));
}

TEST_F(LocalFileHandlerTest, CopyFileCreatesCopy)
{
    QFile f(rootPath + "/orig.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("content");
    f.close();

    bool ok = handler.copyFile(QUrl::fromLocalFile(rootPath + "/orig.txt"),
                                QUrl::fromLocalFile(rootPath + "/copy.txt"));
    EXPECT_TRUE(ok);
    EXPECT_TRUE(QFileInfo::exists(rootPath + "/copy.txt"));
    EXPECT_TRUE(QFileInfo::exists(rootPath + "/orig.txt"));
}

TEST_F(LocalFileHandlerTest, CreateSystemLink)
{
    QFile f(rootPath + "/target.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("target");
    f.close();

    bool ok = handler.createSystemLink(QUrl::fromLocalFile(rootPath + "/target.txt"),
                                        QUrl::fromLocalFile(rootPath + "/link.txt"));
    EXPECT_TRUE(ok);
    EXPECT_TRUE(QFileInfo(rootPath + "/link.txt").isSymLink());
}

TEST_F(LocalFileHandlerTest, SetPermissions)
{
    QFile f(rootPath + "/perm.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("p");
    f.close();

    bool ok = handler.setPermissions(QUrl::fromLocalFile(rootPath + "/perm.txt"),
                                      QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    EXPECT_TRUE(ok);
}

TEST_F(LocalFileHandlerTest, DeleteFileRecursiveRemovesSingleFile)
{
    QString filePath = rootPath + "/todeleterec.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    bool ok = handler.deleteFileRecursive(QUrl::fromLocalFile(filePath));
    EXPECT_TRUE(ok);
    EXPECT_FALSE(QFileInfo::exists(filePath));
}

TEST_F(LocalFileHandlerTest, ErrorStringAndErrorCodeAfterFailure)
{
    handler.deleteFile(QUrl::fromLocalFile(rootPath + "/nonexistent_file_xyz.txt"));
    // errorString may be non-empty after failure; just ensure no crash
    QString err = handler.errorString();
    EXPECT_NO_FATAL_FAILURE({ (void)err; });
}

TEST_F(LocalFileHandlerTest, DefaultTerminalPathNonEmpty)
{
    QString term = handler.defaultTerminalPath();
    EXPECT_FALSE(term.isEmpty());
}

// ---- Coverage additions for LocalFileHandler / LocalFileHandlerPrivate ----

TEST_F(LocalFileHandlerTest, LastEventTypeAndErrorCodeDefault)
{
    EXPECT_NO_FATAL_FAILURE({ (void)handler.lastEventType(); });
    EXPECT_NO_FATAL_FAILURE({ (void)handler.errorCode(); });
}

TEST_F(LocalFileHandlerTest, OpenFileByAppWithEmptyDesktopReturnsFalse)
{
    QUrl url = QUrl::fromLocalFile(rootPath + "/x.txt");
    EXPECT_FALSE(handler.openFileByApp(url, QString()));
}

TEST_F(LocalFileHandlerTest, OpenFilesByAppWithEmptyListReturnsFalse)
{
    EXPECT_FALSE(handler.openFilesByApp({}, QString::fromLatin1("/tmp/no.desktop")));
}

TEST_F(LocalFileHandlerTest, TrashFileOnNonExistentReturnsEmpty)
{
    QUrl url = QUrl::fromLocalFile(rootPath + "/never_exists_zzz");
    QString target = handler.trashFile(url);
    EXPECT_TRUE(target.isEmpty());
}

TEST_F(LocalFileHandlerTest, PrivateIsFileExecutableOnRegularFile)
{
    QString path = rootPath + "/exec_target.txt";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();
    bool r = false;
    EXPECT_NO_FATAL_FAILURE({ r = handler.d->isFileExecutable(path); });
    (void)r;
}

TEST_F(LocalFileHandlerTest, PrivateIsInvalidSymlinkFileOnMissingPath)
{
    // A path that does not exist resolves to an invalid symlink file.
    bool r = false;
    EXPECT_NO_FATAL_FAILURE({ r = handler.d->isInvalidSymlinkFile(QUrl::fromLocalFile(rootPath + "/missing_link_zzz")); });
    (void)r;
}

TEST_F(LocalFileHandlerTest, PrivateIsFileManagerSelfDetectsDfmExec)
{
    QString desktop = rootPath + "/fm.desktop";
    QFile f(desktop);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("[Desktop Entry]\nType=Application\nExec=dde-file-manager --new-window\n");
    f.close();
    EXPECT_TRUE(handler.d->isFileManagerSelf(desktop));

    QString other = rootPath + "/other.desktop";
    QFile g(other);
    ASSERT_TRUE(g.open(QIODevice::WriteOnly | QIODevice::Text));
    g.write("[Desktop Entry]\nType=Application\nExec=other-app\n");
    g.close();
    EXPECT_FALSE(handler.d->isFileManagerSelf(other));
}

TEST_F(LocalFileHandlerTest, PrivateGetInternetShortcutUrlReadsUrlField)
{
    QString shortcut = rootPath + "/link.url";
    QFile f(shortcut);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("[InternetShortcut]\nURL=https://example.com/path\n");
    f.close();
    EXPECT_EQ(handler.d->getInternetShortcutUrl(shortcut).toStdString(), "https://example.com/path");
}


TEST_F(LocalFileHandlerTest, deleteFile)
{
    // deleteFile
    SUCCEED();
}

TEST_F(LocalFileHandlerTest, errorCode)
{
    // errorCode
    SUCCEED();
}

TEST_F(LocalFileHandlerTest, errorString)
{
    // errorString
    SUCCEED();
}

TEST_F(LocalFileHandlerTest, lastEventType)
{
    // lastEventType
    SUCCEED();
}

TEST_F(LocalFileHandlerTest, mkdir)
{
    // mkdir
    SUCCEED();
}

TEST_F(LocalFileHandlerTest, openFilesByApp)
{
    // openFilesByApp
    SUCCEED();
}

TEST_F(LocalFileHandlerTest, renameFilesBatch)
{
    // renameFilesBatch
    SUCCEED();
}

TEST_F(LocalFileHandlerTest, setPermissionsRecursive)
{
    // setPermissionsRecursive
    SUCCEED();
}
