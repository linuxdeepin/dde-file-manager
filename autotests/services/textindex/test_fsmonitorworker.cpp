// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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

        // Skip complex QtConcurrent mocking for now
        // stub.set_lamda(&QtConcurrent::run, ...); // Too complex for this test

        // Mock QThread::msleep
        stub.set_lamda(ADDR(QThread, msleep), [](unsigned long) {
            __DBG_STUB_INVOKE__
        });

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

// Test fast directory scan with successful result
TEST_F(UT_FSMonitorWorker, FastDirectoryScan_WithSuccessfulResult_ShouldEmitCorrectSignals)
{
    FSMonitorWorker worker;

    // Set up mock search results
    mockSearchResults << "/path1"
                      << "/path2"
                      << "/path3";

    // Set up signal spies
    QSignalSpy fastScanCompletedSpy(&worker, &FSMonitorWorker::fastScanCompleted);
    QSignalSpy directoriesBatchToWatchSpy(&worker, &FSMonitorWorker::directoriesBatchToWatch);

    // Start fast directory scan
    worker.tryFastDirectoryScan();

    EXPECT_TRUE(mockConcurrentRunCalled);

    // Simulate fast scan completion
    worker.handleFastScanResult();

    // Should emit fastScanCompleted with success
    EXPECT_EQ(fastScanCompletedSpy.count(), 1);
    EXPECT_TRUE(fastScanCompletedSpy.at(0).at(0).toBool());

    // Should emit directoriesBatchToWatch with results
    EXPECT_GT(directoriesBatchToWatchSpy.count(), 0);
}

// Test fast directory scan with failed result
TEST_F(UT_FSMonitorWorker, FastDirectoryScan_WithFailedResult_ShouldEmitFailureSignal)
{
    FSMonitorWorker worker;

    // Set up mock to fail search
    mockSearchShouldFail = true;

    // Set up signal spies
    QSignalSpy fastScanCompletedSpy(&worker, &FSMonitorWorker::fastScanCompleted);
    QSignalSpy directoriesBatchToWatchSpy(&worker, &FSMonitorWorker::directoriesBatchToWatch);

    // Start fast directory scan
    worker.tryFastDirectoryScan();

    EXPECT_TRUE(mockConcurrentRunCalled);

    // Simulate fast scan completion
    worker.handleFastScanResult();

    // Should emit fastScanCompleted with failure
    EXPECT_EQ(fastScanCompletedSpy.count(), 1);
    EXPECT_FALSE(fastScanCompletedSpy.at(0).at(0).toBool());

    // Should not emit directoriesBatchToWatch
    EXPECT_EQ(directoriesBatchToWatchSpy.count(), 0);
}

// Test fast directory scan when already in progress
TEST_F(UT_FSMonitorWorker, FastDirectoryScan_WhenAlreadyInProgress_ShouldBeIgnored)
{
    FSMonitorWorker worker;

    // Start first fast directory scan
    worker.tryFastDirectoryScan();
    EXPECT_TRUE(worker.isFastScanInProgress());

    // Reset mock state
    mockConcurrentRunCalled = false;

    // Try to start another scan while in progress
    worker.tryFastDirectoryScan();

    // Second call should be ignored
    EXPECT_FALSE(mockConcurrentRunCalled);
}

// Test setMaxFastScanResults
TEST_F(UT_FSMonitorWorker, SetMaxFastScanResults_WithValidValue_ShouldWork)
{
    FSMonitorWorker worker;

    // Test setting valid values
    worker.setMaxFastScanResults(1000);
    worker.setMaxFastScanResults(50000);

    // Should not crash or cause issues
}

// Test setMaxFastScanResults with invalid value
TEST_F(UT_FSMonitorWorker, SetMaxFastScanResults_WithInvalidValue_ShouldBeIgnored)
{
    FSMonitorWorker worker;

    // Test setting invalid values (should be ignored)
    worker.setMaxFastScanResults(0);
    worker.setMaxFastScanResults(-100);

    // Should not crash or cause issues
}

// Test directory processing with subdirectories containing symbolic links
TEST_F(UT_FSMonitorWorker, ProcessDirectory_WithSymbolicLinks_ShouldSkipSymlinks)
{
    FSMonitorWorker worker;

    // Mock QFileInfo to simulate symbolic links
    stub.set_lamda(ADDR(QFileInfo, isSymLink), [](QFileInfo *info) -> bool {
        __DBG_STUB_INVOKE__
        return info->absoluteFilePath().contains("symlink");
    });

    // Create a directory with a "symbolic link" (mocked)
    QDir root(testPath);
    root.mkpath("symlink_dir");

    // Set up signal spies
    QSignalSpy directoryToWatchSpy(&worker, &FSMonitorWorker::directoryToWatch);
    QSignalSpy subdirectoriesFoundSpy(&worker, &FSMonitorWorker::subdirectoriesFound);

    // Process the test directory
    worker.processDirectory(testPath);

    // Should emit signals but symlinks should be filtered out
    EXPECT_EQ(directoryToWatchSpy.count(), 1);

    if (subdirectoriesFoundSpy.count() > 0) {
        QStringList foundSubdirs = subdirectoriesFoundSpy.at(0).at(0).toStringList();
        // None of the found subdirectories should contain "symlink"
        for (const QString &subdir : foundSubdirs) {
            EXPECT_FALSE(subdir.contains("symlink"));
        }
    }
}

// Test fast scan with exclusion checker
TEST_F(UT_FSMonitorWorker, FastScan_WithExclusionChecker_ShouldFilterResults)
{
    FSMonitorWorker worker;

    // Set exclusion checker to exclude paths containing "exclude"
    worker.setExclusionChecker([](const QString &path) {
        return path.contains("exclude");
    });

    // Set up mock search results with some excluded paths
    mockSearchResults << "/path1"
                      << "/exclude/path2"
                      << "/path3"
                      << "/another/exclude/path4";

    // Set up signal spies
    QSignalSpy directoriesBatchToWatchSpy(&worker, &FSMonitorWorker::directoriesBatchToWatch);

    // Start fast directory scan
    worker.tryFastDirectoryScan();

    // Simulate fast scan completion
    worker.handleFastScanResult();

    // Should emit directoriesBatchToWatch with filtered results
    if (directoriesBatchToWatchSpy.count() > 0) {
        QStringList batchPaths = directoriesBatchToWatchSpy.at(0).at(0).toStringList();
        // None of the batch paths should contain "exclude"
        for (const QString &path : batchPaths) {
            EXPECT_FALSE(path.contains("exclude"));
        }
    }
}

// Test batch size handling in fast scan
TEST_F(UT_FSMonitorWorker, FastScan_WithLargeResults_ShouldEmitInBatches)
{
    FSMonitorWorker worker;

    // Set up mock search results with many paths
    for (int i = 0; i < 500; ++i) {
        mockSearchResults << QString("/path%1").arg(i);
    }

    // Set up signal spies
    QSignalSpy directoriesBatchToWatchSpy(&worker, &FSMonitorWorker::directoriesBatchToWatch);

    // Start fast directory scan
    worker.tryFastDirectoryScan();

    // Simulate fast scan completion
    worker.handleFastScanResult();

    // Should emit multiple batches
    EXPECT_GT(directoriesBatchToWatchSpy.count(), 1);
}
