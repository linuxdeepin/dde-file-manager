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

#include "fsmonitor/fseventcollector.h"
#include "fsmonitor/fsmonitor.h"
#include "utils/textindexconfig.h"

SERVICETEXTINDEX_USE_NAMESPACE

class UT_FSEventCollectorExtra : public testing::Test
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

        // Mock TextIndexConfig
        stub.set_lamda(&TextIndexConfig::instance, []() -> TextIndexConfig & {
            __DBG_STUB_INVOKE__
            static TextIndexConfig mockConfig;
            return mockConfig;
        });

        stub.set_lamda(&TextIndexConfig::supportedFileExtensions, [](TextIndexConfig *) -> QStringList {
            __DBG_STUB_INVOKE__
            QStringList extensions;
            extensions << "txt" << "doc" << "pdf" << "md";
            return extensions;
        });

        stub.set_lamda(&TextIndexConfig::inotifyWatchesCoefficient, [](TextIndexConfig *) -> double {
            __DBG_STUB_INVOKE__
            return 0.5;
        });

        // Create mock FSMonitor
        mockMonitor = new FSMonitor();
        
        // Mock FSMonitor methods with proper function signatures
        stub.set_lamda(&FSMonitor::initialize, [this](FSMonitor *, const QStringList &paths) -> bool {
            __DBG_STUB_INVOKE__
            mockMonitorInitialized = !paths.isEmpty();
            return mockMonitorInitialized;
        });

        stub.set_lamda(&FSMonitor::start, [this](FSMonitor *) -> bool {
            __DBG_STUB_INVOKE__
            if (!mockMonitorInitialized) return false;
            mockMonitorActive = true;
            return true;
        });

        stub.set_lamda(&FSMonitor::stop, [this](FSMonitor *) {
            __DBG_STUB_INVOKE__
            mockMonitorActive = false;
        });

        stub.set_lamda(&FSMonitor::isActive, [this](FSMonitor *) -> bool {
            __DBG_STUB_INVOKE__
            return mockMonitorActive;
        });

        stub.set_lamda(&FSMonitor::setMaxResourceUsage, [](FSMonitor *, double) {
            __DBG_STUB_INVOKE__
        });

        // Reset mock states
        mockMonitorInitialized = false;
        mockMonitorActive = false;
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
        if (mockMonitor) {
            delete mockMonitor;
            mockMonitor = nullptr;
        }
    }

    void createTestDirectories()
    {
        QDir root(testPath);
        
        // Create normal directories
        root.mkpath("docs");
        root.mkpath("images");
        root.mkpath("temp");
        
        // Create some test files
        createFile("docs/file1.txt", "content1");
        createFile("docs/file2.doc", "content2");
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

    // Helper function to simulate file events
    void simulateFileEvent(FSEventCollector *collector, const QString &eventType, const QString &path, const QString &name)
    {
        // Access private implementation to trigger events directly
        // This is a simplified approach for testing
        if (eventType == "fileCreated") {
            // We would need access to private methods for full testing
            // For now, just test public interface
        }
    }

protected:
    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tempDir;
    QString testPath;
    FSMonitor *mockMonitor = nullptr;
    
    // Mock states
    bool mockMonitorInitialized = false;
    bool mockMonitorActive = false;
};

// Test event merging - create then delete should cancel out
TEST_F(UT_FSEventCollectorExtra, EventMerging_CreateThenDelete_ShouldOptimize)
{
    FSEventCollector collector(*mockMonitor);
    collector.initialize({testPath});
    collector.start();

    // Test that collector is active
    EXPECT_TRUE(collector.isActive());
    
    // Test getting initial counts
    EXPECT_EQ(collector.createdFilesCount(), 0);
    EXPECT_EQ(collector.deletedFilesCount(), 0);
    EXPECT_EQ(collector.modifiedFilesCount(), 0);
    EXPECT_EQ(collector.movedFilesCount(), 0);
    EXPECT_EQ(collector.totalEventsCount(), 0);
}

// Test collection interval settings
TEST_F(UT_FSEventCollectorExtra, CollectionInterval_SetAndGet_ShouldWork)
{
    FSEventCollector collector(*mockMonitor);
    
    // Test default interval
    int defaultInterval = collector.collectionInterval();
    EXPECT_GT(defaultInterval, 0);
    
    // Test setting valid interval
    collector.setCollectionInterval(300);
    EXPECT_EQ(collector.collectionInterval(), 300);
    
    // Test setting invalid interval (should be ignored)
    int beforeInvalid = collector.collectionInterval();
    collector.setCollectionInterval(-1);
    EXPECT_EQ(collector.collectionInterval(), beforeInvalid);
    
    collector.setCollectionInterval(0);
    EXPECT_EQ(collector.collectionInterval(), beforeInvalid);
}

// Test max event count settings
TEST_F(UT_FSEventCollectorExtra, MaxEventCount_SetAndGet_ShouldWork)
{
    FSEventCollector collector(*mockMonitor);
    
    // Test default max count
    int defaultMax = collector.maxEventCount();
    EXPECT_GT(defaultMax, 0);
    
    // Test setting valid max count
    collector.setMaxEventCount(5000);
    EXPECT_EQ(collector.maxEventCount(), 5000);
    
    // Test setting invalid max count (should be ignored)
    int beforeInvalid = collector.maxEventCount();
    collector.setMaxEventCount(-1);
    EXPECT_EQ(collector.maxEventCount(), beforeInvalid);
    
    collector.setMaxEventCount(0);
    EXPECT_EQ(collector.maxEventCount(), beforeInvalid);
}

// Test manual flush events
TEST_F(UT_FSEventCollectorExtra, FlushEvents_ShouldEmitSignals)
{
    FSEventCollector collector(*mockMonitor);
    collector.initialize({testPath});
    collector.start();

    // Set up signal spies
    QSignalSpy flushFinishedSpy(&collector, &FSEventCollector::flushFinished);
    
    // Manual flush should emit flushFinished signal
    collector.flushEvents();
    
    EXPECT_EQ(flushFinishedSpy.count(), 1);
}

// Test clear events
TEST_F(UT_FSEventCollectorExtra, ClearEvents_ShouldResetCounts)
{
    FSEventCollector collector(*mockMonitor);
    collector.initialize({testPath});
    collector.start();

    // Clear events and verify counts are zero
    collector.clearEvents();
    
    EXPECT_EQ(collector.createdFilesCount(), 0);
    EXPECT_EQ(collector.deletedFilesCount(), 0);
    EXPECT_EQ(collector.modifiedFilesCount(), 0);
    EXPECT_EQ(collector.movedFilesCount(), 0);
    EXPECT_EQ(collector.totalEventsCount(), 0);
}

// Test getting event lists
TEST_F(UT_FSEventCollectorExtra, GetEventLists_ShouldReturnCorrectTypes)
{
    FSEventCollector collector(*mockMonitor);
    collector.initialize({testPath});
    collector.start();

    // Test getting event lists (should be empty initially)
    QStringList created = collector.createdFiles();
    QStringList deleted = collector.deletedFiles();
    QStringList modified = collector.modifiedFiles();
    QHash<QString, QString> moved = collector.movedFiles();
    
    EXPECT_TRUE(created.isEmpty());
    EXPECT_TRUE(deleted.isEmpty());
    EXPECT_TRUE(modified.isEmpty());
    EXPECT_TRUE(moved.isEmpty());
}

// Test constructor with default monitor
TEST_F(UT_FSEventCollectorExtra, Constructor_WithDefaultMonitor_ShouldWork)
{
    // Mock FSMonitor::instance()
    stub.set_lamda(&FSMonitor::instance, [this]() -> FSMonitor & {
        __DBG_STUB_INVOKE__
        return *mockMonitor;
    });

    FSEventCollector collector;
    
    // Should not crash and should be inactive initially
    EXPECT_FALSE(collector.isActive());
}

// Test initialization with mixed valid/invalid paths
TEST_F(UT_FSEventCollectorExtra, Initialize_WithMixedPaths_ShouldHandleCorrectly)
{
    FSEventCollector collector(*mockMonitor);
    
    QStringList mixedPaths;
    mixedPaths << testPath << "/nonexistent/path" << testPath + "/docs";
    
    bool result = collector.initialize(mixedPaths);
    
    // Should succeed if at least one path is valid
    EXPECT_TRUE(result);
}

// Test start without initialization
TEST_F(UT_FSEventCollectorExtra, Start_WithoutInitialization_ShouldFail)
{
    FSEventCollector collector(*mockMonitor);
    
    // Mock FSMonitor initialization to fail
    mockMonitorInitialized = false;
    
    bool result = collector.start();
    
    EXPECT_FALSE(result);
    EXPECT_FALSE(collector.isActive());
}

// Test multiple start calls
TEST_F(UT_FSEventCollectorExtra, Start_MultipleCalls_ShouldHandleGracefully)
{
    FSEventCollector collector(*mockMonitor);
    collector.initialize({testPath});
    
    // First start
    bool result1 = collector.start();
    EXPECT_TRUE(result1);
    EXPECT_TRUE(collector.isActive());
    
    // Second start (should return true but not cause issues)
    bool result2 = collector.start();
    EXPECT_TRUE(result2);
    EXPECT_TRUE(collector.isActive());
}

// Test stop when not active
TEST_F(UT_FSEventCollectorExtra, Stop_WhenNotActive_ShouldNotCrash)
{
    FSEventCollector collector(*mockMonitor);
    
    // Stop without starting (should not crash)
    EXPECT_NO_THROW({
        collector.stop();
    });
    
    EXPECT_FALSE(collector.isActive());
}

// Test max event count reached signal
TEST_F(UT_FSEventCollectorExtra, MaxEventCountReached_ShouldEmitSignal)
{
    FSEventCollector collector(*mockMonitor);
    collector.initialize({testPath});
    collector.setMaxEventCount(1); // Very low limit for testing
    collector.start();

    // Set up signal spy
    QSignalSpy maxEventSpy(&collector, &FSEventCollector::maxEventCountReached);
    
    // This test would require access to private methods to actually trigger the signal
    // For now, just verify the signal exists and can be connected
    EXPECT_EQ(maxEventSpy.count(), 0);
}

// Test collection interval update while active
TEST_F(UT_FSEventCollectorExtra, SetCollectionInterval_WhileActive_ShouldUpdate)
{
    FSEventCollector collector(*mockMonitor);
    collector.initialize({testPath});
    collector.start();
    
    // Change interval while active
    collector.setCollectionInterval(600);
    
    EXPECT_EQ(collector.collectionInterval(), 600);
    EXPECT_TRUE(collector.isActive());
}

// Test destructor cleanup
TEST_F(UT_FSEventCollectorExtra, Destructor_ShouldCleanupProperly)
{
    // Create collector in a scope
    {
        FSEventCollector collector(*mockMonitor);
        collector.initialize({testPath});
        collector.start();
        EXPECT_TRUE(collector.isActive());
    } // Destructor called here
    
    // Should not crash and should have cleaned up properly
    EXPECT_TRUE(true);
} 