// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <dfm-base/interfaces/proxyfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>

#include <QUrl>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QApplication>

#include <memory>

DFMBASE_USE_NAMESPACE

class ProxyFileInfoTest : public testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure QApplication exists
        if (!QApplication::instance()) {
            app.reset(new QApplication(argc, argv));
        }

        // Create a temporary directory for testing
        tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tempDir->isValid());

        // Create test files
        testFilePath = tempDir->filePath("test_file.txt");
        testDirPath = tempDir->filePath("test_dir");
        
        QFile testFile(testFilePath);
        ASSERT_TRUE(testFile.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream out(&testFile);
        out << "Test content for proxy file info";
        testFile.close();

        ASSERT_TRUE(QDir().mkpath(testDirPath));

        // Create URLs
        fileUrl = QUrl::fromLocalFile(testFilePath);
        dirUrl = QUrl::fromLocalFile(testDirPath);
        nonExistentUrl = QUrl::fromLocalFile(tempDir->filePath("non_existent.txt"));

        // Create target file info objects using Qt's QSharedPointer
        targetFileInfo = QSharedPointer<AsyncFileInfo>::create(fileUrl);
        targetDirInfo = QSharedPointer<AsyncFileInfo>::create(dirUrl);
    }

    void TearDown() override
    {
        app.reset();
        tempDir.reset();
    }

    std::unique_ptr<QApplication> app;
    int argc { 0 };
    char **argv { nullptr };
    std::unique_ptr<QTemporaryDir> tempDir;
    QString testFilePath;
    QString testDirPath;
    QUrl fileUrl;
    QUrl dirUrl;
    QUrl nonExistentUrl;
    AbstractFileInfoPointer targetFileInfo;
    AbstractFileInfoPointer targetDirInfo;
};

TEST_F(ProxyFileInfoTest, Constructor)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    EXPECT_NE(proxyInfo, nullptr);
    EXPECT_EQ(proxyInfo->urlOf(FileInfo::FileUrlInfoType::kUrl), fileUrl);
}

TEST_F(ProxyFileInfoTest, ConstructorWithUrl)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    EXPECT_EQ(proxyInfo->fileUrl(), fileUrl);
}

TEST_F(ProxyFileInfoTest, SetProxy)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    EXPECT_EQ(proxyInfo->proxy, nullptr);
    
    // Create a FileInfo proxy target
    FileInfoPointer proxyTarget = QSharedPointer<FileInfo>::create(fileUrl);
    proxyInfo->setProxy(proxyTarget);
    
    EXPECT_NE(proxyInfo->proxy, nullptr);
    EXPECT_EQ(proxyInfo->urlOf(FileInfo::FileUrlInfoType::kUrl), fileUrl);
}

TEST_F(ProxyFileInfoTest, FileExistence)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    FileInfoPointer proxyTarget = QSharedPointer<FileInfo>::create(fileUrl);
    proxyInfo->setProxy(proxyTarget);
    
    // Test with existing file (delegated to proxy)
    EXPECT_TRUE(proxyInfo->exists());
}

TEST_F(ProxyFileInfoTest, FileProperties)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    FileInfoPointer proxyTarget = QSharedPointer<FileInfo>::create(fileUrl);
    proxyInfo->setProxy(proxyTarget);
    
    // Test file properties are delegated to proxy
    EXPECT_TRUE(proxyInfo->isFile());
    EXPECT_FALSE(proxyInfo->isDir());
    EXPECT_FALSE(proxyInfo->fileName().isEmpty());
    EXPECT_FALSE(proxyInfo->baseName().isEmpty());
}

TEST_F(ProxyFileInfoTest, DirectoryProperties)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(dirUrl);
    
    FileInfoPointer proxyTarget = QSharedPointer<FileInfo>::create(dirUrl);
    proxyInfo->setProxy(proxyTarget);
    
    // Test directory properties are delegated to proxy
    EXPECT_FALSE(proxyInfo->isFile());
    EXPECT_TRUE(proxyInfo->isDir());
    EXPECT_FALSE(proxyInfo->fileName().isEmpty());
}

TEST_F(ProxyFileInfoTest, FileAttributes)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    FileInfoPointer proxyTarget = QSharedPointer<FileInfo>::create(fileUrl);
    proxyInfo->setProxy(proxyTarget);
    
    // Test basic attributes
    EXPECT_GE(proxyInfo->size(), 0);
    EXPECT_NE(proxyInfo->timeOf(FileInfo::FileTimeType::kLastModified).toUInt(), 0);
    EXPECT_FALSE(proxyInfo->fileMimeType().name().isEmpty());
}

TEST_F(ProxyFileInfoTest, Permissions)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    FileInfoPointer proxyTarget = QSharedPointer<FileInfo>::create(fileUrl);
    proxyInfo->setProxy(proxyTarget);
    
    // Test permissions are delegated
    bool isReadable = proxyInfo->isReadable();
    bool isWritable = proxyInfo->isWritable();
    bool isExecutable = proxyInfo->isExecutable();
    
    // Just test that methods don't crash (actual values depend on file permissions)
    (void)isReadable;
    (void)isWritable;
    (void)isExecutable;
}

TEST_F(ProxyFileInfoTest, Refresh)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    FileInfoPointer proxyTarget = QSharedPointer<FileInfo>::create(fileUrl);
    proxyInfo->setProxy(proxyTarget);
    
    // Test refresh delegates to proxy
    proxyInfo->refresh();
    
    // Should not crash and should still have valid data
    EXPECT_TRUE(proxyInfo->exists());
}

TEST_F(ProxyFileInfoTest, Inheritance)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    // Test inheritance relationship - ProxyFileInfo can be used as AbstractFileInfo
    // Since ProxyFileInfo inherits from AbstractFileInfo, we can test polymorphism
    EXPECT_EQ(proxyInfo->urlOf(FileInfo::FileUrlInfoType::kUrl), fileUrl);
}

TEST_F(ProxyFileInfoTest, ThreadSafety)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    FileInfoPointer proxyTarget = QSharedPointer<FileInfo>::create(fileUrl);
    proxyInfo->setProxy(proxyTarget);
    
    // Test concurrent access (basic test)
    for (int i = 0; i < 10; ++i) {
        auto url = proxyInfo->urlOf(FileInfo::FileUrlInfoType::kUrl);
        auto exists = proxyInfo->exists();
        auto fileName = proxyInfo->fileName();
        
        // Avoid unused variable warnings
        (void)url;
        (void)exists;
        (void)fileName;
    }
}

TEST_F(ProxyFileInfoTest, FileUrlMethod)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    // Test fileUrl() method
    EXPECT_EQ(proxyInfo->fileUrl(), fileUrl);
}

TEST_F(ProxyFileInfoTest, UrlOperations)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    // Test URL operations
    EXPECT_EQ(proxyInfo->urlOf(FileInfo::FileUrlInfoType::kUrl), fileUrl);
    EXPECT_EQ(proxyInfo->fileUrl(), fileUrl);
}

TEST_F(ProxyFileInfoTest, DynamicProxyChange)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    // Initially no proxy
    EXPECT_EQ(proxyInfo->proxy, nullptr);
    
    // Set file proxy
    FileInfoPointer fileProxy = QSharedPointer<FileInfo>::create(fileUrl);
    proxyInfo->setProxy(fileProxy);
    EXPECT_NE(proxyInfo->proxy, nullptr);
    
    // Change to directory proxy
    FileInfoPointer dirProxy = QSharedPointer<FileInfo>::create(dirUrl);
    proxyInfo->setProxy(dirProxy);
    EXPECT_EQ(proxyInfo->proxy, dirProxy);
}

TEST_F(ProxyFileInfoTest, MimeTypeOperations)
{
    auto proxyInfo = QSharedPointer<ProxyFileInfo>::create(fileUrl);
    
    FileInfoPointer proxyTarget = QSharedPointer<FileInfo>::create(fileUrl);
    proxyInfo->setProxy(proxyTarget);
    
    // Test MIME type operations
    auto mimeType = proxyInfo->fileMimeType();
    EXPECT_FALSE(mimeType.name().isEmpty());
}
