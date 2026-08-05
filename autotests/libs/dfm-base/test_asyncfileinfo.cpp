// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_asyncfileinfo.cpp
 * @brief Unit tests for AsyncFileInfo (asyncfileinfo.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QIcon>
#include <QMutexLocker>
#include <mutex>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

using namespace dfmbase;

class AsyncFileInfoTest : public testing::Test
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
        filePath = rootPath + "/async.txt";
        QFile f(filePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("async content");
        f.close();
        url = QUrl::fromLocalFile(filePath);
    }

    QTemporaryDir tmpDir;
    QString rootPath;
    QString filePath;
    QUrl url;
    static std::once_flag flag;
};

std::once_flag AsyncFileInfoTest::flag;

TEST_F(AsyncFileInfoTest, ConstructAndQueryBasics)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.exists(); });
    EXPECT_NO_FATAL_FAILURE({ info.refresh(); });
    EXPECT_NO_FATAL_FAILURE({ info.cacheAttribute(DFMIO::DFileInfo::AttributeID::kStandardName, QVariant()); });
}

TEST_F(AsyncFileInfoTest, NameOfAllTypes)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({
        (void)info.nameOf(FileInfo::FileNameInfoType::kFileName);
        (void)info.nameOf(FileInfo::FileNameInfoType::kBaseName);
        (void)info.nameOf(FileInfo::FileNameInfoType::kCompleteBaseName);
        (void)info.nameOf(FileInfo::FileNameInfoType::kCompleteSuffix);
        (void)info.nameOf(FileInfo::FileNameInfoType::kFileCopyName);
        (void)info.nameOf(FileInfo::FileNameInfoType::kIconName);
        (void)info.nameOf(FileInfo::FileNameInfoType::kGenericIconName);
        (void)info.nameOf(FileInfo::FileNameInfoType::kMimeTypeName);
        (void)info.nameOf(FileInfo::FileNameInfoType::kFileNameOfRename);
    });
}

TEST_F(AsyncFileInfoTest, PathOfAllTypes)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({
        (void)info.pathOf(FileInfo::FilePathInfoType::kFilePath);
        (void)info.pathOf(FileInfo::FilePathInfoType::kAbsoluteFilePath);
        (void)info.pathOf(FileInfo::FilePathInfoType::kPath);
        (void)info.pathOf(FileInfo::FilePathInfoType::kAbsolutePath);
        (void)info.pathOf(FileInfo::FilePathInfoType::kSymLinkTarget);
        (void)info.pathOf(FileInfo::FilePathInfoType::kCanonicalPath);
    });
}

TEST_F(AsyncFileInfoTest, UrlOfAllTypes)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({
        (void)info.urlOf(FileInfo::FileUrlInfoType::kUrl);
        (void)info.urlOf(FileInfo::FileUrlInfoType::kRedirectedFileUrl);
        (void)info.urlOf(FileInfo::FileUrlInfoType::kOriginalUrl);
        (void)info.urlOf(FileInfo::FileUrlInfoType::kParentUrl);
    });
}

TEST_F(AsyncFileInfoTest, IsAttributesAllTypes)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({
        (void)info.isAttributes(FileInfo::FileIsType::kIsFile);
        (void)info.isAttributes(FileInfo::FileIsType::kIsDir);
        (void)info.isAttributes(FileInfo::FileIsType::kIsReadable);
        (void)info.isAttributes(FileInfo::FileIsType::kIsWritable);
        (void)info.isAttributes(FileInfo::FileIsType::kIsHidden);
        (void)info.isAttributes(FileInfo::FileIsType::kIsSymLink);
        (void)info.isAttributes(FileInfo::FileIsType::kIsExecutable);
        (void)info.isAttributes(FileInfo::FileIsType::kIsRoot);
        (void)info.isAttributes(FileInfo::FileIsType::kIsBundle);
    });
}

TEST_F(AsyncFileInfoTest, CanAttributesAllTypes)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({
        (void)info.canAttributes(FileInfo::FileCanType::kCanDelete);
        (void)info.canAttributes(FileInfo::FileCanType::kCanTrash);
        (void)info.canAttributes(FileInfo::FileCanType::kCanRename);
        (void)info.canAttributes(FileInfo::FileCanType::kCanHidden);
        (void)info.canAttributes(FileInfo::FileCanType::kCanMoveOrCopy);
        (void)info.canAttributes(FileInfo::FileCanType::kCanDrop);
    });
}

TEST_F(AsyncFileInfoTest, ExtendAttributesAllTypes)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({
        (void)info.extendAttributes(FileInfo::FileExtendedInfoType::kFileLocalDevice);
        (void)info.extendAttributes(FileInfo::FileExtendedInfoType::kFileCdRomDevice);
        (void)info.extendAttributes(FileInfo::FileExtendedInfoType::kSizeFormat);
        (void)info.extendAttributes(FileInfo::FileExtendedInfoType::kInode);
        (void)info.extendAttributes(FileInfo::FileExtendedInfoType::kOwner);
        (void)info.extendAttributes(FileInfo::FileExtendedInfoType::kGroup);
        (void)info.extendAttributes(FileInfo::FileExtendedInfoType::kFileIsHid);
        (void)info.extendAttributes(FileInfo::FileExtendedInfoType::kOwnerId);
        (void)info.extendAttributes(FileInfo::FileExtendedInfoType::kGroupId);
    });
}

TEST_F(AsyncFileInfoTest, PermissionAndSizeAndTime)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.permission(QFileDevice::ReadOwner); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.permissions(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.size(); });
    EXPECT_NO_FATAL_FAILURE({
        (void)info.timeOf(FileInfo::FileTimeType::kCreateTime);
        (void)info.timeOf(FileInfo::FileTimeType::kBirthTime);
        (void)info.timeOf(FileInfo::FileTimeType::kMetadataChangeTime);
        (void)info.timeOf(FileInfo::FileTimeType::kLastModified);
        (void)info.timeOf(FileInfo::FileTimeType::kLastRead);
        (void)info.timeOf(FileInfo::FileTimeType::kDeletionTime);
        (void)info.timeOf(FileInfo::FileTimeType::kLastModifiedSecond);
    });
}

TEST_F(AsyncFileInfoTest, CountChildFileAndDisplayAndExtra)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.countChildFile(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.countChildFileAsync(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.displayOf(FileInfo::DisplayInfoType::kFileDisplayName); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.displayOf(FileInfo::DisplayInfoType::kSizeDisplayName); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.extraProperties(); });
}

TEST_F(AsyncFileInfoTest, ViewOfTip)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.viewOfTip(FileInfo::ViewType::kEmptyDir); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.viewOfTip(FileInfo::ViewType::kLoading); });
}

TEST_F(AsyncFileInfoTest, ExtendedAttributesAndCache)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ info.setExtendedAttributes(FileInfo::FileExtendedInfoType::kOwner, QVariant("root")); });
    EXPECT_NO_FATAL_FAILURE({ info.updateAttributes({}); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.asyncQueryDfmFileInfo(0, nullptr, nullptr); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.errorCodeFromDfmio(); });
}

TEST_F(AsyncFileInfoTest, NotifyUrls)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.notifyUrls(); });
    EXPECT_NO_FATAL_FAILURE({ info.setNotifyUrl(QUrl("file:///tmp/x"), "ptr"); });
    EXPECT_NO_FATAL_FAILURE({ info.removeNotifyUrl(QUrl("file:///tmp/x"), "ptr"); });
}

TEST_F(AsyncFileInfoTest, GetUrlByType)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.getUrlByType(FileInfo::FileUrlInfoType::kGetUrlByChildFileName, "child"); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.fileType(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.supportedOfAttributes(FileInfo::SupportType::kDrag); });
}
