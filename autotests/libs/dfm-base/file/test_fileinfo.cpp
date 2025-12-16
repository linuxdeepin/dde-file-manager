// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

#include <QUrl>
#include <QDir>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>

#include <memory>

DFMBASE_USE_NAMESPACE

class FileInfoTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create a temporary directory for testing
        tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tempDir->isValid());

        // Create test files
        testFilePath = tempDir->filePath("test_file.txt");
        testDirPath = tempDir->filePath("test_dir");
        
        QFile testFile(testFilePath);
        ASSERT_TRUE(testFile.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&testFile);
        out << "Test content for file info testing";
        testFile.close();

        ASSERT_TRUE(QDir().mkpath(testDirPath));

        // Create URLs
        fileUrl = QUrl::fromLocalFile(testFilePath);
        dirUrl = QUrl::fromLocalFile(testDirPath);
        nonExistentUrl = QUrl::fromLocalFile(tempDir->filePath("non_existent_file.txt"));
    }

    void TearDown() override
    {
        tempDir.reset();
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    QString testFilePath;
    QString testDirPath;
    QUrl fileUrl;
    QUrl dirUrl;
    QUrl nonExistentUrl;
};

TEST_F(FileInfoTest, Constructor)
{
    // Test with file URL
    auto fileInfo = std::make_shared<FileInfo>(fileUrl);
    ASSERT_NE(fileInfo, nullptr);
    EXPECT_EQ(fileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl), fileUrl);

    // Test with directory URL
    auto dirInfo = std::make_shared<FileInfo>(dirUrl);
    ASSERT_NE(dirInfo, nullptr);
    EXPECT_EQ(dirInfo->urlOf(FileInfo::FileUrlInfoType::kUrl), dirUrl);
}

TEST_F(FileInfoTest, Inheritance)
{
    auto fileInfo = std::make_shared<FileInfo>(fileUrl);
    
    // Test that FileInfo inherits from AbstractFileInfo
    EXPECT_TRUE(dynamic_cast<AbstractFileInfo*>(fileInfo.get()) != nullptr);
}

TEST_F(FileInfoTest, FileExists)
{
    auto existingFileInfo = std::make_shared<FileInfo>(fileUrl);
    EXPECT_TRUE(existingFileInfo->exists());

    auto nonExistentFileInfo = std::make_shared<FileInfo>(nonExistentUrl);
    EXPECT_FALSE(nonExistentFileInfo->exists());
}

TEST_F(FileInfoTest, FileProperties)
{
    auto fileInfo = std::make_shared<FileInfo>(fileUrl);
    
    // Test basic properties
    EXPECT_TRUE(fileInfo->isFile());
    EXPECT_FALSE(fileInfo->isDir());
    EXPECT_TRUE(fileInfo->exists());
    
    // Test file name operations
    EXPECT_EQ(fileInfo->fileName(), "test_file.txt");
    EXPECT_EQ(fileInfo->baseName(), "test_file");
    EXPECT_EQ(fileInfo->completeSuffix(), "txt");
}

TEST_F(FileInfoTest, DirectoryProperties)
{
    auto dirInfo = std::make_shared<FileInfo>(dirUrl);
    
    EXPECT_FALSE(dirInfo->isFile());
    EXPECT_TRUE(dirInfo->isDir());
    EXPECT_TRUE(dirInfo->exists());
    EXPECT_EQ(dirInfo->fileName(), "test_dir");
}

TEST_F(FileInfoTest, FileAttributes)
{
    auto fileInfo = std::make_shared<FileInfo>(fileUrl);
    
    // Test size and timestamps
    EXPECT_GE(fileInfo->size(), 0);
    EXPECT_NE(fileInfo->timeOf(FileInfo::FileTimeType::kLastModified).toUInt(), 0);
    EXPECT_NE(fileInfo->timeOf(FileInfo::FileTimeType::kBirthTime).toUInt(), 0);
    EXPECT_NE(fileInfo->timeOf(FileInfo::FileTimeType::kLastRead).toUInt(), 0);
}

TEST_F(FileInfoTest, Permissions)
{
    auto fileInfo = std::make_shared<FileInfo>(fileUrl);
    
    // Test file permissions
    EXPECT_TRUE(fileInfo->isReadable());
    EXPECT_TRUE(fileInfo->isWritable());
    EXPECT_FALSE(fileInfo->isExecutable()); // Text file should not be executable by default
}

TEST_F(FileInfoTest, MimeTypeOperations)
{
    auto fileInfo = std::make_shared<FileInfo>(fileUrl);
    
    // Test MIME type detection
    EXPECT_FALSE(fileInfo->fileMimeType().name().isEmpty());
    EXPECT_FALSE(fileInfo->displayOf(FileInfo::DisplayInfoType::kMimeTypeDisplayName).isEmpty());
}

TEST_F(FileInfoTest, UrlOperations)
{
    auto fileInfo = std::make_shared<FileInfo>(fileUrl);
    
    // Test URL operations
    EXPECT_EQ(fileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl), fileUrl);
    EXPECT_TRUE(fileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl).isLocalFile());
    EXPECT_FALSE(fileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl).isEmpty());
}

TEST_F(FileInfoTest, PathOperations)
{
    auto fileInfo = std::make_shared<FileInfo>(fileUrl);
    
    // Test path operations
    EXPECT_EQ(fileInfo->absoluteFilePath(), QFileInfo(testFilePath).absoluteFilePath());
    EXPECT_EQ(fileInfo->absolutePath(), QFileInfo(testFilePath).absolutePath());
    EXPECT_FALSE(fileInfo->path().isEmpty());
}

TEST_F(FileInfoTest, CopyConstructor)
{
    auto originalFileInfo = std::make_shared<FileInfo>(fileUrl);
    // FileInfo has deleted copy constructor due to QReadWriteLock
    // Test creating a new FileInfo with same URL instead
    auto copiedFileInfo = std::make_shared<FileInfo>(fileUrl);
    
    EXPECT_EQ(originalFileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl), copiedFileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl));
    EXPECT_EQ(originalFileInfo->fileName(), copiedFileInfo->fileName());
}

TEST_F(FileInfoTest, AssignmentOperator)
{
    auto originalFileInfo = std::make_shared<FileInfo>(fileUrl);
    auto assignedFileInfo = std::make_shared<FileInfo>(QUrl());
    
    // FileInfo has deleted copy assignment due to QReadWriteLock
    // Test creating a new FileInfo with same URL instead
    assignedFileInfo = std::make_shared<FileInfo>(fileUrl);
    
    EXPECT_EQ(originalFileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl), assignedFileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl));
    EXPECT_EQ(originalFileInfo->fileName(), assignedFileInfo->fileName());
}

TEST_F(FileInfoTest, MetaTypeRegistration)
{
    // Test that FileInfoPointer is properly registered as a meta type
    int typeId = qMetaTypeId<FileInfoPointer>();
    EXPECT_NE(typeId, 0);
}

TEST_F(FileInfoTest, BoundaryConditions)
{
    // Test with empty URL
    auto emptyFileInfo = std::make_shared<FileInfo>(QUrl());
    EXPECT_FALSE(emptyFileInfo->exists());
    
    // Test with root path
    QUrl rootUrl = QUrl::fromLocalFile(QDir::rootPath());
    auto rootFileInfo = std::make_shared<FileInfo>(rootUrl);
    EXPECT_TRUE(rootFileInfo->exists());
}

TEST_F(FileInfoTest, HiddenFiles)
{
    QString hiddenFilePath = tempDir->filePath(".hidden_file");
    QFile hiddenFile(hiddenFilePath);
    ASSERT_TRUE(hiddenFile.open(QIODevice::WriteOnly));
    hiddenFile.close();
    
    QUrl hiddenUrl = QUrl::fromLocalFile(hiddenFilePath);
    auto hiddenFileInfo = std::make_shared<FileInfo>(hiddenUrl);
    
    EXPECT_TRUE(hiddenFileInfo->exists());
    EXPECT_TRUE(hiddenFileInfo->isHidden());
}
