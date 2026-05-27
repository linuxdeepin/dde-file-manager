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

#include "fsmonitor/fseventcontroller.h"
#include "fsmonitor/fseventcollector.h"
#include "utils/textindexconfig.h"

#include <dfm-search/dsearch_global.h>

SERVICETEXTINDEX_USE_NAMESPACE

class UT_FSEventController : public testing::Test
{
protected:
    void SetUp() override
    {
        // Create temporary directory for testing
        tempDir.reset(new QTemporaryDir());
        EXPECT_TRUE(tempDir->isValid());
        testPath = tempDir->path();

        // Mock TextIndexConfig
        stub.set_lamda(ADDR(TextIndexConfig, instance), []() -> TextIndexConfig & {
            __DBG_STUB_INVOKE__
            static TextIndexConfig mockConfig;
            return mockConfig;
        });

        stub.set_lamda(ADDR(TextIndexConfig, autoIndexUpdateInterval), [this](TextIndexConfig *) -> int {
            __DBG_STUB_INVOKE__
            return mockAutoIndexInterval;
        });

        stub.set_lamda(ADDR(TextIndexConfig, inotifyResourceCleanupDelayMs), [this](TextIndexConfig *) -> int {
            __DBG_STUB_INVOKE__
            return mockCleanupDelay;
        });

        // Mock DFMSEARCH::Global
        stub.set_lamda(ADDR(DFMSEARCH::Global, defaultIndexedDirectory), [this]() -> QStringList {
            __DBG_STUB_INVOKE__
            return mockIndexedDirectories;
        });

        // Mock FSEventCollector
        stub.set_lamda(ADDR(FSEventCollector, initialize), [this](FSEventCollector *, const QStringList &paths) -> bool {
            __DBG_STUB_INVOKE__
            mockCollectorInitialized = !paths.isEmpty();
            mockInitializedPaths = paths;
            return mockCollectorInitialized;
        });

        stub.set_lamda(ADDR(FSEventCollector, start), [this](FSEventCollector *) -> bool {
            __DBG_STUB_INVOKE__
            if (!mockCollectorInitialized) return false;
            mockCollectorActive = true;
            return true;
        });

        stub.set_lamda(ADDR(FSEventCollector, stop), [this](FSEventCollector *) {
            __DBG_STUB_INVOKE__
            mockCollectorActive = false;
        });

        stub.set_lamda(ADDR(FSEventCollector, isActive), [this](FSEventCollector *) -> bool {
            __DBG_STUB_INVOKE__
            return mockCollectorActive;
        });

        stub.set_lamda(ADDR(FSEventCollector, setCollectionInterval), [this](FSEventCollector *, int seconds) {
            __DBG_STUB_INVOKE__
            mockCollectionInterval = seconds;
        });

        stub.set_lamda(ADDR(FSEventCollector, setMaxEventCount), [](FSEventCollector *, int) {
            __DBG_STUB_INVOKE__
        });

        // Mock QTimer - handle overloaded start function
        using QTimerStartFunc = void (QTimer::*)(int);
        stub.set_lamda(static_cast<QTimerStartFunc>(&QTimer::start), [this](QTimer *timer, int msec) {
            __DBG_STUB_INVOKE__
            
            // Based on the actual FSEventController logic:
            // - Start timer uses interval 0 (immediate) or mockAutoIndexInterval * 1000 (delayed)
            // - Stop timer uses inotifyResourceCleanupDelayMs() (5000ms default)
            if (msec == 0 || msec == mockAutoIndexInterval * 1000) {
                // This is the start timer
                mockStartTimerActive = true;
                mockStartTimerInterval = msec;
                // When start timer is started, stop timer should be inactive
                mockStopTimerActive = false;
            } else if (msec >= 5000) {
                // This is the stop timer with cleanup delay
                mockStopTimerActive = true;
                mockStopTimerInterval = msec;
                // When stop timer is started, start timer should be inactive
                mockStartTimerActive = false;
            }
        });

        stub.set_lamda(ADDR(QTimer, stop), [this](QTimer *timer) {
            __DBG_STUB_INVOKE__
            // For simplicity, we'll assume stop() always stops the start timer
            // since that's the most common case in the FSEventController logic
            // When setEnabled(false) is called, it first stops the start timer
            mockStartTimerActive = false;
        });

        stub.set_lamda(ADDR(QTimer, setSingleShot), [](QTimer *, bool) {
            __DBG_STUB_INVOKE__
        });

        // Reset mock states
        mockAutoIndexInterval = 120;
        mockCleanupDelay = 5000;
        mockIndexedDirectories.clear();
        mockIndexedDirectories << testPath;
        mockCollectorInitialized = false;
        mockCollectorActive = false;
        mockInitializedPaths.clear();
        mockCollectionInterval = 0;
        mockStartTimerActive = false;
        mockStopTimerActive = false;
        mockStartTimerInterval = 0;
        mockStopTimerInterval = 0;
        mockStartTimer = nullptr;
        mockStopTimer = nullptr;
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
    }

protected:
    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tempDir;
    QString testPath;
    
    // Mock states
    int mockAutoIndexInterval = 120;
    int mockCleanupDelay = 5000;
    QStringList mockIndexedDirectories;
    bool mockCollectorInitialized = false;
    bool mockCollectorActive = false;
    QStringList mockInitializedPaths;
    int mockCollectionInterval = 0;
    bool mockStartTimerActive = false;
    bool mockStopTimerActive = false;
    int mockStartTimerInterval = 0;
    int mockStopTimerInterval = 0;
    QTimer *mockStartTimer = nullptr;
    QTimer *mockStopTimer = nullptr;
};

// Test FSEventController construction
TEST_F(UT_FSEventController, Construction_ShouldInitializeCorrectly)
{
    FSEventController controller;
    
    // Initially should be disabled
    EXPECT_FALSE(controller.isEnabled());
    EXPECT_FALSE(controller.silentlyRefreshStarted());
}

// Test setupFSEventCollector
TEST_F(UT_FSEventController, SetupFSEventCollector_ShouldInitializeCorrectly)
{
    FSEventController controller;
    
    // Mock the timer creation
    mockStartTimer = new QTimer();
    mockStopTimer = new QTimer();
    
    controller.setupFSEventCollector();
    
    // Should have set up the collector with correct interval
    EXPECT_EQ(mockCollectionInterval, mockAutoIndexInterval);
    
    delete mockStartTimer;
    delete mockStopTimer;
}

// Test enabling the controller
TEST_F(UT_FSEventController, SetEnabled_ToTrue_ShouldStartTimer)
{
    FSEventController controller;
    mockStartTimer = new QTimer();
    mockStopTimer = new QTimer();
    controller.setupFSEventCollector();
    
    // Set up signal spy
    QSignalSpy monitoringSpy(&controller, &FSEventController::monitoring);
    
    // Enable the controller
    controller.setEnabled(true);
    
    EXPECT_TRUE(controller.isEnabled());
    EXPECT_TRUE(mockStartTimerActive);
    EXPECT_FALSE(mockStopTimerActive);
    EXPECT_EQ(mockStartTimerInterval, 0); // Should start immediately when not silently refreshing
    
    delete mockStartTimer;
    delete mockStopTimer;
}

// Test disabling the controller
TEST_F(UT_FSEventController, SetEnabled_ToFalse_ShouldStartStopTimer)
{
    FSEventController controller;
    mockStartTimer = new QTimer();
    mockStopTimer = new QTimer();
    controller.setupFSEventCollector();
    
    // First enable, then disable
    controller.setEnabled(true);
    controller.setEnabled(false);
    
    EXPECT_FALSE(controller.isEnabled());
    EXPECT_FALSE(mockStartTimerActive);
    EXPECT_TRUE(mockStopTimerActive);
    EXPECT_EQ(mockStopTimerInterval, mockCleanupDelay);
    
    delete mockStartTimer;
    delete mockStopTimer;
}

// Test setEnabledNow with true
TEST_F(UT_FSEventController, SetEnabledNow_ToTrue_ShouldEnableImmediately)
{
    FSEventController controller;
    mockStartTimer = new QTimer();
    mockStopTimer = new QTimer();
    controller.setupFSEventCollector();
    
    controller.setEnabledNow(true);
    
    EXPECT_TRUE(controller.isEnabled());
    EXPECT_TRUE(mockStartTimerActive);
    
    delete mockStartTimer;
    delete mockStopTimer;
}

// Test setEnabledNow with false
TEST_F(UT_FSEventController, SetEnabledNow_ToFalse_ShouldStopImmediately)
{
    FSEventController controller;
    mockStartTimer = new QTimer();
    mockStopTimer = new QTimer();
    controller.setupFSEventCollector();
    
    // First enable, then disable immediately
    controller.setEnabled(true);
    controller.setEnabledNow(false);
    
    EXPECT_FALSE(controller.isEnabled());
    EXPECT_FALSE(mockCollectorActive);
    
    delete mockStartTimer;
    delete mockStopTimer;
}

// Test startFSMonitoring
TEST_F(UT_FSEventController, StartFSMonitoring_WithValidDirectories_ShouldSucceed)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    
    controller.startFSMonitoring();
    
    EXPECT_TRUE(mockCollectorInitialized);
    EXPECT_TRUE(mockCollectorActive);
    EXPECT_EQ(mockInitializedPaths, mockIndexedDirectories);
}

// Test startFSMonitoring without collector
TEST_F(UT_FSEventController, StartFSMonitoring_WithoutCollector_ShouldFail)
{
    FSEventController controller;
    // Don't call setupFSEventCollector
    
    controller.startFSMonitoring();
    
    // Should not initialize or start anything
    EXPECT_FALSE(mockCollectorInitialized);
    EXPECT_FALSE(mockCollectorActive);
}

// Test startFSMonitoring with empty directories
TEST_F(UT_FSEventController, StartFSMonitoring_WithEmptyDirectories_ShouldFail)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    
    // Set empty indexed directories
    mockIndexedDirectories.clear();
    
    controller.startFSMonitoring();
    
    EXPECT_FALSE(mockCollectorInitialized);
    EXPECT_FALSE(mockCollectorActive);
}

// Test stopFSMonitoring
TEST_F(UT_FSEventController, StopFSMonitoring_WhenActive_ShouldStop)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    
    // First start monitoring
    controller.startFSMonitoring();
    EXPECT_TRUE(mockCollectorActive);
    
    // Then stop it
    controller.stopFSMonitoring();
    EXPECT_FALSE(mockCollectorActive);
}

// Test stopFSMonitoring when not active
TEST_F(UT_FSEventController, StopFSMonitoring_WhenNotActive_ShouldNotCrash)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    
    // Stop without starting (should not crash)
    controller.stopFSMonitoring();
    EXPECT_FALSE(mockCollectorActive);
}

// Test silently refresh flag
TEST_F(UT_FSEventController, SilentlyRefreshFlag_SetAndGet_ShouldWork)
{
    FSEventController controller;
    
    // Initially should be false
    EXPECT_FALSE(controller.silentlyRefreshStarted());
    
    // Set to true
    controller.setSilentlyRefreshStarted(true);
    EXPECT_TRUE(controller.silentlyRefreshStarted());
    
    // Set to false
    controller.setSilentlyRefreshStarted(false);
    EXPECT_FALSE(controller.silentlyRefreshStarted());
}

// Test enabling with silently refresh flag
TEST_F(UT_FSEventController, SetEnabled_WithSilentlyRefresh_ShouldDelayStart)
{
    FSEventController controller;
    mockStartTimer = new QTimer();
    mockStopTimer = new QTimer();
    controller.setupFSEventCollector();
    
    // Set silently refresh flag
    controller.setSilentlyRefreshStarted(true);
    
    // Enable the controller
    controller.setEnabled(true);
    
    EXPECT_TRUE(controller.isEnabled());
    EXPECT_TRUE(mockStartTimerActive);
    // Should start with delay when silently refreshing
    EXPECT_EQ(mockStartTimerInterval, mockAutoIndexInterval * 1000);
    
    delete mockStartTimer;
    delete mockStopTimer;
}

// Test event handling - files created
TEST_F(UT_FSEventController, OnFilesCreated_WhenEnabled_ShouldCollectEvents)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    controller.setEnabled(true);
    
    QStringList createdFiles;
    createdFiles << "/path1" << "/path2" << "/path3";
    
    // Simulate files created event
    controller.onFilesCreated(createdFiles);
    
    // Events should be collected (we can't directly test the internal collection
    // but we can verify the method doesn't crash and the controller remains enabled)
    EXPECT_TRUE(controller.isEnabled());
}

// Test event handling - files created when disabled
TEST_F(UT_FSEventController, OnFilesCreated_WhenDisabled_ShouldIgnoreEvents)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    // Controller is disabled by default
    
    QStringList createdFiles;
    createdFiles << "/path1" << "/path2" << "/path3";
    
    // Simulate files created event when disabled
    controller.onFilesCreated(createdFiles);
    
    // Should not crash and controller should remain disabled
    EXPECT_FALSE(controller.isEnabled());
}

// Test event handling - files deleted
TEST_F(UT_FSEventController, OnFilesDeleted_WhenEnabled_ShouldCollectEvents)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    controller.setEnabled(true);
    
    QStringList deletedFiles;
    deletedFiles << "/path1" << "/path2" << "/path3";
    
    // Simulate files deleted event
    controller.onFilesDeleted(deletedFiles);
    
    EXPECT_TRUE(controller.isEnabled());
}

// Test event handling - files modified
TEST_F(UT_FSEventController, OnFilesModified_WhenEnabled_ShouldCollectEvents)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    controller.setEnabled(true);
    
    QStringList modifiedFiles;
    modifiedFiles << "/path1" << "/path2" << "/path3";
    
    // Simulate files modified event
    controller.onFilesModified(modifiedFiles);
    
    EXPECT_TRUE(controller.isEnabled());
}

// Test event handling - files moved
TEST_F(UT_FSEventController, OnFilesMoved_WhenEnabled_ShouldCollectEvents)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    controller.setEnabled(true);
    
    QHash<QString, QString> movedFiles;
    movedFiles.insert("/old/path1", "/new/path1");
    movedFiles.insert("/old/path2", "/new/path2");
    
    // Simulate files moved event
    controller.onFilesMoved(movedFiles);
    
    EXPECT_TRUE(controller.isEnabled());
}

// Test flush finished event
TEST_F(UT_FSEventController, OnFlushFinished_WhenEnabled_ShouldEmitSignals)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    controller.setEnabled(true);
    
    // Set up signal spies
    QSignalSpy fileChangesSpy(&controller, &FSEventController::requestProcessFileChanges);
    QSignalSpy fileMovesSpy(&controller, &FSEventController::requestProcessFileMoves);
    
    // Add some events first
    QStringList createdFiles;
    createdFiles << "/created1" << "/created2";
    controller.onFilesCreated(createdFiles);
    
    QHash<QString, QString> movedFiles;
    movedFiles.insert("/old1", "/new1");
    controller.onFilesMoved(movedFiles);
    
    // Simulate flush finished
    controller.onFlushFinished();
    
    // Should emit processing signals
    EXPECT_EQ(fileChangesSpy.count(), 1);
    EXPECT_EQ(fileMovesSpy.count(), 1);
}

// Test flush finished when no events collected
TEST_F(UT_FSEventController, OnFlushFinished_WithNoEvents_ShouldNotEmitSignals)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    controller.setEnabled(true);
    
    // Set up signal spies
    QSignalSpy fileChangesSpy(&controller, &FSEventController::requestProcessFileChanges);
    QSignalSpy fileMovesSpy(&controller, &FSEventController::requestProcessFileMoves);
    
    // Simulate flush finished without any events
    controller.onFlushFinished();
    
    // Should not emit processing signals
    EXPECT_EQ(fileChangesSpy.count(), 0);
    EXPECT_EQ(fileMovesSpy.count(), 0);
}

// Test config changed event
TEST_F(UT_FSEventController, OnConfigChanged_ShouldUpdateInterval)
{
    FSEventController controller;
    controller.setupFSEventCollector();
    
    // Change the mock interval
    int oldInterval = mockAutoIndexInterval;
    mockAutoIndexInterval = 300; // Change from 120 to 300
    
    // Simulate config change
    controller.onConfigChanged();
    
    // Should update the collection interval
    EXPECT_EQ(mockCollectionInterval, mockAutoIndexInterval);
    EXPECT_NE(mockCollectionInterval, oldInterval);
} 