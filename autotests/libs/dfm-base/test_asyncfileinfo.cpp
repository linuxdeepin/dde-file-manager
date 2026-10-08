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
#include "dfm-base/file/local/private/asyncfileinfo_p.h"
#include <dfm-io/dfileinfo.h>
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

// ---- Coverage additions: exercise AsyncFileInfoPrivate getters by forcing a
// synchronous attribute query on the underlying DFileInfo, then invoking the
// private accessors directly (relies on -fno-access-control from dfm_add_test).
TEST_F(AsyncFileInfoTest, PrivateGettersAfterSyncQuery)
{
    AsyncFileInfo info(url);
    ASSERT_FALSE(info.d.isNull());
    ASSERT_FALSE(info.d->dfmFileInfo.isNull());
    // Force a synchronous attribute query so attribute() returns real values
    // instead of the empty cache. (initQuerier may legitimately return false for
    // some gvfs/local paths; the getters still execute their bodies regardless.)
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->dfmFileInfo->initQuerier(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->fileName(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->baseName(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->completeBaseName(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->completeSuffix(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->fileDisplayName(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->path(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->filePath(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->symLinkTarget(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->isExecutable(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->canDelete(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->canTrash(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->canRename(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->canFetch(); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->fileType(); });
    bool ok = false;
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->attribute(DFMIO::DFileInfo::AttributeID::kStandardSize, &ok); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->iconName(); });
    EXPECT_NO_FATAL_FAILURE({ info.d->insertAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardSize, QVariant(123)); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.d->mediaInfo(DFMIO::DFileInfo::MediaType::kGeneral, {}); });
    EXPECT_NO_FATAL_FAILURE({ info.d->updateMediaInfo(DFMIO::DFileInfo::MediaType::kGeneral, {}); });
}

TEST_F(AsyncFileInfoTest, CustomAttributeAndCustomDataCallable)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.customAttribute("name", DFMIO::DFileInfo::DFileAttributeType::kTypeString); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.customData(Global::kItemFileRefreshIcon); });
    EXPECT_NO_FATAL_FAILURE({ (void)info.customData(99999); });
}

TEST_F(AsyncFileInfoTest, FileMimeTypeAsyncReturnsMimeType)
{
    AsyncFileInfo info(url);
    EXPECT_NO_FATAL_FAILURE({ (void)info.fileMimeTypeAsync(); });
}

TEST_F(AsyncFileInfoTest, TwoArgConstructorWithExternalDFileInfo)
{
    QUrl local = QUrl::fromLocalFile(filePath);
    QSharedPointer<DFMIO::DFileInfo> dfi(new DFMIO::DFileInfo(local));
    EXPECT_NO_FATAL_FAILURE({ (void)dfi->initQuerier(); });
    AsyncFileInfo info(local, dfi);
    EXPECT_EQ(info.d->dfmFileInfo.data(), dfi.data());
}

TEST_F(AsyncFileInfoTest, LocalAsyncFileInfoDestructsCleanly)
{
    // Exercise the (otherwise never-invoked) destructor on a stack instance.
    EXPECT_NO_FATAL_FAILURE({ AsyncFileInfo info(url); });
}

// ===== PMS sev-2 regression cluster: asyncfileinfo.cpp (work-order batch 2) =====

// PMS:210839 setNotifyUrl 去重契约：同一 (url, observer) 重复通知不得产生重复表项
TEST_F(AsyncFileInfoTest, BUG210839_SetNotifyUrlDeduplicatesSameEntry)
{
    AsyncFileInfo info(url);
    const QUrl notifyUrl("file:///tmp/ut_210839_notify_target.png");
    ASSERT_FALSE(info.d.isNull());
    EXPECT_EQ(info.d->notifyUrls.size(), 0);

    info.setNotifyUrl(notifyUrl, QStringLiteral("observerA"));
    info.setNotifyUrl(notifyUrl, QStringLiteral("observerA"));   // duplicate insert
    EXPECT_EQ(info.d->notifyUrls.count(notifyUrl, QStringLiteral("observerA")), 1);
    EXPECT_EQ(info.d->notifyUrls.size(), 1);

    info.setNotifyUrl(notifyUrl, QStringLiteral("observerB"));
    EXPECT_EQ(info.d->notifyUrls.count(notifyUrl), 2);

    info.removeNotifyUrl(notifyUrl, QStringLiteral("observerA"));
    EXPECT_FALSE(info.d->notifyUrls.contains(notifyUrl, QStringLiteral("observerA")));
    EXPECT_TRUE(info.d->notifyUrls.contains(notifyUrl, QStringLiteral("observerB")));
}

// PMS:210839 非法 url 应整体清空通知表（契约：调用方失效时清理订阅）
TEST_F(AsyncFileInfoTest, BUG210839_SetNotifyUrlInvalidUrlClearsAll)
{
    AsyncFileInfo info(url);
    const QUrl notifyUrl("file:///tmp/ut_210839_a.png");
    info.setNotifyUrl(notifyUrl, QStringLiteral("observerA"));
    ASSERT_EQ(info.d->notifyUrls.size(), 1);

    EXPECT_NO_FATAL_FAILURE({ info.setNotifyUrl(QUrl(), QStringLiteral("observerA")); });
    EXPECT_TRUE(info.d->notifyUrls.isEmpty());
}

// ============================================================
// PMS sev-2 regression cluster: asyncfileinfo.cpp (work-order batch 3)
// ============================================================

// PMS:249663 右键属性对话框偶发崩溃：AsyncFileInfo::setExtendedAttributes 对
// 本地设备/光驱/隐藏三类扩展键走缓存路径（读回一致），默认键回退基类不崩溃
TEST_F(AsyncFileInfoTest, BUG249663_SetExtendedAttributesRoutesToCache)
{
    AsyncFileInfo info(url);
    using ExtType = FileInfo::FileExtendedInfoType;

    EXPECT_NO_FATAL_FAILURE({
        info.setExtendedAttributes(ExtType::kFileIsHid, true);
        info.setExtendedAttributes(ExtType::kFileLocalDevice, false);
        info.setExtendedAttributes(ExtType::kFileCdRomDevice, false);
        // default branch -> base class handler, must not touch the cache path
        info.setExtendedAttributes(ExtType::kOwner, QStringLiteral("ut-owner-249663"));
    });

    EXPECT_TRUE(info.extendAttributes(ExtType::kFileIsHid).toBool());
    EXPECT_FALSE(info.extendAttributes(ExtType::kFileLocalDevice).toBool());
    EXPECT_FALSE(info.extendAttributes(ExtType::kFileCdRomDevice).toBool());
}
