// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dfm-base/interfaces/abstractfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/universalutils.h>

#include <QUrl>
#include <QDir>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>

#include <memory>

DFMBASE_USE_NAMESPACE

class AbstractFileInfoTest : public testing::Test
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
        out << "Test content for abstract file info";
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

TEST_F(AbstractFileInfoTest, Constructor)
{
    // Test with file URL
    auto fileInfo = std::make_shared<AsyncFileInfo>(fileUrl);
    ASSERT_NE(fileInfo, nullptr);
    EXPECT_EQ(fileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl), fileUrl);

    // Test with directory URL
    auto dirInfo = std::make_shared<AsyncFileInfo>(dirUrl);
    ASSERT_NE(dirInfo, nullptr);
    EXPECT_EQ(dirInfo->urlOf(FileInfo::FileUrlInfoType::kUrl), dirUrl);
}

TEST_F(AbstractFileInfoTest, UrlPathNormalization)
{
    // Test URL path normalization (removing trailing separator)
    QUrl urlWithSeparator = QUrl::fromLocalFile(testDirPath + QDir::separator());
    auto fileInfo = std::make_shared<AsyncFileInfo>(urlWithSeparator);
    
    EXPECT_EQ(fileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl).path(), QDir::cleanPath(testDirPath));
}

TEST_F(AbstractFileInfoTest, FileExists)
{
    auto existingFileInfo = std::make_shared<AsyncFileInfo>(fileUrl);
    EXPECT_TRUE(existingFileInfo->exists());

    auto nonExistentFileInfo = std::make_shared<AsyncFileInfo>(nonExistentUrl);
    EXPECT_FALSE(nonExistentFileInfo->exists());
}

TEST_F(AbstractFileInfoTest, FilePermissions)
{
    auto fileInfo = std::make_shared<AsyncFileInfo>(fileUrl);
    
    // Test basic permission methods
    EXPECT_TRUE(fileInfo->isReadable());
    EXPECT_TRUE(fileInfo->isWritable());
    
    // Test isFile and isDir
    EXPECT_TRUE(fileInfo->isFile());
    EXPECT_FALSE(fileInfo->isDir());
    
    auto dirInfo = std::make_shared<AsyncFileInfo>(dirUrl);
    EXPECT_FALSE(dirInfo->isFile());
    EXPECT_TRUE(dirInfo->isDir());
}

TEST_F(AbstractFileInfoTest, FileAttributes)
{
    auto fileInfo = std::make_shared<AsyncFileInfo>(fileUrl);
    
    // Test basic attributes
    EXPECT_GE(fileInfo->size(), 0);
    EXPECT_NE(fileInfo->timeOf(FileInfo::FileTimeType::kLastModified).toUInt(), 0);
    EXPECT_NE(fileInfo->timeOf(FileInfo::FileTimeType::kBirthTime).toUInt(), 0);
    EXPECT_NE(fileInfo->timeOf(FileInfo::FileTimeType::kLastRead).toUInt(), 0);
}

TEST_F(AbstractFileInfoTest, FileNameOperations)
{
    auto fileInfo = std::make_shared<AsyncFileInfo>(fileUrl);
    
    // Test file name operations
    EXPECT_EQ(fileInfo->fileName(), "test_file.txt");
    EXPECT_EQ(fileInfo->baseName(), "test_file");
    EXPECT_EQ(fileInfo->completeSuffix(), "txt");
    EXPECT_EQ(fileInfo->suffix(), "txt");
    
    // Test absolute path
    EXPECT_EQ(fileInfo->pathOf(FileInfo::FilePathInfoType::kAbsoluteFilePath), QFileInfo(testFilePath).absoluteFilePath());
    EXPECT_EQ(fileInfo->pathOf(FileInfo::FilePathInfoType::kAbsolutePath), QFileInfo(testFilePath).absolutePath());
}

TEST_F(AbstractFileInfoTest, DirectoryOperations)
{
    auto dirInfo = std::make_shared<AsyncFileInfo>(dirUrl);
    
    EXPECT_TRUE(dirInfo->isDir());
    EXPECT_FALSE(dirInfo->isFile());
    EXPECT_EQ(dirInfo->fileName(), "test_dir");
}

TEST_F(AbstractFileInfoTest, UrlTypeChecking)
{
    auto fileInfo = std::make_shared<AsyncFileInfo>(fileUrl);
    
    // Test URL type checking
    EXPECT_TRUE(fileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl).isLocalFile());
    EXPECT_FALSE(fileInfo->urlOf(FileInfo::FileUrlInfoType::kUrl).isEmpty());
}

// TEST_F(AbstractFileInfoTest, BoundaryConditions)
// {
//     // Test with empty URL
//     auto emptyFileInfo = std::make_shared<AsyncFileInfo>(QUrl());
//     EXPECT_FALSE(emptyFileInfo->exists());
    
//     // // Test with root path
//     // QUrl rootUrl = QUrl::fromLocalFile(QDir::rootPath());
//     // auto rootFileInfo = std::make_shared<AsyncFileInfo>(rootUrl);
//     // EXPECT_TRUE(rootFileInfo->exists());
//     // EXPECT_TRUE(rootFileInfo->isDir());
// }

TEST_F(AbstractFileInfoTest, SymLinkHandling)
{
    QString linkPath = tempDir->filePath("test_link");
    QFile::link(testFilePath, linkPath);
    
    QUrl linkUrl = QUrl::fromLocalFile(linkPath);
    auto linkFileInfo = std::make_shared<AsyncFileInfo>(linkUrl);
    
    EXPECT_TRUE(linkFileInfo->exists());
    EXPECT_TRUE(linkFileInfo->isSymLink());
}

TEST_F(AbstractFileInfoTest, HiddenFiles)
{
    QString hiddenFilePath = tempDir->filePath(".hidden_file");
    QFile hiddenFile(hiddenFilePath);
    ASSERT_TRUE(hiddenFile.open(QIODevice::WriteOnly));
    hiddenFile.close();
    
    QUrl hiddenUrl = QUrl::fromLocalFile(hiddenFilePath);
    auto hiddenFileInfo = std::make_shared<AsyncFileInfo>(hiddenUrl);
    
    EXPECT_TRUE(hiddenFileInfo->exists());
    EXPECT_TRUE(hiddenFileInfo->isHidden());
}

TEST_F(AbstractFileInfoTest, MetaTypeRegistration)
{
    // Test that AbstractFileInfoPointer is properly registered as a meta type
    int typeId = qMetaTypeId<AbstractFileInfoPointer>();
    EXPECT_NE(typeId, 0);
}
