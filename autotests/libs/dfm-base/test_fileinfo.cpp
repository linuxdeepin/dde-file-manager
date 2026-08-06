// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfo.cpp
 * @brief Unit tests for SyncFileInfo / FileInfo (syncfileinfo.cpp, fileinfo.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QIcon>
#include <mutex>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include "dfm-base/file/local/private/syncfileinfo_p.h"
#include <dfm-io/dfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

using namespace dfmbase;

class FileInfoTest : public testing::Test
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

std::once_flag FileInfoTest::flag;

TEST_F(FileInfoTest, CreateFileInfoAndQueryBasicAttributes)
{
    QString filePath = rootPath + "/testfile.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("hello world");
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    info->initQuerier();
    EXPECT_NO_FATAL_FAILURE({ (void)info->exists(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->size(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->nameOf(FileInfo::FileNameInfoType::kFileName); });
}

TEST_F(FileInfoTest, FileInfoNameOfReturnsFileName)
{
    QString filePath = rootPath + "/named.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    info->refresh();
    EXPECT_EQ(info->nameOf(FileInfo::FileNameInfoType::kFileName), QString("named.txt"));
    EXPECT_EQ(info->nameOf(FileInfo::FileNameInfoType::kCompleteBaseName), QString("named"));
}

TEST_F(FileInfoTest, FileInfoPathOfReturnsPath)
{
    QString filePath = rootPath + "/pathtest.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    QString absPath = info->pathOf(FileInfo::FilePathInfoType::kAbsolutePath);
    EXPECT_FALSE(absPath.isEmpty());
}

TEST_F(FileInfoTest, FileInfoUrlOfReturnsUrl)
{
    QString filePath = rootPath + "/urltest.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    QUrl url = QUrl::fromLocalFile(filePath);
    auto info = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(info, nullptr);
    QUrl infoUrl = info->urlOf(FileInfo::FileUrlInfoType::kUrl);
    EXPECT_FALSE(infoUrl.isEmpty());
}

TEST_F(FileInfoTest, FileInfoForNonExistentFile)
{
    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(rootPath + "/nonexistent.txt"));
    ASSERT_NE(info, nullptr);
    EXPECT_FALSE(info->exists());
}

TEST_F(FileInfoTest, FileInfoIsAttributesForDirectory)
{
    QString dirPath = rootPath + "/testdir";
    ASSERT_TRUE(QDir().mkpath(dirPath));
    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(dirPath));
    ASSERT_NE(info, nullptr);
    info->initQuerier();
    EXPECT_NO_FATAL_FAILURE({ (void)info->isAttributes(FileInfo::FileIsType::kIsDir); });
}

TEST_F(FileInfoTest, FileInfoRefreshUpdatesContent)
{
    QString filePath = rootPath + "/refresh.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("initial");
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    info->initQuerier();
    EXPECT_NO_FATAL_FAILURE({ info->refresh(); });
    f.open(QIODevice::WriteOnly);
    f.write("updated content");
    f.close();
    EXPECT_NO_FATAL_FAILURE({ info->refresh(); });
}

TEST_F(FileInfoTest, FileInfoPermissions)
{
    QString filePath = rootPath + "/perm.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    info->initQuerier();
    EXPECT_NO_FATAL_FAILURE({ (void)info->permissions(); });
}

TEST_F(FileInfoTest, FileInfoFileType)
{
    QString filePath = rootPath + "/type.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    EXPECT_NO_FATAL_FAILURE({ (void)info->fileType(); });
}

TEST_F(FileInfoTest, FileInfoDisplayOf)
{
    QString filePath = rootPath + "/display.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    EXPECT_NO_FATAL_FAILURE({ (void)info->displayOf(FileInfo::DisplayInfoType::kFileDisplayName); });
}

TEST_F(FileInfoTest, FileInfoTimeOf)
{
    QString filePath = rootPath + "/time.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    EXPECT_NO_FATAL_FAILURE({ (void)info->timeOf(FileInfo::FileTimeType::kLastModified); });
}

// Exercise many SyncFileInfo attribute query paths for coverage.
TEST_F(FileInfoTest, CanAttributesMultipleTypes)
{
    QString filePath = rootPath + "/canattr.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    info->initQuerier();
    EXPECT_NO_FATAL_FAILURE({ (void)info->canAttributes(FileInfo::FileCanType::kCanRename); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->canAttributes(FileInfo::FileCanType::kCanHidden); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->canAttributes(FileInfo::FileCanType::kCanMoveOrCopy); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->canAttributes(FileInfo::FileCanType::kCanDelete); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->canAttributes(FileInfo::FileCanType::kCanTrash); });
}

TEST_F(FileInfoTest, IsAttributesMultipleTypes)
{
    QString filePath = rootPath + "/isattr.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    info->initQuerier();
    EXPECT_NO_FATAL_FAILURE({ (void)info->isAttributes(FileInfo::FileIsType::kIsFile); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->isAttributes(FileInfo::FileIsType::kIsReadable); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->isAttributes(FileInfo::FileIsType::kIsWritable); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->isAttributes(FileInfo::FileIsType::kIsHidden); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->isAttributes(FileInfo::FileIsType::kIsSymLink); });
}

TEST_F(FileInfoTest, ExtendAttributesMultipleTypes)
{
    QString filePath = rootPath + "/extendattr.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    info->initQuerier();
    EXPECT_NO_FATAL_FAILURE({ (void)info->extendAttributes(FileInfo::FileExtendedInfoType::kFileLocalDevice); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->extendAttributes(FileInfo::FileExtendedInfoType::kSizeFormat); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->extendAttributes(FileInfo::FileExtendedInfoType::kInode); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->extendAttributes(FileInfo::FileExtendedInfoType::kOwner); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->extendAttributes(FileInfo::FileExtendedInfoType::kGroup); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->extendAttributes(FileInfo::FileExtendedInfoType::kFileIsHid); });
}

TEST_F(FileInfoTest, ExtraPropertiesAndCountChildFile)
{
    QString dirPath = rootPath + "/propsdir";
    ASSERT_TRUE(QDir().mkpath(dirPath));
    QFile f(dirPath + "/child.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(dirPath));
    ASSERT_NE(info, nullptr);
    info->initQuerier();
    EXPECT_NO_FATAL_FAILURE({ (void)info->extraProperties(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info->countChildFile(); });
}

TEST_F(FileInfoTest, FileIcon)
{
    QString filePath = rootPath + "/icon.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    info->initQuerier();
    EXPECT_NO_FATAL_FAILURE({ (void)info->fileIcon(); });
}

TEST_F(FileInfoTest, FileMimeType)
{
    QString filePath = rootPath + "/mime.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("plain text");
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    info->initQuerier();
    EXPECT_NO_FATAL_FAILURE({ (void)info->fileMimeType(); });
}

TEST_F(FileInfoTest, PermissionCheck)
{
    QString filePath = rootPath + "/permcheck.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    auto info = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(filePath));
    ASSERT_NE(info, nullptr);
    info->initQuerier();
    EXPECT_NO_FATAL_FAILURE({ (void)info->permission(QFileDevice::ReadOwner); });
}

// ---- Coverage additions: SyncFileInfoPrivate getters + operators + 2-arg ctor.
// Relies on -fno-access-control (dfm_add_test) to reach the private `d` member.
TEST_F(FileInfoTest, SyncFileInfoPrivateGettersAfterSyncQuery)
{
    QString filePath = rootPath + "/archive.tar.gz";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("payload");
    f.close();

    SyncFileInfo info(QUrl::fromLocalFile(filePath));
    ASSERT_FALSE(info.d.isNull());
    ASSERT_FALSE(info.d->dfmFileInfo.isNull());
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->dfmFileInfo->initQuerier(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->baseName(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->completeSuffix(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->symLinkTarget(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->isExecutable(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->canFetch(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->redirectedFileUrl(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->iconName(); });
    EXPECT_NO_FATAL_FAILURE({ info.d->updateMediaInfo(DFMIO::DFileInfo::MediaType::kGeneral, {}); });
}

TEST_F(FileInfoTest, SyncFileInfoEqualityOperators)
{
    QString filePath = rootPath + "/eq.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();

    SyncFileInfo a(QUrl::fromLocalFile(filePath));
    SyncFileInfo b(QUrl::fromLocalFile(filePath));
    // Distinct DFileInfo instances -> not equal by the dfmFileInfo pointer rule.
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(a != b);
    SyncFileInfo &ref = a;
    EXPECT_TRUE(a == ref);
    EXPECT_FALSE(a != ref);
}

TEST_F(FileInfoTest, SyncFileInfoTwoArgConstructor)
{
    QString filePath = rootPath + "/twoarg.txt";
    QFile f(filePath);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.close();
    QUrl local = QUrl::fromLocalFile(filePath);
    QSharedPointer<DFMIO::DFileInfo> dfi(new DFMIO::DFileInfo(local));
    EXPECT_NO_FATAL_FAILURE({ (void)dfi->initQuerier(); });
    SyncFileInfo info(local, dfi);
    EXPECT_EQ(info.d->dfmFileInfo.data(), dfi.data());
}

// ---- Coverage additions: FileInfo base class operators + dtor ----

TEST_F(FileInfoTest, FileInfoEqualityOperators)
{
    QString p1 = rootPath + "/eq1.txt";
    QFile f1(p1); ASSERT_TRUE(f1.open(QIODevice::WriteOnly)); f1.close();
    FileInfo a(QUrl::fromLocalFile(p1));
    FileInfo b(QUrl::fromLocalFile(p1));
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
}

TEST_F(FileInfoTest, FileInfoAssignmentOperator)
{
    QString p1 = rootPath + "/assign.txt";
    QFile f1(p1); ASSERT_TRUE(f1.open(QIODevice::WriteOnly)); f1.close();
    FileInfo a(QUrl::fromLocalFile(p1));
    FileInfo b(QUrl::fromLocalFile(rootPath + "/other.txt"));
    b = a;
    EXPECT_EQ(b.urlOf(FileInfo::FileUrlInfoType::kUrl), a.urlOf(FileInfo::FileUrlInfoType::kUrl));
}
