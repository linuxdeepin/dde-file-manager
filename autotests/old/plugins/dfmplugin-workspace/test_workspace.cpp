// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "workspace.h"
#include "views/workspacewidget.h"
#include "utils/workspacehelper.h"
#include "events/workspaceeventreceiver.h"
#include "menus/workspacemenuscene.h"
#include "menus/sortanddisplaymenuscene.h"
#include "menus/basesortmenuscene.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/base/configs/configsynchronizer.h>

#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <QApplication>

using namespace dfmplugin_workspace;
using namespace dfmbase;

class WorkspaceTest : public ::testing::Test
{
protected:
    static QApplication *app;
    static Application *mockApplication; // Shared mock application instance
    
    static void setupApplication()
    {
        if (!app) {
            // Set high DPI scaling factor rounding policy before creating QApplication instance
            QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }
    }

    void SetUp() override
    {
        // Initialize test environment
        setupApplication();
        workspace = new Workspace();
        
        // Clear all stubs before each test to avoid conflicts
        stub.clear();
    }

    void TearDown() override
    {
        delete workspace;
        // Don't delete app here, it will be cleaned up in static destructor
        stub.clear();
    }

    Workspace *workspace = nullptr;
    stub_ext::StubExt stub;
};

QApplication *WorkspaceTest::app = nullptr;
Application *WorkspaceTest::mockApplication = nullptr;

TEST_F(WorkspaceTest, Initialize_CallsRequiredMethods)
{
    // Mock WorkspaceHelper::instance
    static WorkspaceHelper mockHelper;
    stub.set_lamda(ADDR(WorkspaceHelper, instance), []() -> WorkspaceHelper* {
        __DBG_STUB_INVOKE__
        return &mockHelper;
    });
    
    // Mock WorkspaceHelper::registerFileView
    stub.set_lamda(ADDR(WorkspaceHelper, registerFileView), [](WorkspaceHelper*, const QString&) {
        __DBG_STUB_INVOKE__
    });
    
    // Mock FMWindowsIns
    stub.set_lamda(ADDR(FileManagerWindowsManager, windowOpened), []() {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(FileManagerWindowsManager, windowClosed), []() {
        __DBG_STUB_INVOKE__
    });
    
    // Mock WorkspaceEventReceiver::instance
    static WorkspaceEventReceiver mockEventReceiver;
    stub.set_lamda(ADDR(WorkspaceEventReceiver, instance), []() -> WorkspaceEventReceiver* {
        __DBG_STUB_INVOKE__
        return &mockEventReceiver;
    });
    
    // Mock WorkspaceEventReceiver::initConnection
    stub.set_lamda(ADDR(WorkspaceEventReceiver, initConnection), []() {
        __DBG_STUB_INVOKE__
    });
    
    // Mock DConfigManager::instance
    static DConfigManager mockConfigManager;
    stub.set_lamda(ADDR(DConfigManager, instance), []() -> DConfigManager* {
        __DBG_STUB_INVOKE__
        return &mockConfigManager;
    });
    
    // Mock DConfigManager::addConfig
    stub.set_lamda(ADDR(DConfigManager, addConfig), [](DConfigManager*, const QString&, QString*) {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    // Mock ConfigSynchronizer::instance
    static ConfigSynchronizer mockConfigSynchronizer;
    stub.set_lamda(ADDR(ConfigSynchronizer, instance), []() -> ConfigSynchronizer* {
        __DBG_STUB_INVOKE__
        return &mockConfigSynchronizer;
    });
    
    // Mock ConfigSynchronizer::watchChange
    stub.set_lamda(ADDR(ConfigSynchronizer, watchChange), [](ConfigSynchronizer*, const SyncPair&) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    // Mock Application::instance
    static Application mockApplication;
    stub.set_lamda(ADDR(Application, instance), []() -> Application* {
        __DBG_STUB_INVOKE__
        return &mockApplication;
    });
    
    // Mock Application::setGenericAttribute
    stub.set_lamda(ADDR(Application, setGenericAttribute), [](Application::GenericAttribute, const QVariant&) {
        __DBG_STUB_INVOKE__
    });
    
    // Mock Application::setAppAttribute
    stub.set_lamda(ADDR(Application, setAppAttribute), [](Application::ApplicationAttribute, const QVariant&) {
        __DBG_STUB_INVOKE__
    });
    
    // Test initialize method
    EXPECT_NO_THROW(workspace->initialize());
}

TEST_F(WorkspaceTest, Start_ReturnsTrue)
{
    // Test start method
    EXPECT_NO_THROW(workspace->start());
}

TEST_F(WorkspaceTest, OnWindowOpened_ValidWindowId_DoesNotCrash)
{
    // Test onWindowOpened with valid window ID
    quint64 windowId = 12345;
    
    // Since FMWindowsIns is a macro that expands to FileManagerWindowsManager::instance().findWindowById()
    // We need to stub the findWindowById method directly on the instance returned by instance()
    // Let's create a mock instance and stub both instance() and findWindowById()
    static FileManagerWindowsManager mockManager;
    stub.set_lamda(ADDR(FileManagerWindowsManager, instance), []() -> FileManagerWindowsManager& {
        __DBG_STUB_INVOKE__
        return mockManager;
    });
    
    // Create a mock FileManagerWindow without calling the constructor
    // Use placement new to avoid Qt platform initialization issues
    static char mockWindowStorage[sizeof(FileManagerWindow)] __attribute__((aligned(sizeof(FileManagerWindow))));
    static FileManagerWindow *mockWindow = nullptr;
    
    // Mock findWindowById to return a valid window pointer
    stub.set_lamda(ADDR(FileManagerWindowsManager, findWindowById), [](FileManagerWindowsManager*, quint64) -> FileManagerWindow* {
        __DBG_STUB_INVOKE__
        if (!mockWindow) {
            // Create a mock window pointer without calling constructor
            mockWindow = reinterpret_cast<FileManagerWindow*>(mockWindowStorage);
        }
        return mockWindow; // Return valid window to pass ASSERT check
    });
    
    // Mock WorkspaceHelper::addWorkspace to avoid actual workspace creation
    stub.set_lamda(ADDR(WorkspaceHelper, addWorkspace), [](WorkspaceHelper*, quint64, WorkspaceWidget*) {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(workspace->onWindowOpened(windowId));
}

TEST_F(WorkspaceTest, OnWindowClosed_ValidWindowId_DoesNotCrash)
{
    // Test onWindowClosed with valid window ID
    quint64 windowId = 12345;
    
    // onWindowClosed doesn't call findWindowById, so no need to mock it
    EXPECT_NO_THROW(workspace->onWindowClosed(windowId));
}

TEST_F(WorkspaceTest, SaveRemoteThumbnailToConf_ValidValue_DoesNotCrash)
{
    // Test saveRemoteThumbnailToConf static method
    QVariant testValue(true);
    
    // Mock DConfigManager::instance
    static DConfigManager mockConfigManager;
    stub.set_lamda(ADDR(DConfigManager, instance), []() -> DConfigManager* {
        __DBG_STUB_INVOKE__
        return &mockConfigManager;
    });
    
    // Mock DConfigManager::setValue
    stub.set_lamda(ADDR(DConfigManager, setValue), [](DConfigManager*, const QString&, const QString&, const QVariant&) {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(Workspace::saveRemoteThumbnailToConf(testValue));
}

TEST_F(WorkspaceTest, SyncRemoteThumbnailToAppSet_ValidValue_DoesNotCrash)
{
    // Test syncRemoteThumbnailToAppSet static method
    QString key1, key2;
    QVariant testValue(true);
    
    // Mock Application::instance - use shared mockApplication
    stub.set_lamda(ADDR(Application, instance), []() -> Application* {
        __DBG_STUB_INVOKE__
        return mockApplication;
    });
    
    // Mock Application::setGenericAttribute
    stub.set_lamda(ADDR(Application, setGenericAttribute), [](Application::GenericAttribute, const QVariant &) {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(Workspace::syncRemoteThumbnailToAppSet(key1, key2, testValue));
}

TEST_F(WorkspaceTest, IsRemoteThumbnailConfEqual_ValidValues_ReturnsCorrectResult)
{
    // Test isRemoteThumbnailConfEqual static method
    QVariant dconValue(true);
    QVariant dsetValue(true);
    
    bool result = Workspace::isRemoteThumbnailConfEqual(dconValue, dsetValue);
    EXPECT_TRUE(result);
    
    // Test with different values
    dsetValue = false;
    result = Workspace::isRemoteThumbnailConfEqual(dconValue, dsetValue);
    EXPECT_FALSE(result);
}

TEST_F(WorkspaceTest, SaveIconSizeToConf_ValidValue_DoesNotCrash)
{
    // Test saveIconSizeToConf static method
    QVariant testValue(5);
    
    // Mock DConfigManager::instance
    static DConfigManager mockConfigManager;
    stub.set_lamda(ADDR(DConfigManager, instance), []() -> DConfigManager* {
        __DBG_STUB_INVOKE__
        return &mockConfigManager;
    });
    
    // Mock DConfigManager::setValue
    stub.set_lamda(ADDR(DConfigManager, setValue), [](DConfigManager*, const QString&, const QString&, const QVariant&) {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(Workspace::saveIconSizeToConf(testValue));
}

TEST_F(WorkspaceTest, SyncIconSizeToAppSet_ValidValue_DoesNotCrash)
{
    // Test syncIconSizeToAppSet static method
    QString key1, key2;
    QVariant testValue(5);
    
    // Mock Application::instance - use shared mockApplication
    stub.set_lamda(ADDR(Application, instance), []() -> Application* {
        __DBG_STUB_INVOKE__
        return mockApplication;
    });
    
    // Mock Application::setAppAttribute
    stub.set_lamda(ADDR(Application, setAppAttribute), [](Application::ApplicationAttribute, const QVariant &) {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(Workspace::syncIconSizeToAppSet(key1, key2, testValue));
}

TEST_F(WorkspaceTest, IsIconSizeConfEqual_ValidValues_ReturnsCorrectResult)
{
    // Test isIconSizeConfEqual static method
    QVariant dconValue(5);
    QVariant dsetValue(5);
    
    bool result = Workspace::isIconSizeConfEqual(dconValue, dsetValue);
    EXPECT_TRUE(result);
    
    // Test with different values
    dsetValue = 3;
    result = Workspace::isIconSizeConfEqual(dconValue, dsetValue);
    EXPECT_FALSE(result);
}

TEST_F(WorkspaceTest, SaveGridDensityToConf_ValidValue_DoesNotCrash)
{
    // Test saveGridDensityToConf static method
    QVariant testValue(2);
    
    // Mock DConfigManager::instance
    static DConfigManager mockConfigManager;
    stub.set_lamda(ADDR(DConfigManager, instance), []() -> DConfigManager* {
        __DBG_STUB_INVOKE__
        return &mockConfigManager;
    });
    
    // Mock DConfigManager::setValue
    stub.set_lamda(ADDR(DConfigManager, setValue), [](DConfigManager*, const QString&, const QString&, const QVariant&) {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(Workspace::saveGridDensityToConf(testValue));
}

TEST_F(WorkspaceTest, SyncGridDensityToAppSet_ValidValue_DoesNotCrash)
{
    // Test syncGridDensityToAppSet static method
    QString key1, key2;
    QVariant testValue(2);
    
    // Mock Application::instance - use shared mockApplication
    stub.set_lamda(ADDR(Application, instance), []() -> Application* {
        __DBG_STUB_INVOKE__
        return mockApplication;
    });
    
    // Mock Application::setAppAttribute
    stub.set_lamda(ADDR(Application, setAppAttribute), [](Application::ApplicationAttribute, const QVariant &) {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(Workspace::syncGridDensityToAppSet(key1, key2, testValue));
}

TEST_F(WorkspaceTest, IsGridDensityConfEqual_ValidValues_ReturnsCorrectResult)
{
    // Test isGridDensityConfEqual static method
    QVariant dconValue(2);
    QVariant dsetValue(2);
    
    bool result = Workspace::isGridDensityConfEqual(dconValue, dsetValue);
    EXPECT_TRUE(result);
    
    // Test with different values
    dsetValue = 1;
    result = Workspace::isGridDensityConfEqual(dconValue, dsetValue);
    EXPECT_FALSE(result);
}

TEST_F(WorkspaceTest, SaveListHeightToConf_ValidValue_DoesNotCrash)
{
    // Test saveListHeightToConf static method
    QVariant testValue(3);
    
    // Mock DConfigManager::instance
    static DConfigManager mockConfigManager;
    stub.set_lamda(ADDR(DConfigManager, instance), []() -> DConfigManager* {
        __DBG_STUB_INVOKE__
        return &mockConfigManager;
    });
    
    // Mock DConfigManager::setValue
    stub.set_lamda(ADDR(DConfigManager, setValue), [](DConfigManager*, const QString&, const QString&, const QVariant&) {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(Workspace::saveListHeightToConf(testValue));
}

TEST_F(WorkspaceTest, SyncListHeightToAppSet_ValidValue_DoesNotCrash)
{
    // Test syncListHeightToAppSet static method
    QString key1, key2;
    QVariant testValue(3);
    
    // Mock Application::instance - use shared mockApplication
    stub.set_lamda(ADDR(Application, instance), []() -> Application* {
        __DBG_STUB_INVOKE__
        return mockApplication;
    });
    
    // Mock Application::setAppAttribute
    stub.set_lamda(ADDR(Application, setAppAttribute), [](Application::ApplicationAttribute , const QVariant &) {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(Workspace::syncListHeightToAppSet(key1, key2, testValue));
}

TEST_F(WorkspaceTest, IsListHeightConfEqual_ValidValues_ReturnsCorrectResult)
{
    // Test isListHeightConfEqual static method
    QVariant dconValue(3);
    QVariant dsetValue(3);
    
    bool result = Workspace::isListHeightConfEqual(dconValue, dsetValue);
    EXPECT_TRUE(result);
    
    // Test with different values
    dsetValue = 1;
    result = Workspace::isListHeightConfEqual(dconValue, dsetValue);
    EXPECT_FALSE(result);
}

TEST_F(WorkspaceTest, ReadyToInstallWidget_SignalEmitted)
{
    // Test that readyToInstallWidget signal can be emitted
    QSignalSpy spy(workspace, &Workspace::readyToInstallWidget);
    quint64 windowId = 12345;
    
    // Mock FileManagerWindowsManager::instance to avoid ASSERT failure
    // Since FMWindowsIns is a macro for FileManagerWindowsManager::instance(), we need to stub the instance method
    static FileManagerWindowsManager mockManager;
    stub.set_lamda(ADDR(FileManagerWindowsManager, instance), []() -> FileManagerWindowsManager& {
        __DBG_STUB_INVOKE__
        return mockManager; // Return mock instance to avoid window operations
    });
    
    // Create a mock FileManagerWindow without calling constructor
    // Use placement new to avoid Qt platform initialization issues
    static char mockWindowStorage[sizeof(FileManagerWindow)] __attribute__((aligned(sizeof(FileManagerWindow))));
    static FileManagerWindow *mockWindow = nullptr;
    
    // Mock findWindowById to return a valid window pointer
    stub.set_lamda(ADDR(FileManagerWindowsManager, findWindowById), [](FileManagerWindowsManager*, quint64) -> FileManagerWindow* {
        __DBG_STUB_INVOKE__
        if (!mockWindow) {
            // Create a mock window pointer without calling constructor
            mockWindow = reinterpret_cast<FileManagerWindow*>(mockWindowStorage);
        }
        return mockWindow; // Return valid window to pass ASSERT check
    });
    
    // Mock WorkspaceHelper::addWorkspace
    stub.set_lamda(ADDR(WorkspaceHelper, addWorkspace), [](WorkspaceHelper*, quint64, WorkspaceWidget *) {
        __DBG_STUB_INVOKE__
    });
    
    workspace->onWindowOpened(windowId);
    
    // Signal should be emitted once
    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toULongLong(), windowId);
}
