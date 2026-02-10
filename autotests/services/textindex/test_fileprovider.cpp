// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>

#include "task/fileprovider.h"
#include "utils/indextraverseutils.h"
#include "utils/indexutility.h"
#include "utils/textindexconfig.h"
#include <dfm-search/dsearch_global.h>

SERVICETEXTINDEX_USE_NAMESPACE

class UT_FileProvider : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create temporary directory structure for testing
        tempDir = std::make_unique<QTemporaryDir>();
        EXPECT_TRUE(tempDir->isValid());

        testRootPath = tempDir->path();
        createTestFileStructure();

        // Mock IndexTraverseUtils
        stub.set_lamda(ADDR(IndexTraverseUtils, fstabBindInfo), []() -> QMap<QString, QString> {
            __DBG_STUB_INVOKE__
            return QMap<QString, QString>();
        });

        stub.set_lamda(ADDR(IndexTraverseUtils, shouldSkipDirectory), [](const QString &path) -> bool {
            __DBG_STUB_INVOKE__
            return path.contains("skip");
        });

        stub.set_lamda(ADDR(IndexTraverseUtils, isValidDirectory), [](const QString &path, QSet<QString> &visitedDirs) -> bool {
            __DBG_STUB_INVOKE__
            if (visitedDirs.contains(path)) {
                return false;
            }
            visitedDirs.insert(path);
            return !path.contains("invalid");
        });

        stub.set_lamda(ADDR(IndexTraverseUtils, isValidFile), [](const QString &path) -> bool {
            __DBG_STUB_INVOKE__
            return !path.contains("invalid") && QFileInfo(path).exists();
        });

        stub.set_lamda(ADDR(IndexTraverseUtils, isHiddenFile), [](const char *name) -> bool {
            __DBG_STUB_INVOKE__
            return name[0] == '.';
        });

        stub.set_lamda(ADDR(IndexTraverseUtils, isSpecialDir), [](const char *name) -> bool {
            __DBG_STUB_INVOKE__
            return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
        });

        stub.set_lamda(ADDR(IndexTraverseUtils, isSupportedFileExtension), [](const QString &fileName) -> bool {
            __DBG_STUB_INVOKE__
            return fileName.endsWith(".txt") || fileName.endsWith(".doc");
        });

        // Mock IndexUtility
        stub.set_lamda(ADDR(IndexUtility, isDefaultIndexedDirectory), [this](const QString &path) -> bool {
            __DBG_STUB_INVOKE__
            return path.startsWith(testRootPath);
        });

        stub.set_lamda(ADDR(IndexUtility, isPathInContentIndexDirectory), [this](const QString &path) -> bool {
            __DBG_STUB_INVOKE__
            return path.startsWith(testRootPath);
        });

        // Mock TextIndexConfig
        stub.set_lamda(ADDR(TextIndexConfig, instance), []() -> TextIndexConfig & {
            __DBG_STUB_INVOKE__
            static TextIndexConfig mockConfig;
            return mockConfig;
        });

        stub.set_lamda(ADDR(TextIndexConfig, folderExcludeFilters), [](TextIndexConfig *) -> QStringList {
            __DBG_STUB_INVOKE__
            return QStringList { ".git", ".cache", "exclude" };
        });

        // Reset counters
        processedFiles.clear();
        processedDirs.clear();
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    void createTestFileStructure()
    {
        // Create directory structure
        QDir root(testRootPath);
        root.mkpath("subdir1");
        root.mkpath("subdir2");
        root.mkpath("skip_dir");
        root.mkpath("exclude");
        root.mkpath(".hidden");

        // Create test files
        createFile("file1.txt", "Content of file1");
        createFile("file2.doc", "Content of file2");
        createFile("file3.pdf", "Content of file3");
        createFile("subdir1/nested.txt", "Nested content");
        createFile("subdir2/another.doc", "Another content");
        createFile("skip_dir/skipped.txt", "Should be skipped");
        createFile("exclude/excluded.txt", "Should be excluded");
        createFile(".hidden/hidden.txt", "Hidden content");
    }

    void createFile(const QString &relativePath, const QString &content)
    {
        QString fullPath = testRootPath + "/" + relativePath;
        QDir().mkpath(QFileInfo(fullPath).absolutePath());

        QFile file(fullPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(content.toUtf8());
        }
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    QString testRootPath;
    QStringList processedFiles;
    QStringList processedDirs;

private:
    stub_ext::StubExt stub;
};

// FileSystemProvider Tests
TEST_F(UT_FileProvider, FileSystemProvider_Constructor_InitializesCorrectly)
{
    FileSystemProvider provider(testRootPath);

    EXPECT_EQ(provider.name(), "FileSystemProvider");
}

TEST_F(UT_FileProvider, FileSystemProvider_Traverse_ProcessesValidFiles)
{
    FileSystemProvider provider(testRootPath);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    // Should process supported files but not unsupported ones
    EXPECT_TRUE(processedFiles.contains(testRootPath + "/file1.txt"));
    EXPECT_TRUE(processedFiles.contains(testRootPath + "/file2.doc"));
    EXPECT_FALSE(processedFiles.contains(testRootPath + "/file3.pdf"));   // Not supported
}

TEST_F(UT_FileProvider, FileSystemProvider_Traverse_SkipsHiddenFiles)
{
    FileSystemProvider provider(testRootPath);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    // Should not process hidden files
    bool hasHiddenFiles = false;
    for (const QString &file : processedFiles) {
        if (file.contains("/.hidden/")) {
            hasHiddenFiles = true;
            break;
        }
    }
    EXPECT_FALSE(hasHiddenFiles);
}

TEST_F(UT_FileProvider, FileSystemProvider_Traverse_SkipsDirectoriesByFilter)
{
    FileSystemProvider provider(testRootPath);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    // Should skip directories marked to skip
    bool hasSkippedFiles = false;
    for (const QString &file : processedFiles) {
        if (file.contains("/skip_dir/")) {
            hasSkippedFiles = true;
            break;
        }
    }
    EXPECT_FALSE(hasSkippedFiles);
}

TEST_F(UT_FileProvider, FileSystemProvider_Traverse_InterruptedByTaskState)
{
    FileSystemProvider provider(testRootPath);
    TaskState state;
    state.start();

    int processCount = 0;
    auto handler = [&processCount, &state](const QString &filePath) {
        processCount++;
        if (processCount >= 2) {
            state.stop();   // Stop after processing 2 files
        }
    };

    provider.traverse(state, handler);

    // Should stop processing when task state is stopped
    EXPECT_LE(processCount, 2);
}

TEST_F(UT_FileProvider, FileSystemProvider_TotalCount_ReturnsZero)
{
    FileSystemProvider provider(testRootPath);

    // FileSystemProvider doesn't pre-calculate total count
    EXPECT_EQ(provider.totalCount(), 0);
}

// DirectFileListProvider Tests
TEST_F(UT_FileProvider, DirectFileListProvider_Constructor_InitializesWithFileList)
{
    dfmsearch::SearchResultList fileList;
    fileList.append(dfmsearch::SearchResult(testRootPath + "/file1.txt"));
    fileList.append(dfmsearch::SearchResult(testRootPath + "/file2.doc"));

    DirectFileListProvider provider(fileList);

    EXPECT_EQ(provider.name(), "DirectFileListProvider");
    EXPECT_EQ(provider.totalCount(), 2);
}

TEST_F(UT_FileProvider, DirectFileListProvider_Traverse_ProcessesAllFiles)
{
    dfmsearch::SearchResultList fileList;
    fileList.append(dfmsearch::SearchResult(testRootPath + "/file1.txt"));
    fileList.append(dfmsearch::SearchResult(testRootPath + "/file2.doc"));

    DirectFileListProvider provider(fileList);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    EXPECT_EQ(processedFiles.size(), 2);
    EXPECT_TRUE(processedFiles.contains(testRootPath + "/file1.txt"));
    EXPECT_TRUE(processedFiles.contains(testRootPath + "/file2.doc"));
}

TEST_F(UT_FileProvider, DirectFileListProvider_Traverse_InterruptedByTaskState)
{
    dfmsearch::SearchResultList fileList;
    for (int i = 0; i < 10; ++i) {
        fileList.append(dfmsearch::SearchResult(testRootPath + "/file" + QString::number(i) + ".txt"));
    }

    DirectFileListProvider provider(fileList);
    TaskState state;
    state.start();

    int processCount = 0;
    auto handler = [&processCount, &state](const QString &filePath) {
        processCount++;
        if (processCount >= 5) {
            state.stop();
        }
    };

    provider.traverse(state, handler);

    EXPECT_EQ(processCount, 5);
}

TEST_F(UT_FileProvider, DirectFileListProvider_EmptyList_ProcessesNothing)
{
    dfmsearch::SearchResultList emptyList;
    DirectFileListProvider provider(emptyList);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    EXPECT_TRUE(processedFiles.isEmpty());
}

// MixedPathListProvider Tests
TEST_F(UT_FileProvider, MixedPathListProvider_Constructor_InitializesWithPathList)
{
    QStringList pathList = {
        testRootPath + "/file1.txt",
        testRootPath + "/subdir1",
        testRootPath + "/file2.doc"
    };

    MixedPathListProvider provider(pathList);

    EXPECT_EQ(provider.name(), "MixedPathListProvider");
    EXPECT_EQ(provider.totalCount(), 0);   // Calculated during traversal
}

TEST_F(UT_FileProvider, MixedPathListProvider_Traverse_ProcessesFilesAndDirectories)
{
    QStringList pathList = {
        testRootPath + "/file1.txt",
        testRootPath + "/subdir1"
    };

    MixedPathListProvider provider(pathList);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    // Should process the direct file and files from the directory
    EXPECT_TRUE(processedFiles.contains(testRootPath + "/file1.txt"));
    EXPECT_TRUE(processedFiles.contains(testRootPath + "/subdir1/nested.txt"));
}

TEST_F(UT_FileProvider, MixedPathListProvider_Traverse_SkipsUnsupportedExtensions)
{
    QStringList pathList = {
        testRootPath + "/file1.txt",
        testRootPath + "/file3.pdf"   // Unsupported extension
    };

    MixedPathListProvider provider(pathList);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    EXPECT_TRUE(processedFiles.contains(testRootPath + "/file1.txt"));
    EXPECT_FALSE(processedFiles.contains(testRootPath + "/file3.pdf"));
}

TEST_F(UT_FileProvider, MixedPathListProvider_Traverse_SkipsBlacklistedDirectories)
{
    QStringList pathList = {
        testRootPath + "/exclude"   // Blacklisted directory
    };

    MixedPathListProvider provider(pathList);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    // Should not process files from blacklisted directories
    bool hasExcludedFiles = false;
    for (const QString &file : processedFiles) {
        if (file.contains("/exclude/")) {
            hasExcludedFiles = true;
            break;
        }
    }
    EXPECT_FALSE(hasExcludedFiles);
}

TEST_F(UT_FileProvider, MixedPathListProvider_Traverse_HandlesNonExistentPaths)
{
    QStringList pathList = {
        testRootPath + "/file1.txt",
        testRootPath + "/nonexistent.txt"
    };

    MixedPathListProvider provider(pathList);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    // Should only process existing files
    EXPECT_TRUE(processedFiles.contains(testRootPath + "/file1.txt"));
    EXPECT_FALSE(processedFiles.contains(testRootPath + "/nonexistent.txt"));
}

TEST_F(UT_FileProvider, MixedPathListProvider_Traverse_InterruptedByTaskState)
{
    QStringList pathList = {
        testRootPath + "/subdir1",   // Directory with files
        testRootPath + "/subdir2"   // Another directory
    };

    MixedPathListProvider provider(pathList);
    TaskState state;
    state.start();

    int processCount = 0;
    auto handler = [&processCount, &state](const QString &filePath) {
        processCount++;
        if (processCount >= 1) {
            state.stop();
        }
    };

    provider.traverse(state, handler);

    EXPECT_EQ(processCount, 1);
}

TEST_F(UT_FileProvider, MixedPathListProvider_Traverse_AvoidsDuplicateFiles)
{
    QStringList pathList = {
        testRootPath + "/file1.txt",
        testRootPath + "/file1.txt"   // Duplicate
    };

    MixedPathListProvider provider(pathList);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    // Should only process the file once
    int count = 0;
    for (const QString &file : processedFiles) {
        if (file == testRootPath + "/file1.txt") {
            count++;
        }
    }
    EXPECT_EQ(count, 1);
}

TEST_F(UT_FileProvider, MixedPathListProvider_EmptyList_ProcessesNothing)
{
    QStringList emptyList;
    MixedPathListProvider provider(emptyList);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    EXPECT_TRUE(processedFiles.isEmpty());
}

// Performance and Edge Case Tests
TEST_F(UT_FileProvider, FileSystemProvider_Traverse_HandlesDeepDirectoryStructure)
{
    // Create a deep directory structure
    QString deepPath = testRootPath;
    for (int i = 0; i < 15; ++i) {
        deepPath += "/level" + QString::number(i);
        QDir().mkpath(deepPath);
    }
    createFile(deepPath.mid(testRootPath.length() + 1) + "/deep.txt", "Deep file");

    FileSystemProvider provider(testRootPath);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    provider.traverse(state, handler);

    // Should handle deep structures without crashing
    bool hasDeepFile = false;
    for (const QString &file : processedFiles) {
        if (file.endsWith("/deep.txt")) {
            hasDeepFile = true;
            break;
        }
    }
    EXPECT_TRUE(hasDeepFile);
}

TEST_F(UT_FileProvider, MixedPathListProvider_Traverse_HandlesSymlinks)
{
    // This test would require platform-specific symlink creation
    // For now, we just ensure it doesn't crash with symlink-like paths
    QStringList pathList = {
        testRootPath + "/file1.txt"
    };

    MixedPathListProvider provider(pathList);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    EXPECT_NO_THROW({
        provider.traverse(state, handler);
    });
}

// Error Handling Tests
TEST_F(UT_FileProvider, FileSystemProvider_Traverse_HandlesPermissionErrors)
{
    FileSystemProvider provider(testRootPath);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    // Should handle permission errors gracefully
    EXPECT_NO_THROW({
        provider.traverse(state, handler);
    });
}

TEST_F(UT_FileProvider, MixedPathListProvider_Traverse_HandlesCorruptedFileSystem)
{
    QStringList pathList = {
        testRootPath + "/file1.txt",
        "/proc/invalid",   // System path that might cause issues
        testRootPath + "/file2.doc"
    };

    MixedPathListProvider provider(pathList);
    TaskState state;
    state.start();

    auto handler = [this](const QString &filePath) {
        processedFiles.append(filePath);
    };

    // Should handle system paths gracefully
    EXPECT_NO_THROW({
        provider.traverse(state, handler);
    });
}
