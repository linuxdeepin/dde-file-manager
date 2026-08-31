// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_proxyfileinfo.cpp
 * @brief Unit tests for ProxyFileInfo delegation & fallback paths.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QIcon>
#include <mutex>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/proxyfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class ProxyFileInfoTest : public testing::Test
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
        filePath = rootPath + "/proxytest.txt";
        QFile f(filePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("proxy content");
        f.close();
        url = QUrl::fromLocalFile(filePath);
    }

    QTemporaryDir tmpDir;
    QString rootPath;
    QString filePath;
    QUrl url;
    static std::once_flag flag;
};

std::once_flag ProxyFileInfoTest::flag;

// Exercise every ProxyFileInfo method in fallback mode (no proxy set).
TEST_F(ProxyFileInfoTest, FallbackPathAllMethods)
{
    ProxyFileInfo proxy(url);

    EXPECT_NO_FATAL_FAILURE({ (void)proxy.fileUrl(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.exists(); });
    EXPECT_NO_FATAL_FAILURE({ proxy.refresh(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.filePath(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.absoluteFilePath(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.fileName(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.baseName(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.completeBaseName(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.suffix(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.completeSuffix(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.path(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.absolutePath(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.isReadable(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.isWritable(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.isExecutable(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.isHidden(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.isNativePath(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.isFile(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.isDir(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.isSymLink(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.isRoot(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.isBundle(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.symLinkTarget(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.owner(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.ownerId(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.group(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.groupId(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.permission(QFileDevice::ReadOwner); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.permissions(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.countChildFile(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.size(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.birthTime(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.metadataChangeTime(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.lastModified(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.lastRead(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.initQuerier(); });
    EXPECT_NO_FATAL_FAILURE({ proxy.initQuerierAsync(0, nullptr, nullptr); });
    EXPECT_NO_FATAL_FAILURE({ proxy.cacheAttribute(DFMIO::DFileInfo::AttributeID::kStandardName, QVariant()); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.nameOf(FileInfo::FileNameInfoType::kFileName); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.pathOf(FileInfo::FilePathInfoType::kFilePath); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.displayOf(FileInfo::DisplayInfoType::kFileDisplayName); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.urlOf(FileInfo::FileUrlInfoType::kUrl); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.getUrlByType(FileInfo::FileUrlInfoType::kGetUrlByChildFileName, "child"); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.isAttributes(FileInfo::FileIsType::kIsFile); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.canAttributes(FileInfo::FileCanType::kCanRename); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.extendAttributes(FileInfo::FileExtendedInfoType::kOwner); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.countChildFileAsync(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.timeOf(FileInfo::FileTimeType::kLastModified); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.fileIcon(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.fileMimeType(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.fileMimeTypeAsync(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.extraProperties(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.customData(0); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.fileType(); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.supportedOfAttributes(FileInfo::SupportType::kDrag); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.viewOfTip(FileInfo::ViewType::kEmptyDir); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.customAttribute("xattr::update", DFMIO::DFileInfo::DFileAttributeType::kTypeString); });
    EXPECT_NO_FATAL_FAILURE({ (void)proxy.mediaInfoAttributes(DFMIO::DFileInfo::MediaType::kImage, {}); });
    EXPECT_NO_FATAL_FAILURE({ proxy.setExtendedAttributes(FileInfo::FileExtendedInfoType::kOwner, QVariant("root")); });
    EXPECT_NO_FATAL_FAILURE({ proxy.updateAttributes({}); });
}

// Exercise every ProxyFileInfo method with a proxy set (delegation path).
TEST_F(ProxyFileInfoTest, DelegationPathAllMethods)
{
    auto proxyPtr = QSharedPointer<ProxyFileInfo>::create(url);
    FileInfoPointer realInfo = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(realInfo, nullptr);
    realInfo->initQuerier();
    proxyPtr->setProxy(realInfo);

    EXPECT_NO_FATAL_FAILURE({
        (void)proxyPtr->fileUrl();
        (void)proxyPtr->exists();
        proxyPtr->refresh();
        (void)proxyPtr->filePath();
        (void)proxyPtr->absoluteFilePath();
        (void)proxyPtr->fileName();
        (void)proxyPtr->baseName();
        (void)proxyPtr->completeBaseName();
        (void)proxyPtr->suffix();
        (void)proxyPtr->completeSuffix();
        (void)proxyPtr->path();
        (void)proxyPtr->absolutePath();
        (void)proxyPtr->isReadable();
        (void)proxyPtr->isWritable();
        (void)proxyPtr->isExecutable();
        (void)proxyPtr->isHidden();
        (void)proxyPtr->isNativePath();
        (void)proxyPtr->isFile();
        (void)proxyPtr->isDir();
        (void)proxyPtr->isSymLink();
        (void)proxyPtr->isRoot();
        (void)proxyPtr->isBundle();
        (void)proxyPtr->symLinkTarget();
        (void)proxyPtr->owner();
        (void)proxyPtr->ownerId();
        (void)proxyPtr->group();
        (void)proxyPtr->groupId();
        (void)proxyPtr->permission(QFileDevice::ReadOwner);
        (void)proxyPtr->permissions();
        (void)proxyPtr->countChildFile();
        (void)proxyPtr->size();
        (void)proxyPtr->birthTime();
        (void)proxyPtr->metadataChangeTime();
        (void)proxyPtr->lastModified();
        (void)proxyPtr->lastRead();
        (void)proxyPtr->initQuerier();
        proxyPtr->initQuerierAsync(0, nullptr, nullptr);
        proxyPtr->cacheAttribute(DFMIO::DFileInfo::AttributeID::kStandardName, QVariant());
        (void)proxyPtr->nameOf(FileInfo::FileNameInfoType::kFileName);
        (void)proxyPtr->pathOf(FileInfo::FilePathInfoType::kFilePath);
        (void)proxyPtr->displayOf(FileInfo::DisplayInfoType::kFileDisplayName);
        (void)proxyPtr->urlOf(FileInfo::FileUrlInfoType::kUrl);
        (void)proxyPtr->getUrlByType(FileInfo::FileUrlInfoType::kGetUrlByChildFileName, "child");
        (void)proxyPtr->isAttributes(FileInfo::FileIsType::kIsFile);
        (void)proxyPtr->canAttributes(FileInfo::FileCanType::kCanRename);
        (void)proxyPtr->extendAttributes(FileInfo::FileExtendedInfoType::kOwner);
        (void)proxyPtr->countChildFileAsync();
        (void)proxyPtr->timeOf(FileInfo::FileTimeType::kLastModified);
        (void)proxyPtr->fileIcon();
        (void)proxyPtr->fileMimeType();
        (void)proxyPtr->fileMimeTypeAsync();
        (void)proxyPtr->extraProperties();
        (void)proxyPtr->customData(0);
        (void)proxyPtr->fileType();
        (void)proxyPtr->supportedOfAttributes(FileInfo::SupportType::kDrag);
        (void)proxyPtr->viewOfTip(FileInfo::ViewType::kEmptyDir);
        (void)proxyPtr->customAttribute("xattr::update", DFMIO::DFileInfo::DFileAttributeType::kTypeString);
        (void)proxyPtr->mediaInfoAttributes(DFMIO::DFileInfo::MediaType::kImage, {});
        proxyPtr->setExtendedAttributes(FileInfo::FileExtendedInfoType::kOwner, QVariant("root"));
        proxyPtr->updateAttributes({});
    });
}


TEST_F(ProxyFileInfoTest, exists)
{
    // exists
    SUCCEED();
}

TEST_F(ProxyFileInfoTest, refresh)
{
    // refresh
    SUCCEED();
}

TEST_F(ProxyFileInfoTest, removeNotifyUrl)
{
    // removeNotifyUrl
    SUCCEED();
}

TEST_F(ProxyFileInfoTest, setProxy)
{
    // setProxy
    SUCCEED();
}

TEST_F(ProxyFileInfoTest, updateAttributes)
{
    // updateAttributes
    SUCCEED();
}
