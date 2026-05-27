// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSignalSpy>
#include <QTimer>
#include <QEventLoop>
#include <QThread>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QPromise>

#include "fsmonitor/fsmonitorworker.h"
#include "fsmonitor/fsmonitor.h"
#include "utils/textindexconfig.h"

#include <dfm-base/base/application/application.h>
#include <dfm-search/searchengine.h>
#include <dfm-search/searchfactory.h>
#include <dfm-search/filenamesearchapi.h>

SERVICETEXTINDEX_USE_NAMESPACE
DFM_SEARCH_USE_NS

class UT_FSMonitorWorker : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create temporary directory for testing
        tempDir.reset(new QTemporaryDir());
        EXPECT_TRUE(tempDir->isValid());
        testPath = tempDir->path();

        // Create test directory structure
        createTestDirectories();

        // Mock search engine functionality
        stub.set_lamda(&DFMSEARCH::Global::fileNameIndexStatus, []() -> std::optional<QString> {
            __DBG_STUB_INVOKE__
            return std::optional<QString>("monitoring");
        });

        // For complex template functions, we'll skip detailed mocking and focus on core functionality
        // stub.set_lamda(&SearchFactory::createEngine, [this](SearchType type, QObject *parent) -> SearchEngine * {
        //     __DBG_STUB_INVOKE__
        //     mockSearchEngineCreated = true;
        //     return nullptr; // Simplified for compilation
        // });

        // Mock QFutureWatcher
        stub.set_lamda(ADDR(QFutureWatcher<QStringList>, setFuture), [this](QFutureWatcher<QStringList> *, const QFuture<QStringList> &) {
            __DBG_STUB_INVOKE__
            mockFutureWatcherSet = true;
        });

        stub.set_lamda(ADDR(QFutureWatcher<QStringList>, result), [this](QFutureWatcher<QStringList> *) -> QStringList {
            __DBG_STUB_INVOKE__
            return mockFutureResult;
        });

        // Mock QtConcurrent::run to avoid actual async execution
        // We'll use a simplified approach - mock the search components instead

        // Mock QThread::msleep
        stub.set_lamda(ADDR(QThread, msleep), [](unsigned long) {
            __DBG_STUB_INVOKE__
        });

        // Skip complex mocking for QtConcurrent operations
        // Focus on testing functionality that doesn't require async operations

        // Reset mock states
        mockSearchEngineCreated = false;
        mockSearchShouldFail = false;
        mockSearchResults.clear();
        mockFutureWatcherSet = false;
        mockFutureResult.clear();
        mockConcurrentRunCalled = false;
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
        if (mockSearchEngine) {
            delete mockSearchEngine;
            mockSearchEngine = nullptr;
        }
    }

    void createTestDirectories()
    {
        QDir root(testPath);

        // Create normal directories
        root.mkpath("docs");
        root.mkpath("docs/subdir1");
        root.mkpath("docs/subdir2");
        root.mkpath("images");
        root.mkpath("images/photos");
        root.mkpath("temp");
        root.mkpath(".hidden");
        root.mkpath(".git");

        // Create some test files
        createFile("docs/file1.txt", "content1");
        createFile("docs/subdir1/file2.txt", "content2");
        createFile("images/photo.jpg", "binary content");
        createFile("temp/temp.txt", "temp content");
    }

    void createFile(const QString &relativePath, const QString &content)
    {
        QString fullPath = QDir(testPath).absoluteFilePath(relativePath);
        QFileInfo fileInfo(fullPath);
        QDir().mkpath(fileInfo.absolutePath());

        QFile file(fullPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << content;
        }
    }

protected:
    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tempDir;
    QString testPath;

    // Mock objects and states
    SearchEngine *mockSearchEngine = nullptr;
    bool mockSearchEngineCreated = false;
    bool mockSearchShouldFail = false;
    QStringList mockSearchResults;
    bool mockFutureWatcherSet = false;
    QStringList mockFutureResult;
    bool mockConcurrentRunCalled = false;
};
// Test FSMonitorWorker construction and destruction
TEST_F(UT_FSMonitorWorker, Construction_ShouldInitializeCorrectly)
{
    FSMonitorWorker worker;

    // Worker should be constructed successfully
    EXPECT_FALSE(worker.isFastScanInProgress());
}

// Test directory processing with valid directory
TEST_F(UT_FSMonitorWorker, ProcessDirectory_WithValidDirectory_ShouldEmitSignals)
{
    FSMonitorWorker worker;

    // Set up signal spies
    QSignalSpy directoryToWatchSpy(&worker, &FSMonitorWorker::directoryToWatch);
    QSignalSpy subdirectoriesFoundSpy(&worker, &FSMonitorWorker::subdirectoriesFound);

    // Process the test directory
    worker.processDirectory(testPath);

    // Should emit directoryToWatch signal for the directory itself
    EXPECT_EQ(directoryToWatchSpy.count(), 1);
    EXPECT_EQ(directoryToWatchSpy.at(0).at(0).toString(), testPath);

    // Should emit subdirectoriesFound signal with subdirectories
    EXPECT_EQ(subdirectoriesFoundSpy.count(), 1);
    QStringList foundSubdirs = subdirectoriesFoundSpy.at(0).at(0).toStringList();
    EXPECT_GT(foundSubdirs.size(), 0);
}

// Test directory processing with empty path
TEST_F(UT_FSMonitorWorker, ProcessDirectory_WithEmptyPath_ShouldNotEmitSignals)
{
    FSMonitorWorker worker;

    // Set up signal spies
    QSignalSpy directoryToWatchSpy(&worker, &FSMonitorWorker::directoryToWatch);
    QSignalSpy subdirectoriesFoundSpy(&worker, &FSMonitorWorker::subdirectoriesFound);

    // Process empty path
    worker.processDirectory("");

    // Should not emit any signals
    EXPECT_EQ(directoryToWatchSpy.count(), 0);
    EXPECT_EQ(subdirectoriesFoundSpy.count(), 0);
}

// Test directory processing with non-existent directory
TEST_F(UT_FSMonitorWorker, ProcessDirectory_WithNonExistentDirectory_ShouldNotEmitSignals)
{
    FSMonitorWorker worker;

    // Set up signal spies
    QSignalSpy directoryToWatchSpy(&worker, &FSMonitorWorker::directoryToWatch);
    QSignalSpy subdirectoriesFoundSpy(&worker, &FSMonitorWorker::subdirectoriesFound);

    // Process non-existent directory
    worker.processDirectory("/non/existent/path");

    // Should not emit any signals
    EXPECT_EQ(directoryToWatchSpy.count(), 0);
    EXPECT_EQ(subdirectoriesFoundSpy.count(), 0);
}

// Test directory processing with file path
TEST_F(UT_FSMonitorWorker, ProcessDirectory_WithFilePath_ShouldNotEmitSignals)
{
    FSMonitorWorker worker;

    // Set up signal spies
    QSignalSpy directoryToWatchSpy(&worker, &FSMonitorWorker::directoryToWatch);
    QSignalSpy subdirectoriesFoundSpy(&worker, &FSMonitorWorker::subdirectoriesFound);

    // Process file path instead of directory
    QString filePath = QDir(testPath).absoluteFilePath("docs/file1.txt");
    worker.processDirectory(filePath);

    // Should not emit any signals
    EXPECT_EQ(directoryToWatchSpy.count(), 0);
    EXPECT_EQ(subdirectoriesFoundSpy.count(), 0);
}

// Test exclusion checker functionality
TEST_F(UT_FSMonitorWorker, ExclusionChecker_WhenSet_ShouldFilterDirectories)
{
    FSMonitorWorker worker;

    // Set exclusion checker to exclude .git directories
    worker.setExclusionChecker([](const QString &path) {
        return path.contains(".git");
    });

    // Set up signal spies
    QSignalSpy directoryToWatchSpy(&worker, &FSMonitorWorker::directoryToWatch);
    QSignalSpy subdirectoriesFoundSpy(&worker, &FSMonitorWorker::subdirectoriesFound);

    // Process .git directory (should be excluded)
    QString gitPath = QDir(testPath).absoluteFilePath(".git");
    worker.processDirectory(gitPath);

    // Should not emit any signals due to exclusion
    EXPECT_EQ(directoryToWatchSpy.count(), 0);
    EXPECT_EQ(subdirectoriesFoundSpy.count(), 0);
}

// Test fast directory scan state management without calling async operations
TEST_F(UT_FSMonitorWorker, FastDirectoryScan_StateManagement_ShouldWork)
{
    FSMonitorWorker worker;

    // Initially, fast scan should not be in progress
    EXPECT_FALSE(worker.isFastScanInProgress());

    // Test that we can set max results without issues
    worker.setMaxFastScanResults(1000);
    worker.setMaxFastScanResults(50000);

    // Should still not be in progress
    EXPECT_FALSE(worker.isFastScanInProgress());
}

// Test setMaxFastScanResults with valid values
TEST_F(UT_FSMonitorWorker, SetMaxFastScanResults_WithValidValues_ShouldWork)
{
    FSMonitorWorker worker;

    // Test setting various valid values
    worker.setMaxFastScanResults(100);
    worker.setMaxFastScanResults(1000);
    worker.setMaxFastScanResults(50000);
    worker.setMaxFastScanResults(65536);

    // Should not crash or cause issues
    EXPECT_FALSE(worker.isFastScanInProgress());
}

// Test setMaxFastScanResults with invalid values
TEST_F(UT_FSMonitorWorker, SetMaxFastScanResults_WithInvalidValues_ShouldBeIgnored)
{
    FSMonitorWorker worker;

    // Test setting invalid values (should be ignored)
    worker.setMaxFastScanResults(0);
    worker.setMaxFastScanResults(-100);
    worker.setMaxFastScanResults(-1);

    // Should not crash or cause issues
    EXPECT_FALSE(worker.isFastScanInProgress());
}

// Test exclusion checker with various path patterns
TEST_F(UT_FSMonitorWorker, ExclusionChecker_WithVariousPatterns_ShouldWork)
{
    FSMonitorWorker worker;

    // Test setting different exclusion patterns
    worker.setExclusionChecker([](const QString &path) {
        return path.contains(".git") || path.contains("node_modules");
    });

    // Set up signal spies
    QSignalSpy directoryToWatchSpy(&worker, &FSMonitorWorker::directoryToWatch);
    QSignalSpy subdirectoriesFoundSpy(&worker, &FSMonitorWorker::subdirectoriesFound);

    // Process normal directory (should work)
    worker.processDirectory(testPath);
    EXPECT_EQ(directoryToWatchSpy.count(), 1);

    // Reset spies
    directoryToWatchSpy.clear();
    subdirectoriesFoundSpy.clear();

    // Process excluded directory (should be ignored)
    QString gitPath = QDir(testPath).absoluteFilePath(".git");
    worker.processDirectory(gitPath);
    EXPECT_EQ(directoryToWatchSpy.count(), 0);
}

// Test directory processing with subdirectories containing different types
TEST_F(UT_FSMonitorWorker, ProcessDirectory_WithMixedContent_ShouldProcessDirectoriesOnly)
{
    FSMonitorWorker worker;

    // Create additional test structure
    QDir root(testPath);
    root.mkpath("mixed/subdir1");
    root.mkpath("mixed/subdir2");
    createFile("mixed/file1.txt", "content");
    createFile("mixed/file2.txt", "content");

    // Set up signal spies
    QSignalSpy directoryToWatchSpy(&worker, &FSMonitorWorker::directoryToWatch);
    QSignalSpy subdirectoriesFoundSpy(&worker, &FSMonitorWorker::subdirectoriesFound);

    // Process the mixed directory
    QString mixedPath = QDir(testPath).absoluteFilePath("mixed");
    worker.processDirectory(mixedPath);

    // Should emit directoryToWatch signal for the directory itself
    EXPECT_EQ(directoryToWatchSpy.count(), 1);
    EXPECT_EQ(directoryToWatchSpy.at(0).at(0).toString(), mixedPath);

    // Should emit subdirectoriesFound signal with only subdirectories (not files)
    EXPECT_EQ(subdirectoriesFoundSpy.count(), 1);
    QStringList foundSubdirs = subdirectoriesFoundSpy.at(0).at(0).toStringList();
    EXPECT_EQ(foundSubdirs.size(), 2); // Should find subdir1 and subdir2
}

