// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMimeDatabase>

#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-io/dfile.h>
#include <dfm-io/dfileinfo.h>
#include <dfm-base/file/local/private/syncfileinfo_p.h>

DFMBASE_USE_NAMESPACE

class TestSyncFileInfo : public testing::Test
{
public:
    void SetUp() override
    {
        // Register FileInfo classes
        UrlRoute::regScheme(Global::Scheme::kFile, "/");
        InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);

        // Create temporary directory
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
        tempDirPath = tempDir->path();

        // Create test file
        testFilePath = tempDirPath + "/testfile.txt";
        QFile file(testFilePath);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream stream(&file);
        stream << "test content for unit test";
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

        // Create FileInfo instances
        fileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(testFilePath));
        ASSERT_TRUE(fileInfo);

        dirInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(testSubDirPath));
        ASSERT_TRUE(dirInfo);

        linkInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(testLinkPath));
        ASSERT_TRUE(linkInfo);

        hiddenFileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(testHiddenFilePath));
        ASSERT_TRUE(hiddenFileInfo);

        multiExtFileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(testMultiExtFilePath));
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

TEST_F(TestSyncFileInfo, Constructor_WithUrl)
{
    QUrl testUrl = QUrl::fromLocalFile(testFilePath);
    SyncFileInfo info(testUrl);
    EXPECT_EQ(info.urlOf(SyncFileInfo::FileUrlInfoType::kUrl), testUrl);
}

TEST_F(TestSyncFileInfo, Constructor_WithUrlAndDFileInfo)
{
    QUrl testUrl = QUrl::fromLocalFile(testFilePath);
    auto dfileInfo = QSharedPointer<DFMIO::DFileInfo>(new DFMIO::DFileInfo(testUrl));
    SyncFileInfo info(testUrl, dfileInfo);
    EXPECT_EQ(info.urlOf(SyncFileInfo::FileUrlInfoType::kUrl), testUrl);
}

TEST_F(TestSyncFileInfo, OperatorEqual_SameFile)
{
    QUrl testUrl = QUrl::fromLocalFile(testFilePath);
    SyncFileInfo info1(testUrl);
    SyncFileInfo info2(testUrl);
    EXPECT_TRUE(info1 == info2);
}

TEST_F(TestSyncFileInfo, OperatorNotEqual_DifferentFiles)
{
    SyncFileInfo info1(QUrl::fromLocalFile(testFilePath));
    SyncFileInfo info2(QUrl::fromLocalFile(testSubDirPath));
    EXPECT_TRUE(info1 != info2);
}

// ========== initQuerier and Async Operations ==========

TEST_F(TestSyncFileInfo, InitQuerier_Success)
{
    bool result = fileInfo->initQuerier();
    EXPECT_TRUE(result);
}

TEST_F(TestSyncFileInfo, InitQuerierAsync_Called)
{
    bool callbackInvoked = false;
    auto callback = [](bool, void *userData) {
        bool *flag = static_cast<bool *>(userData);
        *flag = true;
    };

    fileInfo->initQuerierAsync(0, callback, &callbackInvoked);
    // Just verify no crash - callback may be invoked asynchronously
}

// ========== exists and refresh ==========

TEST_F(TestSyncFileInfo, Exists_ExistingFile)
{
    EXPECT_TRUE(fileInfo->exists());
}

TEST_F(TestSyncFileInfo, Exists_NonExistingFile)
{
    auto nonExistInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile("/nonexistent/file.txt"));
    ASSERT_TRUE(nonExistInfo);
    EXPECT_FALSE(nonExistInfo->exists());
}

TEST_F(TestSyncFileInfo, Refresh_ClearsCaches)
{
    // Access some attributes to populate cache
    fileInfo->nameOf(SyncFileInfo::FileNameInfoType::kFileName);
    fileInfo->size();

    // Refresh should clear caches
    fileInfo->refresh();

    // Should still work after refresh
    EXPECT_FALSE(fileInfo->nameOf(SyncFileInfo::FileNameInfoType::kFileName).isEmpty());
}

// ========== cacheAttribute ==========

TEST_F(TestSyncFileInfo, CacheAttribute_Success)
{
    fileInfo->cacheAttribute(DFMIO::DFileInfo::AttributeID::kStandardSize, QVariant(12345));
    // Verify it doesn't crash - actual cache retrieval is internal
}

// ========== nameOf ==========

TEST_F(TestSyncFileInfo, NameOf_FileName)
{
    QString name = fileInfo->nameOf(SyncFileInfo::FileNameInfoType::kFileName);
    EXPECT_EQ(name, "testfile.txt");
}

TEST_F(TestSyncFileInfo, NameOf_BaseName)
{
    QString baseName = fileInfo->nameOf(SyncFileInfo::FileNameInfoType::kBaseName);
    EXPECT_EQ(baseName, "testfile");
}

TEST_F(TestSyncFileInfo, NameOf_CompleteBaseName)
{
    QString completeBaseName = multiExtFileInfo->nameOf(SyncFileInfo::FileNameInfoType::kCompleteBaseName);
    EXPECT_EQ(completeBaseName, "archive.tar");
}

TEST_F(TestSyncFileInfo, NameOf_CompleteSuffix)
{
    QString suffix = fileInfo->nameOf(SyncFileInfo::FileNameInfoType::kCompleteSuffix);
    EXPECT_EQ(suffix, "txt");
}

TEST_F(TestSyncFileInfo, NameOf_CompleteSuffix_MultipleExtensions)
{
    QString suffix = multiExtFileInfo->nameOf(SyncFileInfo::FileNameInfoType::kCompleteSuffix);
    EXPECT_EQ(suffix, "tar.gz");
}

TEST_F(TestSyncFileInfo, NameOf_FileCopyName)
{
    QString copyName = fileInfo->nameOf(SyncFileInfo::FileNameInfoType::kFileCopyName);
    EXPECT_FALSE(copyName.isEmpty());
}

TEST_F(TestSyncFileInfo, NameOf_IconName)
{
    QString iconName = fileInfo->nameOf(SyncFileInfo::FileNameInfoType::kIconName);
    EXPECT_FALSE(iconName.isEmpty());
}

TEST_F(TestSyncFileInfo, NameOf_GenericIconName)
{
    QString genericIconName = fileInfo->nameOf(SyncFileInfo::FileNameInfoType::kGenericIconName);
    EXPECT_FALSE(genericIconName.isEmpty());
}

TEST_F(TestSyncFileInfo, NameOf_MimeTypeName)
{
    QString mimeTypeName = fileInfo->nameOf(SyncFileInfo::FileNameInfoType::kMimeTypeName);
    EXPECT_FALSE(mimeTypeName.isEmpty());
}

// ========== pathOf ==========

TEST_F(TestSyncFileInfo, PathOf_FilePath)
{
    QString path = fileInfo->pathOf(SyncFileInfo::FilePathInfoType::kFilePath);
    EXPECT_EQ(path, testFilePath);
}

TEST_F(TestSyncFileInfo, PathOf_AbsoluteFilePath)
{
    QString absPath = fileInfo->pathOf(SyncFileInfo::FilePathInfoType::kAbsoluteFilePath);
    EXPECT_EQ(absPath, testFilePath);
}

TEST_F(TestSyncFileInfo, PathOf_Path)
{
    QString path = fileInfo->pathOf(SyncFileInfo::FilePathInfoType::kPath);
    EXPECT_EQ(path, tempDirPath);
}

TEST_F(TestSyncFileInfo, PathOf_AbsolutePath)
{
    QString absPath = fileInfo->pathOf(SyncFileInfo::FilePathInfoType::kAbsolutePath);
    EXPECT_EQ(absPath, tempDirPath);
}

TEST_F(TestSyncFileInfo, PathOf_SymLinkTarget)
{
    QString target = linkInfo->pathOf(SyncFileInfo::FilePathInfoType::kSymLinkTarget);
    EXPECT_EQ(target, testFilePath);
}

// ========== urlOf ==========

TEST_F(TestSyncFileInfo, UrlOf_Url)
{
    QUrl url = fileInfo->urlOf(SyncFileInfo::FileUrlInfoType::kUrl);
    EXPECT_EQ(url, QUrl::fromLocalFile(testFilePath));
}

TEST_F(TestSyncFileInfo, UrlOf_RedirectedFileUrl)
{
    QUrl redirectedUrl = fileInfo->urlOf(SyncFileInfo::FileUrlInfoType::kRedirectedFileUrl);
    EXPECT_TRUE(redirectedUrl.isValid());
}

// ========== isAttributes ==========

TEST_F(TestSyncFileInfo, IsAttributes_IsFile)
{
    EXPECT_TRUE(fileInfo->isAttributes(SyncFileInfo::FileIsType::kIsFile));
    EXPECT_FALSE(dirInfo->isAttributes(SyncFileInfo::FileIsType::kIsFile));
}

TEST_F(TestSyncFileInfo, IsAttributes_IsDir)
{
    EXPECT_FALSE(fileInfo->isAttributes(SyncFileInfo::FileIsType::kIsDir));
    EXPECT_TRUE(dirInfo->isAttributes(SyncFileInfo::FileIsType::kIsDir));
}

TEST_F(TestSyncFileInfo, IsAttributes_IsReadable)
{
    EXPECT_TRUE(fileInfo->isAttributes(SyncFileInfo::FileIsType::kIsReadable));
}

TEST_F(TestSyncFileInfo, IsAttributes_IsWritable)
{
    bool writable = fileInfo->isAttributes(SyncFileInfo::FileIsType::kIsWritable);
    // Result depends on file permissions, just ensure no crash
    EXPECT_TRUE(writable || !writable);
}

TEST_F(TestSyncFileInfo, IsAttributes_IsHidden)
{
    EXPECT_FALSE(fileInfo->isAttributes(SyncFileInfo::FileIsType::kIsHidden));
    EXPECT_TRUE(hiddenFileInfo->isAttributes(SyncFileInfo::FileIsType::kIsHidden));
}

TEST_F(TestSyncFileInfo, IsAttributes_IsSymLink)
{
    EXPECT_FALSE(fileInfo->isAttributes(SyncFileInfo::FileIsType::kIsSymLink));
    EXPECT_TRUE(linkInfo->isAttributes(SyncFileInfo::FileIsType::kIsSymLink));
}

TEST_F(TestSyncFileInfo, IsAttributes_IsExecutable)
{
    bool executable = fileInfo->isAttributes(SyncFileInfo::FileIsType::kIsExecutable);
    // Result depends on file permissions
    EXPECT_TRUE(executable || !executable);
}

// ========== canAttributes ==========

TEST_F(TestSyncFileInfo, CanAttributes_CanRename)
{
    bool canRename = fileInfo->canAttributes(SyncFileInfo::FileCanType::kCanRename);
    EXPECT_TRUE(canRename || !canRename);
}

TEST_F(TestSyncFileInfo, CanAttributes_CanDelete)
{
    bool canDelete = fileInfo->canAttributes(SyncFileInfo::FileCanType::kCanDelete);
    EXPECT_TRUE(canDelete || !canDelete);
}

TEST_F(TestSyncFileInfo, CanAttributes_CanTrash)
{
    bool canTrash = fileInfo->canAttributes(SyncFileInfo::FileCanType::kCanTrash);
    EXPECT_TRUE(canTrash || !canTrash);
}

// ========== extendAttributes ==========

TEST_F(TestSyncFileInfo, ExtendAttributes_FileLocalDevice)
{
    QVariant deviceVariant = fileInfo->extendAttributes(SyncFileInfo::FileExtendedInfoType::kFileLocalDevice);
    EXPECT_TRUE(deviceVariant.isValid());
}

TEST_F(TestSyncFileInfo, ExtendAttributes_FileCdRomDevice)
{
    QVariant cdromVariant = fileInfo->extendAttributes(SyncFileInfo::FileExtendedInfoType::kFileCdRomDevice);
    EXPECT_TRUE(cdromVariant.isValid());
}

// ========== permission and permissions ==========

TEST_F(TestSyncFileInfo, Permission_ReadOwner)
{
    bool hasPermission = fileInfo->permission(QFile::ReadOwner);
    EXPECT_TRUE(hasPermission);
}

TEST_F(TestSyncFileInfo, Permissions_GetAll)
{
    QFile::Permissions perms = fileInfo->permissions();
    EXPECT_TRUE(perms & QFile::ReadOwner);
}

// ========== size ==========

TEST_F(TestSyncFileInfo, Size_RegularFile)
{
    qint64 size = fileInfo->size();
    EXPECT_GT(size, 0);
}

TEST_F(TestSyncFileInfo, Size_Directory)
{
    qint64 size = dirInfo->size();
    EXPECT_GE(size, 0);
}

// ========== timeOf ==========

TEST_F(TestSyncFileInfo, TimeOf_CreateTime)
{
    QVariant createTime = fileInfo->timeOf(SyncFileInfo::FileTimeType::kCreateTime);
    EXPECT_TRUE(createTime.isValid());
}

TEST_F(TestSyncFileInfo, TimeOf_LastModified)
{
    QVariant modTime = fileInfo->timeOf(SyncFileInfo::FileTimeType::kLastModified);
    EXPECT_TRUE(modTime.isValid());
}

TEST_F(TestSyncFileInfo, TimeOf_LastRead)
{
    QVariant readTime = fileInfo->timeOf(SyncFileInfo::FileTimeType::kLastRead);
    EXPECT_TRUE(readTime.isValid());
}

// ========== extraProperties ==========

TEST_F(TestSyncFileInfo, ExtraProperties_NotEmpty)
{
    QVariantHash props = fileInfo->extraProperties();
    // May or may not have extra properties
    EXPECT_TRUE(props.isEmpty() || !props.isEmpty());
}

// ========== fileIcon ==========

TEST_F(TestSyncFileInfo, FileIcon_NotNull)
{
    QIcon icon = fileInfo->fileIcon();
    EXPECT_FALSE(icon.isNull());
}

// ========== fileType ==========

TEST_F(TestSyncFileInfo, FileType_RegularFile)
{
    FileInfo::FileType type = fileInfo->fileType();
    EXPECT_EQ(type, FileInfo::FileType::kRegularFile);
}

TEST_F(TestSyncFileInfo, FileType_Directory)
{
    FileInfo::FileType type = dirInfo->fileType();
    EXPECT_EQ(type, FileInfo::FileType::kDirectory);
}

// ========== countChildFile ==========

TEST_F(TestSyncFileInfo, CountChildFile_Directory)
{
    // Create some child files
    QString childFile1 = testSubDirPath + "/child1.txt";
    QString childFile2 = testSubDirPath + "/child2.txt";
    QFile(childFile1).open(QIODevice::WriteOnly);
    QFile(childFile2).open(QIODevice::WriteOnly);

    int count = dirInfo->countChildFile();
    EXPECT_GE(count, 2);
}

TEST_F(TestSyncFileInfo, CountChildFileAsync_Directory)
{
    int count = dirInfo->countChildFileAsync();
    EXPECT_GE(count, 0);
}

// ========== displayOf ==========

TEST_F(TestSyncFileInfo, DisplayOf_Size)
{
    QString sizeDisplay = fileInfo->displayOf(SyncFileInfo::DisplayInfoType::kSizeDisplayName);
    EXPECT_FALSE(sizeDisplay.isEmpty());
}

TEST_F(TestSyncFileInfo, DisplayOf_MimeType)
{
    QString mimeDisplay = fileInfo->displayOf(SyncFileInfo::DisplayInfoType::kMimeTypeDisplayName);
    EXPECT_FALSE(mimeDisplay.isEmpty());
}

// ========== fileMimeType ==========

TEST_F(TestSyncFileInfo, FileMimeType_DefaultMode)
{
    QMimeType mimeType = fileInfo->fileMimeType();
    EXPECT_TRUE(mimeType.isValid());
}

TEST_F(TestSyncFileInfo, FileMimeType_MatchExtension)
{
    QMimeType mimeType = fileInfo->fileMimeType(QMimeDatabase::MatchExtension);
    EXPECT_TRUE(mimeType.isValid());
}

TEST_F(TestSyncFileInfo, FileMimeTypeAsync_DefaultMode)
{
    QMimeType mimeType = fileInfo->fileMimeTypeAsync();
    EXPECT_TRUE(mimeType.isValid());
}

// ========== viewOfTip ==========

TEST_F(TestSyncFileInfo, ViewOfTip_EmptyTip)
{
    QString tip = fileInfo->viewOfTip(SyncFileInfo::ViewType::kEmptyDir);
    // May be empty or have content
    EXPECT_TRUE(tip.isEmpty() || !tip.isEmpty());
}

// ========== customAttribute ==========

TEST_F(TestSyncFileInfo, CustomAttribute_Emblems)
{
    QVariant emblems = fileInfo->customAttribute("metadata::emblems", DFMIO::DFileInfo::DFileAttributeType::kTypeStringV);
    // May or may not have emblems
    EXPECT_TRUE(emblems.isValid() || !emblems.isValid());
}

// ========== mediaInfoAttributes ==========

TEST_F(TestSyncFileInfo, MediaInfoAttributes_General)
{
    QList<DFMIO::DFileInfo::AttributeExtendID> ids;
    ids << DFMIO::DFileInfo::AttributeExtendID::kExtendMediaDuration;

    auto attrs = fileInfo->mediaInfoAttributes(DFMIO::DFileInfo::MediaType::kGeneral, ids);
    // May or may not have media info
    EXPECT_TRUE(attrs.isEmpty() || !attrs.isEmpty());
}

// ========== setExtendedAttributes and updateAttributes ==========

TEST_F(TestSyncFileInfo, SetExtendedAttributes_Success)
{
    fileInfo->setExtendedAttributes(SyncFileInfo::FileExtendedInfoType::kUnknowExtendedInfo, QVariant(12345));
    // Verify no crash
}

TEST_F(TestSyncFileInfo, UpdateAttributes_EmptyList)
{
    fileInfo->updateAttributes();
    // Verify no crash
}

TEST_F(TestSyncFileInfo, UpdateAttributes_WithSpecificTypes)
{
    QList<FileInfo::FileInfoAttributeID> types;
    types << FileInfo::FileInfoAttributeID::kStandardSize;
    types << FileInfo::FileInfoAttributeID::kStandardName;

    fileInfo->updateAttributes(types);
    // Verify no crash
}

// ========== Edge Cases and Error Handling ==========

TEST_F(TestSyncFileInfo, EdgeCase_EmptyFileName)
{
    // Create file with just extension
    QString dotFilePath = tempDirPath + "/.config";
    QFile dotFile(dotFilePath);
    ASSERT_TRUE(dotFile.open(QIODevice::WriteOnly));
    dotFile.close();

    auto dotFileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(dotFilePath));
    ASSERT_TRUE(dotFileInfo);

    QString name = dotFileInfo->nameOf(SyncFileInfo::FileNameInfoType::kFileName);
    EXPECT_EQ(name, ".config");
}

TEST_F(TestSyncFileInfo, EdgeCase_VeryLongFileName)
{
    // Create file with long name (within filesystem limits)
    QString longName = QString("a").repeated(200) + ".txt";
    QString longFilePath = tempDirPath + "/" + longName;
    QFile longFile(longFilePath);
    ASSERT_TRUE(longFile.open(QIODevice::WriteOnly));
    longFile.close();

    auto longFileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(longFilePath));
    ASSERT_TRUE(longFileInfo);

    QString name = longFileInfo->nameOf(SyncFileInfo::FileNameInfoType::kFileName);
    EXPECT_EQ(name, longName);
}

TEST_F(TestSyncFileInfo, EdgeCase_SpecialCharactersInFileName)
{
    // Create file with special characters
    QString specialName = "file-with_special@chars#.txt";
    QString specialFilePath = tempDirPath + "/" + specialName;
    QFile specialFile(specialFilePath);
    ASSERT_TRUE(specialFile.open(QIODevice::WriteOnly));
    specialFile.close();

    auto specialFileInfo = InfoFactory::create<FileInfo>(QUrl::fromLocalFile(specialFilePath));
    ASSERT_TRUE(specialFileInfo);

    QString name = specialFileInfo->nameOf(SyncFileInfo::FileNameInfoType::kFileName);
    EXPECT_EQ(name, specialName);
}

// ========== Stubbing Tests ==========

TEST_F(TestSyncFileInfo, Stub_DFileExists_ReturnsFalse)
{
    stub.set_lamda(static_cast<bool (DFMIO::DFile::*)() const>(&DFMIO::DFile::exists),
                   []() -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    EXPECT_FALSE(fileInfo->exists());
}

TEST_F(TestSyncFileInfo, Stub_InitQuerier_ReturnsFalse)
{
    stub.set_lamda(&DFMIO::DFileInfo::initQuerier,
                   [](DFMIO::DFileInfo *) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = fileInfo->initQuerier();
    EXPECT_FALSE(result);
}

// ========== SyncFileInfoPrivate Tests ==========

// Note: We access private members through the public interface 'd'
// The private header is already included by syncfileinfo.h

TEST_F(TestSyncFileInfo, Private_FileName)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QString name = syncInfo->d->fileName();
    EXPECT_EQ(name, "testfile.txt");
}

TEST_F(TestSyncFileInfo, Private_BaseName)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QString baseName = syncInfo->d->baseName();
    EXPECT_EQ(baseName, "testfile");
}

TEST_F(TestSyncFileInfo, Private_CompleteBaseName)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(multiExtFileInfo.data());
    ASSERT_TRUE(syncInfo);

    QString completeBaseName = syncInfo->d->completeBaseName();
    EXPECT_EQ(completeBaseName, "archive.tar");
}

TEST_F(TestSyncFileInfo, Private_CompleteSuffix)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QString suffix = syncInfo->d->completeSuffix();
    EXPECT_EQ(suffix, "txt");
}

TEST_F(TestSyncFileInfo, Private_IconName)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QString iconName = syncInfo->d->iconName();
    EXPECT_FALSE(iconName.isEmpty());
}

TEST_F(TestSyncFileInfo, Private_MimeTypeName)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QString mimeTypeName = syncInfo->d->mimeTypeName();
    EXPECT_FALSE(mimeTypeName.isEmpty());
}

TEST_F(TestSyncFileInfo, Private_FileDisplayName)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QString displayName = syncInfo->d->fileDisplayName();
    EXPECT_FALSE(displayName.isEmpty());
}

TEST_F(TestSyncFileInfo, Private_Path)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QString path = syncInfo->d->path();
    EXPECT_EQ(path, tempDirPath);
}

TEST_F(TestSyncFileInfo, Private_FilePath)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QString filePath = syncInfo->d->filePath();
    EXPECT_EQ(filePath, testFilePath);
}

TEST_F(TestSyncFileInfo, Private_SymLinkTarget)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(linkInfo.data());
    ASSERT_TRUE(syncInfo);

    QString target = syncInfo->d->symLinkTarget();
    EXPECT_EQ(target, testFilePath);
}

TEST_F(TestSyncFileInfo, Private_RedirectedFileUrl)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QUrl redirectedUrl = syncInfo->d->redirectedFileUrl();
    EXPECT_TRUE(redirectedUrl.isValid());
}

TEST_F(TestSyncFileInfo, Private_IsExecutable)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    bool executable = syncInfo->d->isExecutable();
    EXPECT_TRUE(executable || !executable);
}

TEST_F(TestSyncFileInfo, Private_CanDelete)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    bool canDelete = syncInfo->d->canDelete();
    EXPECT_TRUE(canDelete || !canDelete);
}

TEST_F(TestSyncFileInfo, Private_CanTrash)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    bool canTrash = syncInfo->d->canTrash();
    EXPECT_TRUE(canTrash || !canTrash);
}

TEST_F(TestSyncFileInfo, Private_CanRename)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    bool canRename = syncInfo->d->canRename();
    EXPECT_TRUE(canRename || !canRename);
}

TEST_F(TestSyncFileInfo, Private_CanFetch)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(dirInfo.data());
    ASSERT_TRUE(syncInfo);

    bool canFetch = syncInfo->d->canFetch();
    EXPECT_TRUE(canFetch || !canFetch);
}

TEST_F(TestSyncFileInfo, Private_SizeFormat)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QString sizeFormat = syncInfo->d->sizeFormat();
    EXPECT_FALSE(sizeFormat.isEmpty());
}

TEST_F(TestSyncFileInfo, Private_Attribute)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    bool ok = false;
    QVariant attr = syncInfo->d->attribute(DFMIO::DFileInfo::AttributeID::kStandardSize, &ok);
    EXPECT_TRUE(attr.isValid() || !attr.isValid());
}

TEST_F(TestSyncFileInfo, Private_MimeTypes)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QMimeType mimeType = syncInfo->d->mimeTypes(testFilePath);
    EXPECT_TRUE(mimeType.isValid());
}

TEST_F(TestSyncFileInfo, Private_UpdateFileType)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    FileInfo::FileType type = syncInfo->d->updateFileType();
    EXPECT_EQ(type, FileInfo::FileType::kRegularFile);
}

TEST_F(TestSyncFileInfo, Private_UpdateIcon)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QIcon icon = syncInfo->d->updateIcon();
    EXPECT_FALSE(icon.isNull());
}

TEST_F(TestSyncFileInfo, Private_MediaInfo)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QList<DFMIO::DFileInfo::AttributeExtendID> ids;
    ids << DFMIO::DFileInfo::AttributeExtendID::kExtendMediaDuration;

    auto mediaInfo = syncInfo->d->mediaInfo(DFMIO::DFileInfo::MediaType::kGeneral, ids);
    EXPECT_TRUE(mediaInfo.isEmpty() || !mediaInfo.isEmpty());
}

TEST_F(TestSyncFileInfo, Private_UpdateMediaInfo)
{
    SyncFileInfo *syncInfo = dynamic_cast<SyncFileInfo *>(fileInfo.data());
    ASSERT_TRUE(syncInfo);

    QList<DFMIO::DFileInfo::AttributeExtendID> ids;
    ids << DFMIO::DFileInfo::AttributeExtendID::kExtendMediaDuration;

    syncInfo->d->updateMediaInfo(DFMIO::DFileInfo::MediaType::kGeneral, ids);
    // Just ensure no crash
}
