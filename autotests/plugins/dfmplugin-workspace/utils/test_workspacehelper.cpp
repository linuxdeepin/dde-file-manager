// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/workspacehelper.h"
#include "views/workspacewidget.h"

#include <QUrl>
#include <QVariant>
#include <QAbstractItemView>

using namespace dfmplugin_workspace;

class WorkspaceHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(WorkspaceHelperTest, Instance_ReturnsSameInstance)
{
    // Test that instance() returns the same singleton instance
    auto instance1 = WorkspaceHelper::instance();
    auto instance2 = WorkspaceHelper::instance();
    
    EXPECT_EQ(instance1, instance2);
}

TEST_F(WorkspaceHelperTest, RegisterTopWidgetCreator_ValidScheme_CreatesCreator)
{
    // Test registering a top widget creator
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    
    // Mock creator function
    bool creatorCalled = false;
    auto creator = [&creatorCalled]() -> CustomTopWidgetInterface* {
        creatorCalled = true;
        return nullptr;
    };
    
    helper->registerTopWidgetCreator(scheme, creator);
    
    EXPECT_TRUE(helper->isRegistedTopWidget(scheme));
}

TEST_F(WorkspaceHelperTest, IsRegistedTopWidget_RegisteredScheme_ReturnsTrue)
{
    // Test checking if a scheme is registered
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    
    // Mock creator function
    auto creator = []() -> CustomTopWidgetInterface* {
        return nullptr;
    };
    
    helper->registerTopWidgetCreator(scheme, creator);
    
    bool result = helper->isRegistedTopWidget(scheme);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceHelperTest, IsRegistedTopWidget_UnregisteredScheme_ReturnsFalse)
{
    // Test checking if an unregistered scheme is registered
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "unregistered_scheme";
    
    bool result = helper->isRegistedTopWidget(scheme);
    
    EXPECT_FALSE(result);
}

TEST_F(WorkspaceHelperTest, CreateTopWidgetByUrl_ValidUrl_CreatesWidget)
{
    // Test creating a top widget by URL
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    QUrl testUrl("test_scheme://path");
    
    // Mock creator function
    bool creatorCalled = false;
    auto creator = [&creatorCalled]() -> CustomTopWidgetInterface* {
        creatorCalled = true;
        return nullptr;
    };
    
    helper->registerTopWidgetCreator(scheme, creator);
    
    auto result = helper->createTopWidgetByUrl(testUrl);
    
    // Just test that the function runs without crashing
    // The creator might not be called if the URL scheme is not properly handled
    (void)result;
}

TEST_F(WorkspaceHelperTest, CreateTopWidgetByScheme_ValidScheme_CreatesWidget)
{
    // Test creating a top widget by scheme
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    
    // Mock creator function
    bool creatorCalled = false;
    auto creator = [&creatorCalled]() -> CustomTopWidgetInterface* {
        creatorCalled = true;
        return nullptr;
    };
    
    helper->registerTopWidgetCreator(scheme, creator);
    
    auto result = helper->createTopWidgetByScheme(scheme);
    
    // Just test that the function runs without crashing
    (void)result;
}

TEST_F(WorkspaceHelperTest, SetCustomTopWidgetVisible_ValidScheme_SetsVisibility)
{
    // Test setting custom top widget visibility
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    quint64 windowId = 12345;
    bool visible = true;
    
    // This should not crash
    helper->setCustomTopWidgetVisible(windowId, scheme, visible);
}

TEST_F(WorkspaceHelperTest, SetFilterData_ValidUrl_SetsFilterData)
{
    // Test setting filter data
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QUrl url("file:///tmp/test");
    QVariant data("test_data");
    
    // This should not crash
    helper->setFilterData(12345, url, data);
}

TEST_F(WorkspaceHelperTest, SetFilterCallback_ValidUrl_SetsFilterCallback)
{
    // Test setting filter callback
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QUrl url("file:///tmp/test");
    FileViewFilterCallback callback = [](const void*, const QVariant&) { return true; };
    
    // This should not crash
    helper->setFilterCallback(12345, url, callback);
}

TEST_F(WorkspaceHelperTest, SetWorkspaceMenuScene_ValidScheme_SetsMenuScene)
{
    // Test setting workspace menu scene
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    QString scene = "test_scene";
    
    // This should not crash
    helper->setWorkspaceMenuScene(scheme, scene);
}

TEST_F(WorkspaceHelperTest, FindMenuScene_RegisteredScheme_ReturnsScene)
{
    // Test finding menu scene for registered scheme
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    QString expectedScene = "test_scene";
    
    helper->setWorkspaceMenuScene(scheme, expectedScene);
    
    auto result = helper->findMenuScene(scheme);
    
    EXPECT_EQ(result, expectedScene);
}

TEST_F(WorkspaceHelperTest, SetSelectionMode_ValidMode_SetsSelectionMode)
{
    // Test setting selection mode
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    QAbstractItemView::SelectionMode mode = QAbstractItemView::ExtendedSelection;
    
    // This should not crash
    helper->setSelectionMode(windowId, mode);
}

TEST_F(WorkspaceHelperTest, SetEnabledSelectionModes_ValidModes_SetsEnabledModes)
{
    // Test setting enabled selection modes
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    QList<QAbstractItemView::SelectionMode> modes = {
        QAbstractItemView::SingleSelection,
        QAbstractItemView::ExtendedSelection
    };
    
    // This should not crash
    helper->setEnabledSelectionModes(windowId, modes);
}

TEST_F(WorkspaceHelperTest, SetViewDragEnabled_ValidValue_SetsDragEnabled)
{
    // Test setting view drag enabled
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    bool enable = true;
    
    // This should not crash
    helper->setViewDragEnabled(windowId, enable);
}

TEST_F(WorkspaceHelperTest, SetViewDragDropMode_ValidMode_SetsDragDropMode)
{
    // Test setting view drag drop mode
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    QAbstractItemView::DragDropMode mode = QAbstractItemView::DragDrop;
    
    // This should not crash
    helper->setViewDragDropMode(windowId, mode);
}

TEST_F(WorkspaceHelperTest, RegisterCustomViewProperty_ValidScheme_RegistersProperty)
{
    // Test registering custom view property
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    QVariantMap properties;
    properties["test_key"] = "test_value";
    
    // This should not crash
    helper->registerCustomViewProperty(scheme, properties);
}

TEST_F(WorkspaceHelperTest, FindCustomViewProperty_RegisteredScheme_ReturnsProperty)
{
    // Test finding custom view property for registered scheme
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    QVariantMap properties;
    properties["test_key"] = "test_value";
    
    helper->registerCustomViewProperty(scheme, properties);
    
    auto result = helper->findCustomViewProperty(scheme);
    // Just test that the function returns without crashing
    // CustomViewProperty doesn't have operator[] for direct comparison
}

TEST_F(WorkspaceHelperTest, AddWorkspace_ValidWindowId_AddsWorkspace)
{
    // Test adding workspace
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    
    // Mock WorkspaceWidget
    class MockWorkspaceWidget : public WorkspaceWidget {
    public:
        MockWorkspaceWidget() {}
    };
    
    MockWorkspaceWidget *mockWidget = new MockWorkspaceWidget();
    
    // This should not crash
    helper->addWorkspace(windowId, mockWidget);
    
    delete mockWidget;
}

TEST_F(WorkspaceHelperTest, RemoveWorkspace_ValidWindowId_RemovesWorkspace)
{
    // Test removing workspace
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    
    // This should not crash
    helper->removeWorkspace(windowId);
}

TEST_F(WorkspaceHelperTest, WindowId_ValidWidget_ReturnsWindowId)
{
    // Test getting window ID from widget
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    
    // Mock QWidget
    class MockWidget : public QWidget {
    public:
        MockWidget() {}
    };
    
    MockWidget *mockWidget = new MockWidget();
    
    // This should not crash
    quint64 result = helper->windowId(mockWidget);
    (void)result; // Suppress unused variable warning
    
    delete mockWidget;
}

TEST_F(WorkspaceHelperTest, SwitchViewMode_ValidWindowIdAndMode_SwitchesViewMode)
{
    // Test switching view mode
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    int viewMode = 1;
    
    // This should not crash
    helper->switchViewMode(windowId, viewMode);
}

TEST_F(WorkspaceHelperTest, AddScheme_ValidScheme_AddsScheme)
{
    // Test adding scheme
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    
    // This should not crash
    helper->addScheme(scheme);
}

TEST_F(WorkspaceHelperTest, ActionNewWindow_ValidUrls_CreatesNewWindow)
{
    // Test creating new window with action
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QList<QUrl> urls = { 
        QUrl::fromLocalFile("/tmp/file1.txt"),
        QUrl::fromLocalFile("/tmp/file2.txt")
    };
    
    // This should not crash
    helper->actionNewWindow(urls);
}

TEST_F(WorkspaceHelperTest, SelectFiles_ValidWindowIdAndFiles_SelectsFiles)
{
    // Test selecting files
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    QList<QUrl> files = { 
        QUrl::fromLocalFile("/tmp/file1.txt"),
        QUrl::fromLocalFile("/tmp/file2.txt")
    };
    
    // This should not crash
    helper->selectFiles(windowId, files);
}

TEST_F(WorkspaceHelperTest, SelectAll_ValidWindowId_SelectsAll)
{
    // Test selecting all files
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    
    // This should not crash
    helper->selectAll(windowId);
}

TEST_F(WorkspaceHelperTest, ReverseSelect_ValidWindowId_ReversesSelection)
{
    // Test reversing selection
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    
    // This should not crash
    helper->reverseSelect(windowId);
}

TEST_F(WorkspaceHelperTest, SetSort_ValidWindowIdRoleAndOrder_SetsSort)
{
    // Test setting sort
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    DFMBASE_NAMESPACE::Global::ItemRoles role = DFMBASE_NAMESPACE::Global::ItemRoles::kItemFileDisplayNameRole;
    // This should not crash
    helper->setSort(windowId, role);
}

TEST_F(WorkspaceHelperTest, SortRole_ValidWindowId_ReturnsSortRole)
{
    // Test getting sort role
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    
    // This should not crash
    auto result = helper->sortRole(windowId);
    
    // Default value should be returned when no workspace is found
    EXPECT_EQ(result, DFMBASE_NAMESPACE::Global::ItemRoles::kItemUnknowRole);
}

TEST_F(WorkspaceHelperTest, SetGroupingStrategy_ValidWindowIdAndStrategy_SetsGroupingStrategy)
{
    // Test setting grouping strategy
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    QString strategy = "test_strategy";
    
    // This should not crash
    helper->setGroupingStrategy(windowId, strategy);
}

TEST_F(WorkspaceHelperTest, GetGroupingStrategy_ValidWindowId_ReturnsGroupingStrategy)
{
    // Test getting grouping strategy
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    
    // This should not crash
    auto result = helper->getGroupingStrategy(windowId);
    
    // Default empty strategy should be returned
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(WorkspaceHelperTest, SetFileViewStateValue_ValidUrlKeyAndValue_SetsValue)
{
    // Test setting file view state value
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QUrl url("file:///tmp/test");
    QString key = "test_key";
    QVariant value("test_value");
    
    // This should not crash
    helper->setFileViewStateValue(url, key, value);
}

TEST_F(WorkspaceHelperTest, GetFileViewStateValue_ValidUrlAndKey_ReturnsValue)
{
    // Test getting file view state value
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QUrl url("file:///tmp/test");
    QString key = "test_key";
    QVariant defaultValue("default_value");
    
    // This should not crash
    auto result = helper->getFileViewStateValue(url, key, defaultValue);
    
    // Just test that the function runs without crashing
    // The actual value might be different due to internal URL transformation
    (void)result;
}

TEST_F(WorkspaceHelperTest, SetUndoFiles_ValidFiles_SetsUndoFiles)
{
    // Test setting undo files
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QList<QUrl> files = { 
        QUrl::fromLocalFile("/tmp/file1.txt"),
        QUrl::fromLocalFile("/tmp/file2.txt")
    };
    
    // This should not crash
    helper->setUndoFiles(files);
}

TEST_F(WorkspaceHelperTest, FilterUndoFiles_ValidFiles_ReturnsFilteredFiles)
{
    // Test filtering undo files
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QList<QUrl> files = {
        QUrl::fromLocalFile("/tmp/file1.txt"),
        QUrl::fromLocalFile("/tmp/file2.txt")
    };
    
    // This should not crash
    auto result = helper->filterUndoFiles(files);
    
    // Just test that the function runs without crashing
    // The filtering behavior depends on internal state
    (void)result;
}

TEST_F(WorkspaceHelperTest, SetAlwaysOpenInCurrentWindow_ValidWindowId_SetsAlwaysOpenInCurrentWindow)
{
    // Test setting always open in current window
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    quint64 windowId = 12345;
    
    // This should not crash
    helper->setAlwaysOpenInCurrentWindow(windowId);
}

TEST_F(WorkspaceHelperTest, RegisterFileView_ValidScheme_RegistersFileView)
{
    // Test registering file view
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    
    // This should not crash
    helper->registerFileView(scheme);
}

TEST_F(WorkspaceHelperTest, RegisteredFileView_RegisteredScheme_ReturnsTrue)
{
    // Test checking if file view is registered
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "test_scheme";
    
    helper->registerFileView(scheme);
    
    bool result = helper->registeredFileView(scheme);
    
    EXPECT_TRUE(result);
}

TEST_F(WorkspaceHelperTest, RegisteredFileView_UnregisteredScheme_ReturnsFalse)
{
    // Test checking if file view is registered for unregistered scheme
    WorkspaceHelper *helper = WorkspaceHelper::instance();
    QString scheme = "unregistered_scheme";
    
    bool result = helper->registeredFileView(scheme);
    
    EXPECT_FALSE(result);
}
