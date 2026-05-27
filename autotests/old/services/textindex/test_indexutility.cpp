// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QTemporaryDir>
#include <QFileInfo>

#include "utils/indexutility.h"
#include "utils/textindexconfig.h"

SERVICETEXTINDEX_USE_NAMESPACE

class UT_IndexUtility : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create temporary directory for testing
        tempDir = std::make_unique<QTemporaryDir>();
        EXPECT_TRUE(tempDir->isValid());

        testIndexDir = tempDir->path() + "/index";
        QDir().mkpath(testIndexDir);

        // Mock DFMSEARCH::Global functions
        stub.set_lamda(ADDR(DFMSEARCH::Global, isFileNameIndexReadyForSearch), []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(ADDR(DFMSEARCH::Global, defaultIndexedDirectory), []() -> QStringList {
            __DBG_STUB_INVOKE__
            return QStringList { "/home/test", "/home/user/Documents" };
        });

        stub.set_lamda(ADDR(DFMSEARCH::Global, isContentIndexAvailable), []() -> bool {
            __DBG_STUB_INVOKE__
            return true;
        });

        stub.set_lamda(ADDR(DFMSEARCH::Global, isPathInContentIndexDirectory), [](const QString &path) -> bool {
            __DBG_STUB_INVOKE__
            return path.startsWith("/home/test") || path.startsWith("/home/user/Documents");
        });

        stub.set_lamda(ADDR(DFMSEARCH::Global, contentIndexDirectory), [this]() -> QString {
            __DBG_STUB_INVOKE__
            return testIndexDir;
        });

        // Mock TextIndexConfig
        stub.set_lamda(ADDR(TextIndexConfig, instance), []() -> TextIndexConfig & {
            __DBG_STUB_INVOKE__
            static TextIndexConfig mockConfig;
            return mockConfig;
        });

        stub.set_lamda(ADDR(TextIndexConfig, maxIndexFileSizeMB), [this](TextIndexConfig *) -> int {
            __DBG_STUB_INVOKE__
            return mockMaxFileSizeMB;
        });

        stub.set_lamda(ADDR(TextIndexConfig, supportedFileExtensions), [this](TextIndexConfig *) -> QStringList {
            __DBG_STUB_INVOKE__
            return mockSupportedExtensions;
        });
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    void createTestStatusFile(const QDateTime &lastUpdate = QDateTime::currentDateTime(), int version = 1)
    {
        QJsonObject status;
        status["lastUpdateTime"] = lastUpdate.toString(Qt::ISODate);
        status["version"] = version;

        QJsonDocument doc(status);
        QString statusPath = testIndexDir + "/index_status.json";

        QFile file(statusPath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));
        file.write(doc.toJson());
        file.close();
    }

    void createTestFile(const QString &relativePath, qint64 sizeBytes = 1024)
    {
        QString fullPath = testIndexDir + "/" + relativePath;
        QDir().mkpath(QFileInfo(fullPath).absolutePath());

        QFile file(fullPath);
        EXPECT_TRUE(file.open(QIODevice::WriteOnly));

        // Write data to reach desired size
        QByteArray data(sizeBytes, 'A');
        file.write(data);
        file.close();
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    QString testIndexDir;
    int mockMaxFileSizeMB = 50;
    QStringList mockSupportedExtensions = { "txt", "pdf", "doc", "docx", "html" };

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_IndexUtility, IsIndexWithAnything_FileNameIndexReady_ReturnsTrue)
{
    bool result = IndexUtility::isIndexWithAnything("/home/test");

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexUtility, IsIndexWithAnything_FileNameIndexNotReady_ReturnsFalse)
{
    stub.set_lamda(ADDR(DFMSEARCH::Global, isFileNameIndexReadyForSearch), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = IndexUtility::isIndexWithAnything("/home/test/document.txt");

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexUtility, IsDefaultIndexedDirectory_ValidPath_ReturnsTrue)
{
    bool result = IndexUtility::isDefaultIndexedDirectory("/home/test");

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexUtility, IsDefaultIndexedDirectory_InvalidPath_ReturnsFalse)
{
    bool result = IndexUtility::isDefaultIndexedDirectory("/invalid/path");

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexUtility, IsPathInContentIndexDirectory_ValidPath_ReturnsTrue)
{
    bool result = IndexUtility::isPathInContentIndexDirectory("/home/test/subfolder");

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexUtility, IsPathInContentIndexDirectory_InvalidPath_ReturnsFalse)
{
    bool result = IndexUtility::isPathInContentIndexDirectory("/invalid/path");

    EXPECT_FALSE(result);
}

// Additional comprehensive tests for directory matching edge cases
TEST_F(UT_IndexUtility, IsPathInContentIndexDirectory_ExactMatch_ReturnsTrue)
{
    // Test exact match for indexed directory
    bool result1 = IndexUtility::isPathInContentIndexDirectory("/home/test");
    bool result2 = IndexUtility::isPathInContentIndexDirectory("/home/user/Documents");

    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
}

TEST_F(UT_IndexUtility, IsPathInContentIndexDirectory_ExactMatchWithTrailingSlash_ReturnsTrue)
{
    // Test exact match with trailing slash
    bool result1 = IndexUtility::isPathInContentIndexDirectory("/home/test/");
    bool result2 = IndexUtility::isPathInContentIndexDirectory("/home/user/Documents/");

    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
}

TEST_F(UT_IndexUtility, IsPathInContentIndexDirectory_FalsePositivePrevention_ReturnsFalse)
{
    // Test prevention of false positives - these should NOT match
    bool result1 = IndexUtility::isPathInContentIndexDirectory("/home/test_backup");  // Similar but different
    bool result2 = IndexUtility::isPathInContentIndexDirectory("/home/testing");      // Starts with indexed dir name
    bool result3 = IndexUtility::isPathInContentIndexDirectory("/home/user/Documents_old");  // Similar pattern
    bool result4 = IndexUtility::isPathInContentIndexDirectory("/home/user/DocumentsBackup"); // Another similar pattern

    EXPECT_FALSE(result1);
    EXPECT_FALSE(result2);
    EXPECT_FALSE(result3);
    EXPECT_FALSE(result4);
}

TEST_F(UT_IndexUtility, IsPathInContentIndexDirectory_DeepSubdirectory_ReturnsTrue)
{
    // Test deep subdirectories
    bool result1 = IndexUtility::isPathInContentIndexDirectory("/home/test/deep/nested/folder/file.txt");
    bool result2 = IndexUtility::isPathInContentIndexDirectory("/home/user/Documents/projects/myproject/src");

    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
}

TEST_F(UT_IndexUtility, IsPathInContentIndexDirectory_PathWithTrailingSlash_ReturnsTrue)
{
    // Test subdirectories with trailing slash
    bool result1 = IndexUtility::isPathInContentIndexDirectory("/home/test/subfolder/");
    bool result2 = IndexUtility::isPathInContentIndexDirectory("/home/user/Documents/projects/");

    EXPECT_TRUE(result1);
    EXPECT_TRUE(result2);
}

TEST_F(UT_IndexUtility, StatusFilePath_ReturnsCorrectPath)
{
    QString result = IndexUtility::statusFilePath();

    EXPECT_EQ(result, testIndexDir + "/index_status.json");
}

TEST_F(UT_IndexUtility, SaveIndexStatus_ValidDateTime_CreatesFile)
{
    QDateTime testTime = QDateTime::fromString("2025-01-01T12:00:00", Qt::ISODate);

    IndexUtility::saveIndexStatus(testTime);

    QString statusPath = testIndexDir + "/index_status.json";
    EXPECT_TRUE(QFile::exists(statusPath));

    // Verify content
    QFile file(statusPath);
    EXPECT_TRUE(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();

    EXPECT_EQ(obj["lastUpdateTime"].toString(), testTime.toString(Qt::ISODate));
    EXPECT_EQ(obj["version"].toInt(), 1);   // Default version
}

TEST_F(UT_IndexUtility, SaveIndexStatus_WithVersion_CreatesFileWithCorrectVersion)
{
    QDateTime testTime = QDateTime::fromString("2025-01-01T12:00:00", Qt::ISODate);
    int testVersion = 5;

    IndexUtility::saveIndexStatus(testTime, testVersion);

    QString statusPath = testIndexDir + "/index_status.json";
    QFile file(statusPath);
    EXPECT_TRUE(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();

    EXPECT_EQ(obj["version"].toInt(), testVersion);
}

TEST_F(UT_IndexUtility, GetLastUpdateTime_ValidFile_ReturnsFormattedTime)
{
    QDateTime testTime = QDateTime::fromString("2025-01-01T12:00:00", Qt::ISODate);
    createTestStatusFile(testTime);

    QString result = IndexUtility::getLastUpdateTime();

    EXPECT_EQ(result, "2025-01-01 12:00:00");
}

TEST_F(UT_IndexUtility, GetLastUpdateTime_NoFile_ReturnsEmptyString)
{
    QString result = IndexUtility::getLastUpdateTime();

    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_IndexUtility, GetIndexVersion_ValidFile_ReturnsCorrectVersion)
{
    createTestStatusFile(QDateTime::currentDateTime(), 3);

    int result = IndexUtility::getIndexVersion();

    EXPECT_EQ(result, 3);
}

TEST_F(UT_IndexUtility, GetIndexVersion_NoFile_ReturnsMinusOne)
{
    int result = IndexUtility::getIndexVersion();

    EXPECT_EQ(result, -1);
}

TEST_F(UT_IndexUtility, IsCompatibleVersion_MatchingVersion_ReturnsTrue)
{
    createTestStatusFile(QDateTime::currentDateTime(), 1);

    bool result = IndexUtility::isCompatibleVersion();

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexUtility, IsCompatibleVersion_DifferentVersion_ReturnsFalse)
{
    createTestStatusFile(QDateTime::currentDateTime(), 3);

    bool result = IndexUtility::isCompatibleVersion();

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexUtility, IsCompatibleVersion_NoFile_ReturnsFalse)
{
    bool result = IndexUtility::isCompatibleVersion();

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexUtility, RemoveIndexStatusFile_FileExists_RemovesFile)
{
    createTestStatusFile();
    QString statusPath = testIndexDir + "/index_status.json";
    EXPECT_TRUE(QFile::exists(statusPath));

    IndexUtility::removeIndexStatusFile();

    EXPECT_FALSE(QFile::exists(statusPath));
}

TEST_F(UT_IndexUtility, RemoveIndexStatusFile_NoFile_DoesNotCrash)
{
    EXPECT_NO_THROW({
        IndexUtility::removeIndexStatusFile();
    });
}

TEST_F(UT_IndexUtility, ClearIndexDirectory_WithFiles_RemovesAllFiles)
{
    // Create test files
    createTestFile("test1.txt");
    createTestFile("test2.pdf");
    createTestFile("subdir/test3.doc");

    IndexUtility::clearIndexDirectory();

    // Check that files in root directory are removed
    EXPECT_FALSE(QFile::exists(testIndexDir + "/test1.txt"));
    EXPECT_FALSE(QFile::exists(testIndexDir + "/test2.pdf"));

    // Directory should still exist
    EXPECT_TRUE(QDir(testIndexDir).exists());
}

TEST_F(UT_IndexUtility, CheckFileSize_SmallFile_ReturnsTrue)
{
    createTestFile("small.txt", 1024);   // 1KB
    QFileInfo fileInfo(testIndexDir + "/small.txt");

    bool result = IndexUtility::checkFileSize(fileInfo);

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexUtility, CheckFileSize_LargeFile_ReturnsFalse)
{
    mockMaxFileSizeMB = 1;   // 1MB limit
    createTestFile("large.txt", 2 * 1024 * 1024);   // 2MB
    QFileInfo fileInfo(testIndexDir + "/large.txt");

    bool result = IndexUtility::checkFileSize(fileInfo);

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexUtility, IsSupportedFile_SupportedExtension_ReturnsTrue)
{
    createTestFile("document.txt", 1024);
    QString filePath = testIndexDir + "/document.txt";

    bool result = IndexUtility::isSupportedFile(filePath);

    EXPECT_TRUE(result);
}

TEST_F(UT_IndexUtility, IsSupportedFile_UnsupportedExtension_ReturnsFalse)
{
    createTestFile("binary.exe", 1024);
    QString filePath = testIndexDir + "/binary.exe";

    bool result = IndexUtility::isSupportedFile(filePath);

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexUtility, IsSupportedFile_TooLargeFile_ReturnsFalse)
{
    mockMaxFileSizeMB = 1;   // 1MB limit
    createTestFile("large.txt", 2 * 1024 * 1024);   // 2MB
    QString filePath = testIndexDir + "/large.txt";

    bool result = IndexUtility::isSupportedFile(filePath);

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexUtility, IsSupportedFile_NonExistentFile_ReturnsFalse)
{
    QString filePath = testIndexDir + "/nonexistent.txt";

    bool result = IndexUtility::isSupportedFile(filePath);

    EXPECT_FALSE(result);
}

TEST_F(UT_IndexUtility, IsSupportedFile_Directory_ReturnsFalse)
{
    QString dirPath = testIndexDir + "/testdir";
    QDir().mkpath(dirPath);

    bool result = IndexUtility::isSupportedFile(dirPath);

    EXPECT_FALSE(result);
}
