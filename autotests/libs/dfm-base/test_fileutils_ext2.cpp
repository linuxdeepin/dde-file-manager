// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileutils_ext2.cpp
 * @brief Second batch of FileUtils tests targeting remaining functions.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QList>
#include <QPair>
#include <QString>
#include <QImage>
#include <mutex>
#include <QIcon>

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class FileUtilsExt2Test : public testing::Test
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

std::once_flag FileUtilsExt2Test::flag;

TEST_F(FileUtilsExt2Test, SupportedMaxLengthKnownFs)
{
    EXPECT_GE(FileUtils::supportedMaxLength("ext4"), 1);
    EXPECT_GE(FileUtils::supportedMaxLength("unknownfs"), 1);
}

TEST_F(FileUtilsExt2Test, IsDesktopFileFalse)
{
    QString path = rootPath + "/notadesktop.txt";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("text");
    f.close();
    EXPECT_FALSE(FileUtils::isDesktopFile(QUrl::fromLocalFile(path)));
}

TEST_F(FileUtilsExt2Test, IsTrashDesktopFile)
{
    EXPECT_FALSE(FileUtils::isTrashDesktopFile(QUrl("file:///tmp/no_such.desktop")));
}

TEST_F(FileUtilsExt2Test, IsHomeDesktopFile)
{
    EXPECT_FALSE(FileUtils::isHomeDesktopFile(QUrl("file:///tmp/no_such.desktop")));
}

TEST_F(FileUtilsExt2Test, IsSameFileByUrl)
{
    QString path = rootPath + "/same.txt";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();
    QUrl url = QUrl::fromLocalFile(path);
    EXPECT_NO_FATAL_FAILURE({
        (void)FileUtils::isSameFile(url, url, Global::CreateFileInfoType::kCreateFileInfoSync);
    });
}

TEST_F(FileUtilsExt2Test, IsCdRomDevice)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::isCdRomDevice(QUrl("file:///tmp")); });
}

TEST_F(FileUtilsExt2Test, TrashRootUrl)
{
    QUrl url = FileUtils::trashRootUrl();
    EXPECT_FALSE(url.scheme().isEmpty());
}

TEST_F(FileUtilsExt2Test, IsTrashFileNonTrashUrl)
{
    EXPECT_FALSE(FileUtils::isTrashFile(QUrl("file:///tmp/notrash")));
}

TEST_F(FileUtilsExt2Test, DirFfileCountForDir)
{
    QString dir = rootPath + "/countdir";
    QDir().mkpath(dir);
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::dirFfileCount(QUrl::fromLocalFile(dir)); });
}

TEST_F(FileUtilsExt2Test, BindUrlTransformNonTrash)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::bindUrlTransform(QUrl("file:///tmp/x")); });
}

TEST_F(FileUtilsExt2Test, FindIconFromXdgTheme)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::findIconFromXdg("folder"); });
}

TEST_F(FileUtilsExt2Test, CacheCopyingFileUrlLifecycle)
{
    QUrl url("file:///tmp/dfm_ext2_copy_test.txt");
    FileUtils::cacheCopyingFileUrl(url);
    EXPECT_TRUE(FileUtils::containsCopyingFileUrl(url));
    FileUtils::removeCopyingFileUrl(url);
    EXPECT_FALSE(FileUtils::containsCopyingFileUrl(url));
}

TEST_F(FileUtilsExt2Test, NotifyFileChangeManualNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({
        FileUtils::notifyFileChangeManual(Global::FileNotifyType::kFileAdded, QUrl("file:///tmp/dfm_notify_test"));
    });
}

TEST_F(FileUtilsExt2Test, SetBackGroundNoCrash)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::setBackGround("/no/such/picture.png"); });
}

TEST_F(FileUtilsExt2Test, FileBatchCustomText)
{
    QList<QUrl> urls { QUrl("file:///tmp/a.txt"), QUrl("file:///tmp/b.txt") };
    QPair<QString, QString> pair { "a", "A" };
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::fileBatchCustomText(urls, pair); });
}

TEST_F(FileUtilsExt2Test, ComputerAndHomeDesktopFileUrl)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DesktopAppUrl::computerDesktopFileUrl(); });
    EXPECT_NO_FATAL_FAILURE({ (void)DesktopAppUrl::homeDesktopFileUrl(); });
}

TEST_F(FileUtilsExt2Test, TrashDesktopFileUrl)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DesktopAppUrl::trashDesktopFileUrl(); });
}
