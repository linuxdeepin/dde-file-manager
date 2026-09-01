// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileutils_ext.cpp
 * @brief Extended unit tests for FileUtils pure-logic functions.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QImage>
#include <QModelIndex>
#include <QFont>
#include <QFontMetrics>
#include <QStandardPaths>
#include <mutex>

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class FileUtilsExtTest : public testing::Test
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

std::once_flag FileUtilsExtTest::flag;

TEST_F(FileUtilsExtTest, IsDesktopFileSuffix)
{
    EXPECT_TRUE(FileUtils::isDesktopFileSuffix(QUrl("file:///tmp/foo.desktop")));
    EXPECT_FALSE(FileUtils::isDesktopFileSuffix(QUrl("file:///tmp/foo.txt")));
}

TEST_F(FileUtilsExtTest, IsTrashFileAndRoot)
{
    QUrl trashUrl("trash:///");
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::isTrashFile(trashUrl); });
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::isTrashRootFile(trashUrl); });
}

TEST_F(FileUtilsExtTest, TrashPathToNormal)
{
    QString result = FileUtils::trashPathToNormal("/foo\\bar");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(FileUtilsExtTest, NormalPathToTrash)
{
    QString result = FileUtils::normalPathToTrash("/foo/bar");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(FileUtilsExtTest, BindPathTransformIdentity)
{
    QString p = "/some/random/path";
    EXPECT_EQ(FileUtils::bindPathTransform(p, false), p);
    EXPECT_EQ(FileUtils::bindPathTransform(p, true), p);
}

TEST_F(FileUtilsExtTest, BindUrlTransformNonTrash)
{
    QUrl url = QUrl::fromLocalFile(rootPath + "/test.txt");
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::bindUrlTransform(url); });
}

TEST_F(FileUtilsExtTest, SymlinkTargetAndResolve)
{
    QString target = rootPath + "/target.txt";
    QString link = rootPath + "/link.txt";
    QFile f(target);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("hi");
    f.close();
    QFile::link(target, link);

    EXPECT_EQ(FileUtils::symlinkTarget(QUrl::fromLocalFile(link)), target);
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::resolveSymlink(QUrl::fromLocalFile(link)); });
}

TEST_F(FileUtilsExtTest, SymlinkTargetNonExistent)
{
    EXPECT_TRUE(FileUtils::symlinkTarget(QUrl::fromLocalFile("/no/such/link")).isEmpty());
}

TEST_F(FileUtilsExtTest, ConvertToSRgbColorSpaceNullImage)
{
    QImage nullImg;
    QImage result = FileUtils::convertToSRgbColorSpace(nullImg);
    EXPECT_TRUE(result.isNull());
}

TEST_F(FileUtilsExtTest, ConvertToSRgbColorSpaceNormalImage)
{
    QImage img(2, 2, QImage::Format_RGB32);
    img.fill(Qt::red);
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::convertToSRgbColorSpace(img); });
}

TEST_F(FileUtilsExtTest, ToUnicode)
{
    QByteArray data("hello world");
    QString s = FileUtils::toUnicode(data);
    EXPECT_FALSE(s.isEmpty());
}

TEST_F(FileUtilsExtTest, DirFfileCountInvalidUrl)
{
    EXPECT_EQ(FileUtils::dirFfileCount(QUrl()), 0);
}

TEST_F(FileUtilsExtTest, SupportLongName)
{
    QUrl url = QUrl::fromLocalFile(rootPath);
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::supportLongName(url); });
}

TEST_F(FileUtilsExtTest, FindIconFromXdg)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::findIconFromXdg("folder"); });
}

TEST_F(FileUtilsExtTest, IsContainProhibitPathEmpty)
{
    EXPECT_FALSE(FileUtils::isContainProhibitPath(QList<QUrl> {}));
}

TEST_F(FileUtilsExtTest, GetFileNameLength)
{
    QUrl url = QUrl::fromLocalFile(rootPath + "/namelen.txt");
    EXPECT_GE(FileUtils::getFileNameLength(url, "namelen.txt"), 0);
}

TEST_F(FileUtilsExtTest, IsHigherHierarchy)
{
    EXPECT_FALSE(FileUtils::isHigherHierarchy(QUrl("file:///home/user/docs"),
                                              QUrl("file:///home/user")));
}

TEST_F(FileUtilsExtTest, PreprocessingFileName)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FileUtils::preprocessingFileName("test_file"); });
}
