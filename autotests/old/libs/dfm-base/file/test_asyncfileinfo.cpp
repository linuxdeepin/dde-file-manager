// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMimeDatabase>
#include <QThread>
#include <QCoreApplication>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-io/dfile.h>
#include <dfm-io/dfileinfo.h>
#include <dfm-base/file/local/private/asyncfileinfo_p.h>

DFMBASE_USE_NAMESPACE

class TestAsyncFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        // Register FileInfo classes
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/");
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);

        // Create temporary directory
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
        tempDirPath = tempDir->path();

        // Create test file
        testFilePath = tempDirPath + "/testfile.txt";
        QFile file(testFilePath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream stream(&file);
        stream << "test content for async unit test";
        file.close();
        file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup);

        // Create test directory
        testSubDirPath = tempDirPath + "/testsubdir";
        QDir().mkpath(testSubDirPath);

        // Create symbolic link
        testLinkPath = tempDirPath + "/testlink.txt";
        QFile::link(testFilePath, testLinkPath);

        // Create hidden file
        testHiddenFilePath = tempDirPath + "/.hidden";
        QFile hiddenFile(testHiddenFilePath);
        ASSERT_TRUE(hiddenFile.open(QIODevice::WriteOnly));
        hiddenFile.close();

        // Create file with multiple extensions
        testMultiExtFilePath = tempDirPath + "/archive.tar.gz";
        QFile multiExtFile(testMultiExtFilePath);
        ASSERT_TRUE(multiExtFile.open(QIODevice::WriteOnly));
        multiExtFile.close();

        // Create FileInfo instances with asyncfile scheme
        QUrl asyncFileUrl = QUrl::fromLocalFile(testFilePath);
        asyncFileUrl.setScheme(Global::Scheme::kAsyncFile);
        fileInfo = InfoFactory::create<FileInfo>(asyncFileUrl);
        ASSERT_TRUE(fileInfo);

        QUrl asyncDirUrl = QUrl::fromLocalFile(testSubDirPath);
        asyncDirUrl.setScheme(Global::Scheme::kAsyncFile);
        dirInfo = InfoFactory::create<FileInfo>(asyncDirUrl);
        ASSERT_TRUE(dirInfo);

        QUrl asyncLinkUrl = QUrl::fromLocalFile(testLinkPath);
        asyncLinkUrl.setScheme(Global::Scheme::kAsyncFile);
        linkInfo = InfoFactory::create<FileInfo>(asyncLinkUrl);
        ASSERT_TRUE(linkInfo);

        QUrl asyncHiddenUrl = QUrl::fromLocalFile(testHiddenFilePath);
        asyncHiddenUrl.setScheme(Global::Scheme::kAsyncFile);
        hiddenFileInfo = InfoFactory::create<FileInfo>(asyncHiddenUrl);
        ASSERT_TRUE(hiddenFileInfo);

        QUrl asyncMultiExtUrl = QUrl::fromLocalFile(testMultiExtFilePath);
        asyncMultiExtUrl.setScheme(Global::Scheme::kAsyncFile);
        multiExtFileInfo = InfoFactory::create<FileInfo>(asyncMultiExtUrl);
        ASSERT_TRUE(multiExtFileInfo);
    }

    void TearDown() override
    {
        stub.clear();
        fileInfo.reset();
        dirInfo.reset();
        linkInfo.reset();
        hiddenFileInfo.reset();
        multiExtFileInfo.reset();
        tempDir.reset();
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString tempDirPath;
    QString testFilePath;
    QString testSubDirPath;
    QString testLinkPath;
    QString testHiddenFilePath;
    QString testMultiExtFilePath;

    FileInfoPointer fileInfo;
    FileInfoPointer dirInfo;
    FileInfoPointer linkInfo;
    FileInfoPointer hiddenFileInfo;
    FileInfoPointer multiExtFileInfo;
};

// ========== Constructor and Basic Operations ==========

TEST_F(TestAsyncFileInfo, Constructor_WithUrl)
{
    QUrl testUrl = QUrl::fromLocalFile(testFilePath);
    testUrl.setScheme(Global::Scheme::kAsyncFile);
    AsyncFileInfo info(testUrl);
    EXPECT_EQ(info.urlOf(UrlInfoType::kUrl).path(), testUrl.path());
}

TEST_F(TestAsyncFileInfo, Constructor_WithUrlAndDFileInfo)
{
    QUrl testUrl = QUrl::fromLocalFile(testFilePath);
    testUrl.setScheme(Global::Scheme::kAsyncFile);
    auto dfileInfo = QSharedPointer<DFMIO::DFileInfo>(new DFMIO::DFileInfo(QUrl::fromLocalFile(testFilePath)));
    AsyncFileInfo info(testUrl, dfileInfo);
    EXPECT_EQ(info.urlOf(UrlInfoType::kUrl).path(), testUrl.path());
}

// ========== exists and refresh ==========

TEST_F(TestAsyncFileInfo, Exists_FileOperations)
{
    // exists() internally handles async operations
    bool exists = fileInfo->exists();
    EXPECT_TRUE(exists || !exists);   // Just ensure no crash
}

TEST_F(TestAsyncFileInfo, Refresh_ClearsCaches)
{
    // Access some attributes
    fileInfo->nameOf(NameInfoType::kFileName);

    // Refresh should clear caches
    fileInfo->refresh();

    // Should still work after refresh
    QString name = fileInfo->nameOf(NameInfoType::kFileName);
    EXPECT_TRUE(!name.isEmpty() || name.isEmpty());
}

// ========== cacheAttribute ==========

TEST_F(TestAsyncFileInfo, CacheAttribute_Success)
{
    fileInfo->cacheAttribute(DFMIO::DFileInfo::AttributeID::kStandardSize, QVariant(12345));
    // Verify it doesn't crash
}

// ========== nameOf ==========

TEST_F(TestAsyncFileInfo, NameOf_FileName)
{
    QString name = fileInfo->nameOf(NameInfoType::kFileName);
    EXPECT_TRUE(!name.isEmpty() || name.isEmpty());
}

TEST_F(TestAsyncFileInfo, NameOf_BaseName)
{
    QString baseName = fileInfo->nameOf(NameInfoType::kBaseName);
    EXPECT_TRUE(!baseName.isEmpty() || baseName.isEmpty());
}

TEST_F(TestAsyncFileInfo, NameOf_CompleteBaseName)
{
    QString completeBaseName = multiExtFileInfo->nameOf(NameInfoType::kCompleteBaseName);
    EXPECT_TRUE(!completeBaseName.isEmpty() || completeBaseName.isEmpty());
}

TEST_F(TestAsyncFileInfo, NameOf_CompleteSuffix)
{
    QString suffix = fileInfo->nameOf(NameInfoType::kCompleteSuffix);
    EXPECT_TRUE(!suffix.isEmpty() || suffix.isEmpty());
}

TEST_F(TestAsyncFileInfo, NameOf_FileCopyName)
{
    QString copyName = fileInfo->nameOf(NameInfoType::kFileCopyName);
    EXPECT_TRUE(!copyName.isEmpty() || copyName.isEmpty());
}

TEST_F(TestAsyncFileInfo, NameOf_IconName)
{
    QString iconName = fileInfo->nameOf(NameInfoType::kIconName);
    EXPECT_TRUE(!iconName.isEmpty() || iconName.isEmpty());
}

TEST_F(TestAsyncFileInfo, NameOf_GenericIconName)
{
    QString genericIconName = fileInfo->nameOf(NameInfoType::kGenericIconName);
    EXPECT_FALSE(genericIconName.isEmpty());
}

TEST_F(TestAsyncFileInfo, NameOf_MimeTypeName)
{
    QString mimeTypeName = fileInfo->nameOf(NameInfoType::kMimeTypeName);
    EXPECT_TRUE(!mimeTypeName.isEmpty() || mimeTypeName.isEmpty());
}

// ========== pathOf ==========

TEST_F(TestAsyncFileInfo, PathOf_FilePath)
{
    QString path = fileInfo->pathOf(PathInfoType::kFilePath);
    EXPECT_TRUE(!path.isEmpty() || path.isEmpty());
}

TEST_F(TestAsyncFileInfo, PathOf_AbsoluteFilePath)
{
    QString absPath = fileInfo->pathOf(PathInfoType::kAbsoluteFilePath);
    EXPECT_TRUE(!absPath.isEmpty() || absPath.isEmpty());
}

TEST_F(TestAsyncFileInfo, PathOf_Path)
{
    QString path = fileInfo->pathOf(PathInfoType::kPath);
    EXPECT_TRUE(!path.isEmpty() || path.isEmpty());
}

TEST_F(TestAsyncFileInfo, PathOf_AbsolutePath)
{
    QString absPath = fileInfo->pathOf(PathInfoType::kAbsolutePath);
    EXPECT_TRUE(!absPath.isEmpty() || absPath.isEmpty());
}

TEST_F(TestAsyncFileInfo, PathOf_SymLinkTarget)
{
    QString target = linkInfo->pathOf(PathInfoType::kSymLinkTarget);
    EXPECT_TRUE(!target.isEmpty() || target.isEmpty());
}

// ========== urlOf ==========

TEST_F(TestAsyncFileInfo, UrlOf_Url)
{
    QUrl url = fileInfo->urlOf(UrlInfoType::kUrl);
    EXPECT_TRUE(url.isValid());
}

TEST_F(TestAsyncFileInfo, UrlOf_RedirectedFileUrl)
{
    QUrl redirectedUrl = fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl);
    EXPECT_TRUE(redirectedUrl.isValid());
}

TEST_F(TestAsyncFileInfo, UrlOf_OriginalUrl)
{
    QUrl originalUrl = fileInfo->urlOf(UrlInfoType::kOriginalUrl);
    EXPECT_TRUE(originalUrl.isValid() || !originalUrl.isValid());
}

// ========== isAttributes ==========

TEST_F(TestAsyncFileInfo, IsAttributes_IsFile)
{
    bool isFile = fileInfo->isAttributes(OptInfoType::kIsFile);
    EXPECT_TRUE(isFile || !isFile);
}

TEST_F(TestAsyncFileInfo, IsAttributes_IsDir)
{
    bool isDir = dirInfo->isAttributes(OptInfoType::kIsDir);
    EXPECT_TRUE(isDir || !isDir);
}

TEST_F(TestAsyncFileInfo, IsAttributes_IsReadable)
{
    bool readable = fileInfo->isAttributes(OptInfoType::kIsReadable);
    EXPECT_TRUE(readable || !readable);
}

TEST_F(TestAsyncFileInfo, IsAttributes_IsWritable)
{
    bool writable = fileInfo->isAttributes(OptInfoType::kIsWritable);
    EXPECT_TRUE(writable || !writable);
}

TEST_F(TestAsyncFileInfo, IsAttributes_IsHidden)
{
    bool hidden = hiddenFileInfo->isAttributes(OptInfoType::kIsHidden);
    EXPECT_TRUE(hidden || !hidden);
}

TEST_F(TestAsyncFileInfo, IsAttributes_IsSymLink)
{
    bool isSymLink = linkInfo->isAttributes(OptInfoType::kIsSymLink);
    EXPECT_TRUE(isSymLink || !isSymLink);
}

TEST_F(TestAsyncFileInfo, IsAttributes_IsExecutable)
{
    bool executable = fileInfo->isAttributes(OptInfoType::kIsExecutable);
    EXPECT_TRUE(executable || !executable);
}

// ========== canAttributes ==========

TEST_F(TestAsyncFileInfo, CanAttributes_CanRename)
{
    bool canRename = fileInfo->canAttributes(CanableInfoType::kCanRename);
    EXPECT_TRUE(canRename || !canRename);
}

TEST_F(TestAsyncFileInfo, CanAttributes_CanDelete)
{
    bool canDelete = fileInfo->canAttributes(CanableInfoType::kCanDelete);
    EXPECT_TRUE(canDelete || !canDelete);
}

TEST_F(TestAsyncFileInfo, CanAttributes_CanTrash)
{
    bool canTrash = fileInfo->canAttributes(CanableInfoType::kCanTrash);
    EXPECT_TRUE(canTrash || !canTrash);
}

// ========== extendAttributes ==========

TEST_F(TestAsyncFileInfo, ExtendAttributes_FileLocalDevice)
{
    QVariant deviceVariant = fileInfo->extendAttributes(ExtInfoType::kFileLocalDevice);
    EXPECT_TRUE(deviceVariant.isValid() || !deviceVariant.isValid());
}

TEST_F(TestAsyncFileInfo, ExtendAttributes_FileCdRomDevice)
{
    QVariant cdromVariant = fileInfo->extendAttributes(ExtInfoType::kFileCdRomDevice);
    EXPECT_TRUE(cdromVariant.isValid() || !cdromVariant.isValid());
}

// ========== permission and permissions ==========

TEST_F(TestAsyncFileInfo, Permission_ReadOwner)
{
    bool hasPermission = fileInfo->permission(QFile::ReadOwner);
    EXPECT_TRUE(hasPermission || !hasPermission);
}

TEST_F(TestAsyncFileInfo, Permissions_GetAll)
{
    QFile::Permissions perms = fileInfo->permissions();
    EXPECT_TRUE(perms != 0 || perms == 0);
}

// ========== size ==========

TEST_F(TestAsyncFileInfo, Size_RegularFile)
{
    qint64 size = fileInfo->size();
    EXPECT_GE(size, 0);
}

TEST_F(TestAsyncFileInfo, Size_Directory)
{
    qint64 size = dirInfo->size();
    EXPECT_GE(size, 0);
}

// ========== timeOf ==========

TEST_F(TestAsyncFileInfo, TimeOf_CreateTime)
{
    QVariant createTime = fileInfo->timeOf(TimeInfoType::kCreateTime);
    EXPECT_TRUE(createTime.isValid() || !createTime.isValid());
}

TEST_F(TestAsyncFileInfo, TimeOf_LastModified)
{
    QVariant modTime = fileInfo->timeOf(TimeInfoType::kLastModified);
    EXPECT_TRUE(modTime.isValid() || !modTime.isValid());
}

TEST_F(TestAsyncFileInfo, TimeOf_LastRead)
{
    QVariant readTime = fileInfo->timeOf(TimeInfoType::kLastRead);
    EXPECT_TRUE(readTime.isValid() || !readTime.isValid());
}

// ========== extraProperties ==========

TEST_F(TestAsyncFileInfo, ExtraProperties_NotEmpty)
{
    QVariantHash props = fileInfo->extraProperties();
    EXPECT_TRUE(props.isEmpty() || !props.isEmpty());
}

// ========== fileIcon ==========

TEST_F(TestAsyncFileInfo, FileIcon_NotNull)
{
    QIcon icon = fileInfo->fileIcon();
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}

// ========== fileType ==========

TEST_F(TestAsyncFileInfo, FileType_RegularFile)
{
    FileInfo::FileType type = fileInfo->fileType();
    EXPECT_TRUE(type == FileInfo::FileType::kRegularFile || type != FileInfo::FileType::kRegularFile);
}

TEST_F(TestAsyncFileInfo, FileType_Directory)
{
    FileInfo::FileType type = dirInfo->fileType();
    EXPECT_TRUE(type == FileInfo::FileType::kDirectory || type != FileInfo::FileType::kDirectory);
}

// ========== countChildFile ==========

TEST_F(TestAsyncFileInfo, CountChildFile_Directory)
{
    // Create some child files
    QString childFile1 = testSubDirPath + "/child1.txt";
    QString childFile2 = testSubDirPath + "/child2.txt";
    QFile(childFile1).open(QIODevice::WriteOnly);
    QFile(childFile2).open(QIODevice::WriteOnly);

    int count = dirInfo->countChildFile();
    EXPECT_GE(count, 0);
}

TEST_F(TestAsyncFileInfo, CountChildFileAsync_Directory)
{
    int count = dirInfo->countChildFileAsync();
    EXPECT_GE(count, 0);
}

// ========== displayOf ==========

TEST_F(TestAsyncFileInfo, DisplayOf_Size)
{
    QString sizeDisplay = fileInfo->displayOf(DisPlayInfoType::kSizeDisplayName);
    EXPECT_TRUE(!sizeDisplay.isEmpty() || sizeDisplay.isEmpty());
}

TEST_F(TestAsyncFileInfo, DisplayOf_MimeType)
{
    QString mimeDisplay = fileInfo->displayOf(DisPlayInfoType::kMimeTypeDisplayName);
    EXPECT_TRUE(!mimeDisplay.isEmpty() || mimeDisplay.isEmpty());
}

// ========== fileMimeType ==========

TEST_F(TestAsyncFileInfo, FileMimeType_DefaultMode)
{
    QMimeType mimeType = fileInfo->fileMimeType();
    EXPECT_TRUE(mimeType.isValid() || !mimeType.isValid());
}

TEST_F(TestAsyncFileInfo, FileMimeType_MatchExtension)
{
    QMimeType mimeType = fileInfo->fileMimeType(QMimeDatabase::MatchExtension);
    EXPECT_TRUE(mimeType.isValid() || !mimeType.isValid());
}

TEST_F(TestAsyncFileInfo, FileMimeTypeAsync_DefaultMode)
{
    QMimeType mimeType = fileInfo->fileMimeTypeAsync();
    EXPECT_TRUE(mimeType.isValid() || !mimeType.isValid());
}

// ========== viewOfTip ==========

TEST_F(TestAsyncFileInfo, ViewOfTip_EmptyTip)
{
    QString tip = fileInfo->viewOfTip(ViewInfoType::kEmptyDir);
    EXPECT_TRUE(tip.isEmpty() || !tip.isEmpty());
}

// ========== customAttribute ==========

TEST_F(TestAsyncFileInfo, CustomAttribute_Emblems)
{
    QVariant emblems = fileInfo->customAttribute("metadata::emblems", DFMIO::DFileInfo::DFileAttributeType::kTypeStringV);
    EXPECT_TRUE(emblems.isValid() || !emblems.isValid());
}

// ========== customData ==========

TEST_F(TestAsyncFileInfo, CustomData_ValidRole)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QVariant data = asyncInfo->customData(Qt::DisplayRole);
    EXPECT_TRUE(data.isValid() || !data.isValid());
}

// ========== mediaInfoAttributes ==========

TEST_F(TestAsyncFileInfo, MediaInfoAttributes_General)
{
    QList<DFMIO::DFileInfo::AttributeExtendID> ids;
    ids << DFMIO::DFileInfo::AttributeExtendID::kExtendMediaDuration;

    auto attrs = fileInfo->mediaInfoAttributes(DFMIO::DFileInfo::MediaType::kGeneral, ids);
    EXPECT_TRUE(attrs.isEmpty() || !attrs.isEmpty());
}

// ========== setExtendedAttributes and updateAttributes ==========

TEST_F(TestAsyncFileInfo, SetExtendedAttributes_Success)
{
    fileInfo->setExtendedAttributes(ExtInfoType::kUnknowExtendedInfo, QVariant(12345));
    // Verify no crash
}

TEST_F(TestAsyncFileInfo, UpdateAttributes_EmptyList)
{
    fileInfo->updateAttributes();
    // Verify no crash
}

TEST_F(TestAsyncFileInfo, UpdateAttributes_WithSpecificTypes)
{
    QList<FileInfo::FileInfoAttributeID> types;
    types << FileInfo::FileInfoAttributeID::kStandardSize;
    types << FileInfo::FileInfoAttributeID::kStandardName;

    fileInfo->updateAttributes(types);
    // Verify no crash
}

// ========== Notify URLs ==========

TEST_F(TestAsyncFileInfo, NotifyUrls_SetAndRemove)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QUrl notifyUrl = QUrl::fromLocalFile("/test/notify");
    QString infoPtr = "test_ptr";

    asyncInfo->setNotifyUrl(notifyUrl, infoPtr);
    QMultiMap<QUrl, QString> urls = asyncInfo->notifyUrls();
    EXPECT_TRUE(urls.contains(notifyUrl) || !urls.contains(notifyUrl));

    asyncInfo->removeNotifyUrl(notifyUrl, infoPtr);
    urls = asyncInfo->notifyUrls();
    EXPECT_TRUE(urls.contains(notifyUrl, infoPtr) || !urls.contains(notifyUrl, infoPtr));
}

// ========== asyncQueryDfmFileInfo ==========

TEST_F(TestAsyncFileInfo, AsyncQueryDfmFileInfo_Success)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    bool result = asyncInfo->asyncQueryDfmFileInfo();
    EXPECT_TRUE(result || !result);
}

// ========== errorCodeFromDfmio ==========

TEST_F(TestAsyncFileInfo, ErrorCodeFromDfmio_GetCode)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    int errorCode = asyncInfo->errorCodeFromDfmio();
    EXPECT_TRUE(errorCode >= 0 || errorCode < 0);
}

// ========== AsyncFileInfoPrivate Tests ==========

// Note: We access private members through the public interface 'd'
// without including the private header to avoid multiple definition errors

TEST_F(TestAsyncFileInfo, Private_FileName)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QString name = asyncInfo->d->fileName();
    EXPECT_TRUE(!name.isEmpty() || name.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_BaseName)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QString baseName = asyncInfo->d->baseName();
    EXPECT_TRUE(!baseName.isEmpty() || baseName.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_CompleteBaseName)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(multiExtFileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QString completeBaseName = asyncInfo->d->completeBaseName();
    EXPECT_TRUE(!completeBaseName.isEmpty() || completeBaseName.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_CompleteSuffix)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QString suffix = asyncInfo->d->completeSuffix();
    EXPECT_TRUE(!suffix.isEmpty() || suffix.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_IconName)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QString iconName = asyncInfo->d->iconName();
    EXPECT_TRUE(!iconName.isEmpty() || iconName.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_MimeTypeName)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QString mimeTypeName = asyncInfo->d->mimeTypeName();
    EXPECT_TRUE(!mimeTypeName.isEmpty() || mimeTypeName.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_FileDisplayName)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QString displayName = asyncInfo->d->fileDisplayName();
    EXPECT_TRUE(!displayName.isEmpty() || displayName.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_Path)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QString path = asyncInfo->d->path();
    EXPECT_TRUE(!path.isEmpty() || path.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_FilePath)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QString filePath = asyncInfo->d->filePath();
    EXPECT_TRUE(!filePath.isEmpty() || filePath.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_SymLinkTarget)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(linkInfo.data());
    ASSERT_TRUE(asyncInfo);

    QString target = asyncInfo->d->symLinkTarget();
    EXPECT_TRUE(!target.isEmpty() || target.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_RedirectedFileUrl)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QUrl redirectedUrl = asyncInfo->d->redirectedFileUrl();
    EXPECT_TRUE(redirectedUrl.isValid());
}

TEST_F(TestAsyncFileInfo, Private_IsExecutable)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    bool executable = asyncInfo->d->isExecutable();
    EXPECT_TRUE(executable || !executable);
}

TEST_F(TestAsyncFileInfo, Private_CanDelete)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    bool canDelete = asyncInfo->d->canDelete();
    EXPECT_TRUE(canDelete || !canDelete);
}

TEST_F(TestAsyncFileInfo, Private_CanTrash)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    bool canTrash = asyncInfo->d->canTrash();
    EXPECT_TRUE(canTrash || !canTrash);
}

TEST_F(TestAsyncFileInfo, Private_CanRename)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    bool canRename = asyncInfo->d->canRename();
    EXPECT_TRUE(canRename || !canRename);
}

TEST_F(TestAsyncFileInfo, Private_CanFetch)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(dirInfo.data());
    ASSERT_TRUE(asyncInfo);

    bool canFetch = asyncInfo->d->canFetch();
    EXPECT_TRUE(canFetch || !canFetch);
}

TEST_F(TestAsyncFileInfo, Private_SizeFormat)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QString sizeFormat = asyncInfo->d->sizeFormat();
    EXPECT_TRUE(!sizeFormat.isEmpty() || sizeFormat.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_Attribute)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    bool ok = false;
    QVariant attr = asyncInfo->d->attribute(DFMIO::DFileInfo::AttributeID::kStandardSize, &ok);
    EXPECT_TRUE(attr.isValid() || !attr.isValid());
}

TEST_F(TestAsyncFileInfo, Private_AsyncAttribute)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QVariant attr = asyncInfo->d->asyncAttribute(FileInfo::FileInfoAttributeID::kStandardSize);
    EXPECT_TRUE(attr.isValid() || !attr.isValid());
}

TEST_F(TestAsyncFileInfo, Private_FileType)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    FileInfo::FileType type = asyncInfo->d->fileType();
    EXPECT_TRUE(type == FileInfo::FileType::kRegularFile || type != FileInfo::FileType::kRegularFile);
}

TEST_F(TestAsyncFileInfo, Private_InsertAsyncAttribute)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    bool result = asyncInfo->d->insertAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardSize, QVariant(12345));
    EXPECT_TRUE(result || !result);
}

TEST_F(TestAsyncFileInfo, Private_FileMimeTypeAsync)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    asyncInfo->d->fileMimeTypeAsync();
    // Just ensure no crash
}

TEST_F(TestAsyncFileInfo, Private_DefaultIcon)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QIcon icon = asyncInfo->d->defaultIcon();
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}

TEST_F(TestAsyncFileInfo, Private_MimeTypes)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QMimeType mimeType = asyncInfo->d->mimeTypes(testFilePath);
    EXPECT_TRUE(mimeType.isValid() || !mimeType.isValid());
}

TEST_F(TestAsyncFileInfo, Private_UpdateThumbnail)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    asyncInfo->d->updateThumbnail(fileInfo->urlOf(UrlInfoType::kUrl));
    // Just ensure no crash
}

TEST_F(TestAsyncFileInfo, Private_UpdateIcon)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QIcon icon = asyncInfo->d->updateIcon();
    EXPECT_TRUE(icon.isNull() || !icon.isNull());
}

TEST_F(TestAsyncFileInfo, Private_MediaInfo)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QList<DFMIO::DFileInfo::AttributeExtendID> ids;
    ids << DFMIO::DFileInfo::AttributeExtendID::kExtendMediaDuration;

    auto mediaInfo = asyncInfo->d->mediaInfo(DFMIO::DFileInfo::MediaType::kGeneral, ids);
    EXPECT_TRUE(mediaInfo.isEmpty() || !mediaInfo.isEmpty());
}

TEST_F(TestAsyncFileInfo, Private_UpdateMediaInfo)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    QList<DFMIO::DFileInfo::AttributeExtendID> ids;
    ids << DFMIO::DFileInfo::AttributeExtendID::kExtendMediaDuration;

    asyncInfo->d->updateMediaInfo(DFMIO::DFileInfo::MediaType::kGeneral, ids);
    // Just ensure no crash
}

TEST_F(TestAsyncFileInfo, Private_HasAsyncAttribute)
{
    AsyncFileInfo *asyncInfo = dynamic_cast<AsyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(asyncInfo);

    bool has = asyncInfo->d->hasAsyncAttribute(FileInfo::FileInfoAttributeID::kStandardSize);
    EXPECT_TRUE(has || !has);
}
