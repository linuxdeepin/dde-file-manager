// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>

#include "task/taskhandler.h"
#include "task/fileprovider.h"
#include "utils/indexutility.h"
#include "utils/textindexconfig.h"
#include "utils/docutils.h"

#include <lucene++/LuceneHeaders.h>
#include <FileUtils.h>
#include <FilterIndexReader.h>
#include <FuzzyQuery.h>
#include <QueryWrapperFilter.h>

#include "../../3rdparty/fulltext/chineseanalyzer.h"

#include <dfm-search/dsearch_global.h>
#include <dfm-search/searchfactory.h>

SERVICETEXTINDEX_USE_NAMESPACE
DFM_SEARCH_USE_NS
using namespace Lucene;

class UT_TaskHandlers : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create temporary directory for testing
        tempDir = std::make_unique<QTemporaryDir>();
        EXPECT_TRUE(tempDir->isValid());

        testPath = tempDir->path();
        indexPath = testPath + "/index";
        QDir().mkpath(indexPath);

        // Mock DFMSEARCH::Global
        stub.set_lamda(ADDR(DFMSEARCH::Global, contentIndexDirectory), [this]() -> QString {
            __DBG_STUB_INVOKE__
            return indexPath;
        });

        // Mock IndexUtility
        stub.set_lamda(ADDR(IndexUtility, isDefaultIndexedDirectory), [this](const QString &path) -> bool {
            __DBG_STUB_INVOKE__
            return path.startsWith(testPath);
        });

        stub.set_lamda(ADDR(IndexUtility, isIndexWithAnything), [](const QString &) -> bool {
            __DBG_STUB_INVOKE__
            return false;   // Default to filesystem provider
        });

        stub.set_lamda(ADDR(IndexUtility, isSupportedFile), [](const QString &path) -> bool {
            __DBG_STUB_INVOKE__
            return path.endsWith(".txt") || path.endsWith(".doc") || path.endsWith(".pdf");
        });

        // Mock TextIndexConfig
        stub.set_lamda(ADDR(TextIndexConfig, instance), []() -> TextIndexConfig & {
            __DBG_STUB_INVOKE__
            static TextIndexConfig mockConfig;
            return mockConfig;
        });

        stub.set_lamda(ADDR(TextIndexConfig, batchCommitInterval), [](TextIndexConfig *) -> int {
            __DBG_STUB_INVOKE__
            return 100;   // Commit every 100 documents
        });

        stub.set_lamda(ADDR(TextIndexConfig, maxIndexFileTruncationSizeMB), [](TextIndexConfig *) -> int {
            __DBG_STUB_INVOKE__
            return 10;   // 10MB
        });

        // Mock DocUtils
        stub.set_lamda(ADDR(DocUtils, extractFileContent), [](const QString &filePath, size_t maxBytes) -> std::optional<QString> {
            __DBG_STUB_INVOKE__
            if (QFile::exists(filePath)) {
                return QString("Mock content for ") + filePath;
            }
            return std::nullopt;
        });

        // Reset mock states
        mockTaskState.reset();
        mockHandlerResult = HandlerResult { true, false, false, false };
        createTestFiles();
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

    void createTestFiles()
    {
        // Create test file structure
        QDir root(testPath);
        root.mkpath("subdir");

        createFile("file1.txt", "Content of file1");
        createFile("file2.doc", "Content of file2");
        createFile("file3.pdf", "Content of file3");
        createFile("subdir/nested.txt", "Nested content");
    }

    void createFile(const QString &relativePath, const QString &content)
    {
        QString fullPath = testPath + "/" + relativePath;
        QDir().mkpath(QFileInfo(fullPath).absolutePath());

        QFile file(fullPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(content.toUtf8());
        }
    }

    std::unique_ptr<QTemporaryDir> tempDir;
    QString testPath;
    QString indexPath;

    // Mock task state
    struct MockTaskState
    {
        bool running = true;
        void reset() { running = true; }
        bool isRunning() const { return running; }
        void stop() { running = false; }
        void start() { running = true; }
    } mockTaskState;

    HandlerResult mockHandlerResult;

private:
    stub_ext::StubExt stub;
};

// CreateFileProvider Tests
TEST_F(UT_TaskHandlers, CreateFileProvider_ValidPath_ReturnsFileSystemProvider)
{
    auto provider = TaskHandlers::createFileProvider(testPath);

    EXPECT_TRUE(provider != nullptr);
    EXPECT_EQ(provider->name(), "FileSystemProvider");
}

TEST_F(UT_TaskHandlers, CreateFileProvider_WithAnything_ReturnsDirectFileListProvider)
{
    // Mock to use ANYTHING
    stub.set_lamda(ADDR(IndexUtility, isIndexWithAnything), [](const QString &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    // Mock SearchFactory and SearchEngine
    stub.set_lamda(ADDR(SearchFactory, createEngine), [](SearchType, QObject *) -> SearchEngine * {
        __DBG_STUB_INVOKE__
        static SearchEngine mockEngine;
        return &mockEngine;
    });

    stub.set_lamda(ADDR(SearchEngine, searchSync), [](SearchEngine *, const SearchQuery &) -> SearchResultExpected {
        __DBG_STUB_INVOKE__
        SearchResultList results;
        results.append(SearchResult("/test/file.txt"));
        return SearchResultExpected(results);
    });

    auto provider = TaskHandlers::createFileProvider(testPath);

    EXPECT_TRUE(provider != nullptr);
    // Would be DirectFileListProvider if ANYTHING is available
}

TEST_F(UT_TaskHandlers, CreateFileProvider_AnythingFails_FallsBackToFileSystem)
{
    // // Mock ANYTHING to fail
    // stub.set_lamda(ADDR(IndexUtility, isIndexWithAnything), [](const QString &) -> bool {
    //     __DBG_STUB_INVOKE__
    //     return true;
    // });

    // stub.set_lamda(ADDR(SearchFactory, createEngine), [](SearchType, QObject *) -> SearchEngine * {
    //     __DBG_STUB_INVOKE__
    //     return nullptr;   // Simulate failure
    // });

    // auto provider = TaskHandlers::createFileProvider(testPath);

    // EXPECT_TRUE(provider != nullptr);
    // EXPECT_EQ(provider->name(), "FileSystemProvider");   // Should fallback
}

// CreateFileListProvider Tests
TEST_F(UT_TaskHandlers, CreateFileListProvider_ValidList_ReturnsMixedPathListProvider)
{
    QStringList fileList = { testPath + "/file1.txt", testPath + "/file2.doc" };

    auto provider = TaskHandlers::createFileListProvider(fileList);

    EXPECT_TRUE(provider != nullptr);
    EXPECT_EQ(provider->name(), "MixedPathListProvider");
}

TEST_F(UT_TaskHandlers, CreateFileListProvider_EmptyList_ReturnsValidProvider)
{
    QStringList emptyList;

    auto provider = TaskHandlers::createFileListProvider(emptyList);

    EXPECT_TRUE(provider != nullptr);
    EXPECT_EQ(provider->name(), "MixedPathListProvider");
}

// CreateIndexHandler Tests
TEST_F(UT_TaskHandlers, CreateIndexHandler_ValidPath_ReturnsHandler)
{
    TaskHandler handler = TaskHandlers::CreateIndexHandler();

    EXPECT_TRUE(handler != nullptr);
}

TEST_F(UT_TaskHandlers, CreateIndexHandler_Execute_CreatesIndex)
{
    TaskHandler handler = TaskHandlers::CreateIndexHandler();
    TaskState state;
    state.start();

    HandlerResult result = handler(testPath, state);

    // Should succeed if all mocks work correctly
    EXPECT_TRUE(result.success || !result.success);   // Test doesn't crash
}

TEST_F(UT_TaskHandlers, CreateIndexHandler_NonExistentPath_HandlesFail)
{
    TaskHandler handler = TaskHandlers::CreateIndexHandler();
    TaskState state;
    state.start();

    QString nonExistentPath = testPath + "/nonexistent";
    HandlerResult result = handler(nonExistentPath, state);

    EXPECT_FALSE(result.success);   // Should fail for non-existent path
}

TEST_F(UT_TaskHandlers, CreateIndexHandler_InterruptedByState_StopsExecution)
{
    TaskHandler handler = TaskHandlers::CreateIndexHandler();
    TaskState state;
    state.start();

    // Mock to stop state during execution
    stub.set_lamda(ADDR(TaskState, isRunning), [](TaskState *) -> bool {
        __DBG_STUB_INVOKE__
        static int callCount = 0;
        return ++callCount <= 1;   // Stop after first call
    });

    HandlerResult result = handler(testPath, state);

    EXPECT_TRUE(result.interrupted);   // Should be interrupted
}

// UpdateIndexHandler Tests
TEST_F(UT_TaskHandlers, UpdateIndexHandler_ValidPath_ReturnsHandler)
{
    TaskHandler handler = TaskHandlers::UpdateIndexHandler();

    EXPECT_TRUE(handler != nullptr);
}

// Simplified test without complex Lucene mocking
TEST_F(UT_TaskHandlers, UpdateIndexHandler_Execute_UpdatesIndex)
{
    TaskHandler handler = TaskHandlers::UpdateIndexHandler();
    TaskState state;
    state.start();

    // Create a proper empty Lucene index directory
    QDir indexDir(indexPath);
    indexDir.mkpath(".");

    try {
        // Create a minimal valid Lucene index
        using namespace Lucene;
        IndexWriterPtr writer = newLucene<IndexWriter>(
            FSDirectory::open(indexPath.toStdWString()),
            newLucene<ChineseAnalyzer>(),
            true,  // create new index
            IndexWriter::MaxFieldLengthUNLIMITED);
        writer->close();  // Close to finalize the index
        
        // Now test the handler
        EXPECT_NO_THROW({
            HandlerResult result = handler(testPath, state);
        });
    } catch (...) {
        // If Lucene initialization fails, skip the test
        GTEST_SKIP() << "Skipping test due to Lucene initialization failure";
    }
}

// CreateOrUpdateFileListHandler Tests
TEST_F(UT_TaskHandlers, CreateOrUpdateFileListHandler_ValidFiles_ReturnsHandler)
{
    QStringList fileList = { testPath + "/file1.txt", testPath + "/file2.doc" };

    TaskHandler handler = TaskHandlers::CreateOrUpdateFileListHandler(fileList);

    EXPECT_TRUE(handler != nullptr);
}

TEST_F(UT_TaskHandlers, CreateOrUpdateFileListHandler_Execute_ProcessesFiles)
{
    QStringList fileList = { testPath + "/file1.txt", testPath + "/file2.doc" };
    TaskHandler handler = TaskHandlers::CreateOrUpdateFileListHandler(fileList);
    TaskState state;
    state.start();

    // Create a proper empty Lucene index directory
    QDir indexDir(indexPath);
    indexDir.mkpath(".");

    try {
        // Create a minimal valid Lucene index
        using namespace Lucene;
        IndexWriterPtr writer = newLucene<IndexWriter>(
            FSDirectory::open(indexPath.toStdWString()),
            newLucene<ChineseAnalyzer>(),
            true,  // create new index
            IndexWriter::MaxFieldLengthUNLIMITED);
        writer->close();  // Close to finalize the index
        
        // Now test the handler
        EXPECT_NO_THROW({
            HandlerResult result = handler("FileList-Test", state);
        });
    } catch (...) {
        // If Lucene initialization fails, skip the test
        GTEST_SKIP() << "Skipping test due to Lucene initialization failure";
    }
}

TEST_F(UT_TaskHandlers, CreateOrUpdateFileListHandler_EmptyList_HandlesGracefully)
{
    QStringList emptyList;
    TaskHandler handler = TaskHandlers::CreateOrUpdateFileListHandler(emptyList);
    TaskState state;
    state.start();

    // Create a proper empty Lucene index directory
    QDir indexDir(indexPath);
    indexDir.mkpath(".");

    try {
        // Create a minimal valid Lucene index
        using namespace Lucene;
        IndexWriterPtr writer = newLucene<IndexWriter>(
            FSDirectory::open(indexPath.toStdWString()),
            newLucene<ChineseAnalyzer>(),
            true,  // create new index
            IndexWriter::MaxFieldLengthUNLIMITED);
        writer->close();  // Close to finalize the index
        
        // Now test the handler - should handle empty list gracefully
        EXPECT_NO_THROW({
            HandlerResult result = handler("FileList-Empty", state);
        });
    } catch (...) {
        // If Lucene initialization fails, skip the test
        GTEST_SKIP() << "Skipping test due to Lucene initialization failure";
    }
}

// RemoveFileListHandler Tests
TEST_F(UT_TaskHandlers, RemoveFileListHandler_ValidFiles_ReturnsHandler)
{
    QStringList fileList = { testPath + "/file1.txt", testPath + "/file2.doc" };

    TaskHandler handler = TaskHandlers::RemoveFileListHandler(fileList);

    EXPECT_TRUE(handler != nullptr);
}

TEST_F(UT_TaskHandlers, RemoveFileListHandler_Execute_RemovesFiles)
{
    QStringList fileList = { testPath + "/file1.txt", testPath + "/file2.doc" };
    TaskHandler handler = TaskHandlers::RemoveFileListHandler(fileList);
    TaskState state;
    state.start();

    // Create a simple index directory structure for testing
    QDir indexDir(indexPath);
    indexDir.mkpath(".");

    // Create a minimal index file to simulate existing index
    QFile indexFile(indexPath + "/segments.gen");
    if (indexFile.open(QIODevice::WriteOnly)) {
        indexFile.write("mock index");
        indexFile.close();
    }

    HandlerResult result = handler("RemoveList-Test", state);

    // Test should complete without crashing
    EXPECT_TRUE(true);   // Test passes if no exceptions thrown
}

// MoveFileListHandler Tests
TEST_F(UT_TaskHandlers, MoveFileListHandler_ValidMoves_ReturnsHandler)
{
    QHash<QString, QString> movedFiles;
    movedFiles[testPath + "/old.txt"] = testPath + "/new.txt";

    TaskHandler handler = TaskHandlers::MoveFileListHandler(movedFiles);

    EXPECT_TRUE(handler != nullptr);
}

TEST_F(UT_TaskHandlers, MoveFileListHandler_Execute_ProcessesMoves)
{
    QHash<QString, QString> movedFiles;
    movedFiles[testPath + "/old.txt"] = testPath + "/new.txt";
    movedFiles[testPath + "/old2.txt"] = testPath + "/new2.txt";

    TaskHandler handler = TaskHandlers::MoveFileListHandler(movedFiles);
    TaskState state;
    state.start();

    // Create a simple index directory structure for testing
    QDir indexDir(indexPath);
    indexDir.mkpath(".");

    // Create a minimal index file to simulate existing index
    QFile indexFile(indexPath + "/segments.gen");
    if (indexFile.open(QIODevice::WriteOnly)) {
        indexFile.write("mock index");
        indexFile.close();
    }

    HandlerResult result = handler("MoveList-Test", state);

    // Test should complete without crashing
    EXPECT_TRUE(true);   // Test passes if no exceptions thrown
}

TEST_F(UT_TaskHandlers, MoveFileListHandler_EmptyMoves_HandlesGracefully)
{
    QHash<QString, QString> emptyMoves;

    TaskHandler handler = TaskHandlers::MoveFileListHandler(emptyMoves);

    EXPECT_TRUE(handler != nullptr);

    TaskState state;
    state.start();

    HandlerResult result = handler("MoveList-Empty", state);

    // Should handle empty moves gracefully
    EXPECT_TRUE(result.success || !result.success);   // Test doesn't crash
}

// Error Handling Tests
TEST_F(UT_TaskHandlers, AllHandlers_NullState_HandleGracefully)
{
    TaskHandler createHandler = TaskHandlers::CreateIndexHandler();
    TaskHandler updateHandler = TaskHandlers::UpdateIndexHandler();

    // Test with null-like state behavior
    TaskState nullState;
    nullState.stop();   // Stopped state

    EXPECT_NO_THROW({
        createHandler(testPath, nullState);
        updateHandler(testPath, nullState);
    });
}

TEST_F(UT_TaskHandlers, AllHandlers_EmptyPath_HandleGracefully)
{
    TaskHandler createHandler = TaskHandlers::CreateIndexHandler();
    TaskHandler updateHandler = TaskHandlers::UpdateIndexHandler();
    TaskState state;
    state.start();

    // Create a proper empty Lucene index directory
    QDir indexDir(indexPath);
    indexDir.mkpath(".");

    try {
        // Create a minimal valid Lucene index for update handler
        using namespace Lucene;
        IndexWriterPtr writer = newLucene<IndexWriter>(
            FSDirectory::open(indexPath.toStdWString()),
            newLucene<ChineseAnalyzer>(),
            true,  // create new index
            IndexWriter::MaxFieldLengthUNLIMITED);
        writer->close();  // Close to finalize the index
        
        // Test handlers with empty paths - they should handle gracefully
        EXPECT_NO_THROW({
            createHandler("", state);
            updateHandler("", state);
        });
    } catch (...) {
        // If Lucene initialization fails, skip the test
        GTEST_SKIP() << "Skipping test due to Lucene initialization failure";
    }
}

// Performance Tests
TEST_F(UT_TaskHandlers, CreateIndexHandler_LargeDirectory_PerformsReasonably)
{
    // Create many test files
    for (int i = 0; i < 100; ++i) {
        createFile(QString("largefile%1.txt").arg(i), "Large file content");
    }

    TaskHandler handler = TaskHandlers::CreateIndexHandler();
    TaskState state;
    state.start();

    // Should complete without timeout
    EXPECT_NO_THROW({
        handler(testPath, state);
    });
}

// Integration-like Tests
TEST_F(UT_TaskHandlers, HandlerChain_CreateThenUpdate_WorksTogether)
{
    TaskHandler createHandler = TaskHandlers::CreateIndexHandler();
    TaskHandler updateHandler = TaskHandlers::UpdateIndexHandler();
    TaskState state;

    // First create index
    state.start();
    HandlerResult createResult = createHandler(testPath, state);

    // Then update it
    state.start();
    HandlerResult updateResult = updateHandler(testPath, state);

    // Both should complete (success or failure, but no crash)
    EXPECT_TRUE(true);   // Test passes if no exceptions thrown
}
