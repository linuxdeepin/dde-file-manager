// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/utils/burncheckstrategy.h"
#include "plugins/common/dfmplugin-burn/utils/burnhelper.h"

#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QWidget>

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE

class UT_BurnCheckStrategy : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(ADDR(QWidget, show), [&] {
            __DBG_STUB_INVOKE__
        });
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());
        testPath = tempDir->path();
    }

    virtual void TearDown() override
    {
        delete tempDir;
        tempDir = nullptr;
        stub.clear();
    }

    void createTestFile(const QString &fileName, const QString &content = "test content")
    {
        QFile file(testPath + "/" + fileName);
        ASSERT_TRUE(file.open(QIODevice::WriteOnly));
        QTextStream stream(&file);
        stream << content;
        file.close();
    }

    void createTestDir(const QString &dirName)
    {
        QDir dir(testPath);
        ASSERT_TRUE(dir.mkpath(dirName));
    }

    QString createLongString(int length, char c = 'a')
    {
        return QString(length, c);
    }

public:
    stub_ext::StubExt stub;
    QTemporaryDir *tempDir = nullptr;
    QString testPath;
};

// Base BurnCheckStrategy Tests
TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_Constructor)
{
    BurnCheckStrategy strategy(testPath);

    EXPECT_EQ(strategy.currentStagePath, testPath);
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_check_EmptyDirectory)
{
    BurnCheckStrategy strategy(testPath);

    stub.set_lamda(ADDR(BurnHelper, localFileInfoList), [] {
        __DBG_STUB_INVOKE__
        return QFileInfoList();
    });

    bool result = strategy.check();
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_check_SingleFile)
{
    createTestFile("test.txt");

    BurnCheckStrategy strategy(testPath);

    bool result = strategy.check();
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_check_NonExistentFile)
{
    BurnCheckStrategy strategy("/non/existent/path");

    QFileInfo nonExistentInfo("/non/existent/path");

    stub.set_lamda(ADDR(BurnHelper, localFileInfoList), [&nonExistentInfo] {
        __DBG_STUB_INVOKE__
        return QFileInfoList() << nonExistentInfo;
    });

    bool result = strategy.check();
    EXPECT_TRUE(result);   // Non-existent files should be valid
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_check_DirectoryWithFiles)
{
    createTestDir("subdir");
    createTestFile("subdir/file1.txt");
    createTestFile("subdir/file2.txt");

    BurnCheckStrategy strategy(testPath);

    bool result = strategy.check();
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_lastError)
{
    BurnCheckStrategy strategy(testPath);

    // Initially should be empty
    QString error = strategy.lastError();
    EXPECT_TRUE(error.isEmpty());
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_lastInvalidName)
{
    BurnCheckStrategy strategy(testPath);

    // Initially should be empty
    QString invalidName = strategy.lastInvalidName();
    EXPECT_TRUE(invalidName.isEmpty());
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_autoFeed_ShortString)
{
    BurnCheckStrategy strategy(testPath);

    QString shortText = "short";
    QString result = strategy.autoFeed(shortText);
    EXPECT_EQ(result, shortText);   // Should remain unchanged
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_autoFeed_LongString)
{
    BurnCheckStrategy strategy(testPath);

    QString longText = createLongString(100, 'a');
    QString result = strategy.autoFeed(longText);
    EXPECT_TRUE(result.contains('\n'));   // Should contain line breaks
    EXPECT_GT(result.length(), longText.length());   // Should be longer due to line breaks
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_validCommonFileNameBytes_Valid)
{
    BurnCheckStrategy strategy(testPath);

    QString validName = "valid_filename.txt";
    bool result = strategy.validCommonFileNameBytes(validName);
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_validCommonFileNameBytes_TooLong)
{
    BurnCheckStrategy strategy(testPath);

    QString longName = createLongString(300, 'a') + ".txt";   // > 255 bytes
    bool result = strategy.validCommonFileNameBytes(longName);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_validComontFilePathBytes_Valid)
{
    BurnCheckStrategy strategy(testPath);

    QString validPath = "/path/to/file.txt";
    bool result = strategy.validComontFilePathBytes(validPath);
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_validComontFilePathBytes_TooLong)
{
    BurnCheckStrategy strategy(testPath);

    QString longPath = "/" + createLongString(1100, 'a');   // > 1024 bytes
    bool result = strategy.validComontFilePathBytes(longPath);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_validCommonFilePathDeepLength_Valid)
{
    BurnCheckStrategy strategy(testPath);

    QString validPath = "/a/b/c/d/e/f/g/h";   // 8 levels
    bool result = strategy.validCommonFilePathDeepLength(validPath);
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_validCommonFilePathDeepLength_TooDeep)
{
    BurnCheckStrategy strategy(testPath);

    QString deepPath = "/a/b/c/d/e/f/g/h/i/j";   // > 8 levels
    bool result = strategy.validCommonFilePathDeepLength(deepPath);
    EXPECT_FALSE(result);
}

// ISO9660CheckStrategy Tests
TEST_F(UT_BurnCheckStrategy, ISO9660CheckStrategy_Constructor)
{
    ISO9660CheckStrategy strategy(testPath);

    EXPECT_EQ(strategy.currentStagePath, testPath);
}

TEST_F(UT_BurnCheckStrategy, ISO9660CheckStrategy_validFileNameCharacters_Valid)
{
    ISO9660CheckStrategy strategy(testPath);

    QString validName = "short_name.txt";   // < 32 characters
    bool result = strategy.validFileNameCharacters(validName);
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, ISO9660CheckStrategy_validFileNameCharacters_TooLong)
{
    ISO9660CheckStrategy strategy(testPath);

    QString longName = createLongString(35, 'a') + ".txt";   // > 32 characters
    bool result = strategy.validFileNameCharacters(longName);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnCheckStrategy, ISO9660CheckStrategy_validFilePathDeepLength)
{
    ISO9660CheckStrategy strategy(testPath);

    QString validPath = "/a/b/c/d/e/f/g/h";   // 8 levels
    bool result = strategy.validFilePathDeepLength(validPath);
    EXPECT_TRUE(result);

    QString deepPath = "/a/b/c/d/e/f/g/h/i/j";   // > 8 levels
    result = strategy.validFilePathDeepLength(deepPath);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnCheckStrategy, ISO9660CheckStrategy_check_ValidFiles)
{
    createTestFile("short.txt");
    createTestDir("dir");
    createTestFile("dir/file.txt");

    ISO9660CheckStrategy strategy(testPath);

    bool result = strategy.check();
    EXPECT_TRUE(result);
}

// JolietCheckStrategy Tests
TEST_F(UT_BurnCheckStrategy, JolietCheckStrategy_Constructor)
{
    JolietCheckStrategy strategy(testPath);

    EXPECT_EQ(strategy.currentStagePath, testPath);
}

TEST_F(UT_BurnCheckStrategy, JolietCheckStrategy_validFileNameCharacters_Valid)
{
    JolietCheckStrategy strategy(testPath);

    QString validName = createLongString(59, 'a') + ".txt";   // < 64 characters
    bool result = strategy.validFileNameCharacters(validName);
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, JolietCheckStrategy_validFileNameCharacters_TooLong)
{
    JolietCheckStrategy strategy(testPath);

    QString longName = createLongString(70, 'a') + ".txt";   // > 64 characters
    bool result = strategy.validFileNameCharacters(longName);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnCheckStrategy, JolietCheckStrategy_validFilePathCharacters_Valid)
{
    JolietCheckStrategy strategy(testPath);

    QString validPath = "/path/to/" + createLongString(100, 'a');   // < 120 characters
    bool result = strategy.validFilePathCharacters(validPath);
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, JolietCheckStrategy_validFilePathCharacters_TooLong)
{
    JolietCheckStrategy strategy(testPath);

    QString longPath = "/path/to/" + createLongString(130, 'a');   // > 120 characters
    bool result = strategy.validFilePathCharacters(longPath);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnCheckStrategy, JolietCheckStrategy_check_ValidFiles)
{
    createTestFile("medium_name.txt");
    createTestDir("subdir");
    createTestFile("subdir/another.txt");

    JolietCheckStrategy strategy(testPath);

    bool result = strategy.check();
    EXPECT_TRUE(result);
}

// RockRidgeCheckStrategy Tests
TEST_F(UT_BurnCheckStrategy, RockRidgeCheckStrategy_Constructor)
{
    RockRidgeCheckStrategy strategy(testPath);

    EXPECT_EQ(strategy.currentStagePath, testPath);
}

TEST_F(UT_BurnCheckStrategy, RockRidgeCheckStrategy_validFileNameBytes_Valid)
{
    RockRidgeCheckStrategy strategy(testPath);

    QString validName = createLongString(200, 'a') + ".txt";   // < 255 bytes
    bool result = strategy.validFileNameBytes(validName);
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, RockRidgeCheckStrategy_validFileNameBytes_TooLong)
{
    RockRidgeCheckStrategy strategy(testPath);

    QString longName = createLongString(300, 'a') + ".txt";   // > 255 bytes
    bool result = strategy.validFileNameBytes(longName);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnCheckStrategy, RockRidgeCheckStrategy_validFilePathBytes_Valid)
{
    RockRidgeCheckStrategy strategy(testPath);

    QString validPath = "/path/" + createLongString(900, 'a');   // < 1024 bytes
    bool result = strategy.validFilePathBytes(validPath);
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, RockRidgeCheckStrategy_validFilePathBytes_TooLong)
{
    RockRidgeCheckStrategy strategy(testPath);

    QString longPath = "/" + createLongString(1100, 'a');   // > 1024 bytes
    bool result = strategy.validFilePathBytes(longPath);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnCheckStrategy, RockRidgeCheckStrategy_validFilePathDeepLength)
{
    RockRidgeCheckStrategy strategy(testPath);

    QString validPath = "/a/b/c/d/e/f/g/h";   // 8 levels
    bool result = strategy.validFilePathDeepLength(validPath);
    EXPECT_TRUE(result);

    QString deepPath = "/a/b/c/d/e/f/g/h/i/j";   // > 8 levels
    result = strategy.validFilePathDeepLength(deepPath);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnCheckStrategy, RockRidgeCheckStrategy_check_ValidFiles)
{
    createTestFile("long_filename_but_within_limits.txt");
    createTestDir("subdir");
    createTestFile("subdir/another_file.txt");

    RockRidgeCheckStrategy strategy(testPath);

    bool result = strategy.check();
    EXPECT_TRUE(result);
}

// UDFCheckStrategy Tests
TEST_F(UT_BurnCheckStrategy, UDFCheckStrategy_Constructor)
{
    UDFCheckStrategy strategy(testPath);

    EXPECT_EQ(strategy.currentStagePath, testPath);
}

TEST_F(UT_BurnCheckStrategy, UDFCheckStrategy_validFileNameBytes_Valid)
{
    UDFCheckStrategy strategy(testPath);

    QString validName = createLongString(200, 'a') + ".txt";   // < 255 bytes
    bool result = strategy.validFileNameBytes(validName);
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, UDFCheckStrategy_validFileNameBytes_TooLong)
{
    UDFCheckStrategy strategy(testPath);

    QString longName = createLongString(300, 'a') + ".txt";   // > 255 bytes
    bool result = strategy.validFileNameBytes(longName);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnCheckStrategy, UDFCheckStrategy_validFilePathBytes_Valid)
{
    UDFCheckStrategy strategy(testPath);

    QString validPath = "/path/" + createLongString(900, 'a');   // < 1024 bytes
    bool result = strategy.validFilePathBytes(validPath);
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, UDFCheckStrategy_validFilePathBytes_TooLong)
{
    UDFCheckStrategy strategy(testPath);

    QString longPath = "/" + createLongString(1100, 'a');   // > 1024 bytes
    bool result = strategy.validFilePathBytes(longPath);
    EXPECT_FALSE(result);
}

TEST_F(UT_BurnCheckStrategy, UDFCheckStrategy_check_ValidFiles)
{
    createTestFile("very_long_filename_that_is_still_within_udf_limits.txt");
    createTestDir("subdir");
    createTestFile("subdir/another_long_filename.txt");

    UDFCheckStrategy strategy(testPath);

    bool result = strategy.check();
    EXPECT_TRUE(result);
}

// Integration Tests
TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_check_ComplexDirectoryStructure)
{
    // Create a complex directory structure
    createTestDir("level1");
    createTestDir("level1/level2");
    createTestDir("level1/level2/level3");
    createTestFile("level1/file1.txt");
    createTestFile("level1/level2/file2.txt");
    createTestFile("level1/level2/level3/file3.txt");

    BurnCheckStrategy strategy(testPath);

    bool result = strategy.check();
    EXPECT_TRUE(result);
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_check_FileDoesNotExist)
{
    BurnCheckStrategy strategy(testPath);

    // Mock BurnHelper to return a non-existent file
    QFileInfo nonExistentFile("/non/existent/file.txt");
    stub.set_lamda(ADDR(BurnHelper, localFileInfoList), [&nonExistentFile] {
        __DBG_STUB_INVOKE__
        return QFileInfoList() << nonExistentFile;
    });

    bool result = strategy.check();
    EXPECT_TRUE(result);   // Non-existent files should pass validation
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_check_RegularFile)
{
    createTestFile("regular.txt");

    BurnCheckStrategy strategy(testPath);

    // Test with regular file (not directory)
    QString filePath = testPath + "/regular.txt";
    BurnCheckStrategy fileStrategy(filePath);

    bool result = fileStrategy.check();
    EXPECT_TRUE(result);   // Regular files should pass
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_validFile_ErrorScenarios)
{
    createTestFile("test.txt");

    // Create a strategy that will fail validation
    class TestStrategy : public BurnCheckStrategy
    {
    public:
        TestStrategy(const QString &path)
            : BurnCheckStrategy(path) { }

        bool validFileNameCharacters(const QString &fileName) override
        {
            Q_UNUSED(fileName)
            return false;   // Always fail
        }
    };

    TestStrategy strategy(testPath);

    bool result = strategy.check();
    EXPECT_FALSE(result);
    EXPECT_FALSE(strategy.lastError().isEmpty());
    EXPECT_FALSE(strategy.lastInvalidName().isEmpty());
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_ErrorMessages)
{
    createTestFile("test.txt");

    // Test different error scenarios
    class TestStrategy : public BurnCheckStrategy
    {
    public:
        TestStrategy(const QString &path, int errorType)
            : BurnCheckStrategy(path), errorType(errorType) { }

        bool validFileNameCharacters(const QString &fileName) override
        {
            return errorType != 1;
        }

        bool validFilePathCharacters(const QString &filePath) override
        {
            return errorType != 2;
        }

        bool validFileNameBytes(const QString &fileName) override
        {
            return errorType != 3;
        }

        bool validFilePathBytes(const QString &filePath) override
        {
            return errorType != 4;
        }

        bool validFilePathDeepLength(const QString &filePath) override
        {
            return errorType != 5;
        }

    private:
        int errorType;
    };

    // Test FileNameCharacters error
    TestStrategy strategy1(testPath, 1);
    EXPECT_FALSE(strategy1.check());
    EXPECT_TRUE(strategy1.lastError().contains("Invalid FileNameCharacters Length"));

    // Test FilePathCharacters error
    TestStrategy strategy2(testPath, 2);
    EXPECT_FALSE(strategy2.check());
    EXPECT_TRUE(strategy2.lastError().contains("Invalid FilePathCharacters Length"));

    // Test FileNameBytes error
    TestStrategy strategy3(testPath, 3);
    EXPECT_FALSE(strategy3.check());
    EXPECT_TRUE(strategy3.lastError().contains("Invalid FileNameBytes Length"));

    // Test FilePathBytes error
    TestStrategy strategy4(testPath, 4);
    EXPECT_FALSE(strategy4.check());
    EXPECT_TRUE(strategy4.lastError().contains("Invalid FilePathBytes Length"));

    // Test FilePathDeepLength error
    TestStrategy strategy5(testPath, 5);
    EXPECT_FALSE(strategy5.check());
    EXPECT_TRUE(strategy5.lastError().contains("Invalid FilePathDeepLength"));
}

TEST_F(UT_BurnCheckStrategy, BurnCheckStrategy_RecursiveDirectoryCheck)
{
    // Create nested directory structure
    createTestDir("deep");
    createTestDir("deep/nested");
    createTestDir("deep/nested/structure");
    createTestFile("deep/file1.txt");
    createTestFile("deep/nested/file2.txt");
    createTestFile("deep/nested/structure/file3.txt");

    BurnCheckStrategy strategy(testPath);

    // Mock BurnHelper methods
    stub.set_lamda(ADDR(BurnHelper, localFileInfoListRecursive), [] {
        __DBG_STUB_INVOKE__
        QFileInfoList list;
        list << QFileInfo("/test/file1.txt");
        list << QFileInfo("/test/file2.txt");
        list << QFileInfo("/test/file3.txt");
        return list;
    });

    bool result = strategy.check();
    EXPECT_TRUE(result);
}
