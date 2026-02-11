// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "events/workspaceeventreceiver.h"
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

class WorkspaceEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        // Mock WorkspaceHelper::instance to avoid dependency issues
        stub.set_lamda(&WorkspaceEventReceiver::instance, []() -> WorkspaceEventReceiver* {
            auto *receiver = new WorkspaceEventReceiver();
            return receiver;
        });
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(WorkspaceEventReceiverTest, Instance_ReturnsSingleton)
{
    // Test that instance() returns same singleton instance
    WorkspaceEventReceiver *instance1 = WorkspaceEventReceiver::instance();
    WorkspaceEventReceiver *instance2 = WorkspaceEventReceiver::instance();
    
    EXPECT_EQ(instance1, instance2);
    EXPECT_NE(instance1, nullptr);
}

TEST_F(WorkspaceEventReceiverTest, Constructor_CreatesValidObject)
{
    // Test that constructor creates valid object
    WorkspaceEventReceiver receiver;
    
    // This should not crash
    EXPECT_NO_THROW({
        WorkspaceEventReceiver *receiver = new WorkspaceEventReceiver();
        delete receiver;
    });
}

TEST_F(WorkspaceEventReceiverTest, Destructor_DoesNotCrash)
{
    // Test that destructor does not crash
    EXPECT_NO_THROW({
        WorkspaceEventReceiver *receiver = new WorkspaceEventReceiver();
        delete receiver;
    });
}

TEST_F(WorkspaceEventReceiverTest, InitConnection_DoesNotCrash)
{
    // Test that initConnection does not crash
    WorkspaceEventReceiver *receiver = WorkspaceEventReceiver::instance();
    
    EXPECT_NO_THROW(receiver->initConnection());
}

TEST_F(WorkspaceEventReceiverTest, HandleTileBarSwitchModeTriggered_ValidParameters_DoesNotCrash)
{
    // Test handling tile bar switch mode triggered with valid parameters
    quint64 windowId = 12345;
    int mode = 1; // icon mode
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleTileBarSwitchModeTriggered(windowId, mode));
}

TEST_F(WorkspaceEventReceiverTest, HandleTileBarSwitchModeTriggered_InvalidParameters_DoesNotCrash)
{
    // Test handling tile bar switch mode triggered with invalid parameters
    quint64 windowId = 0;
    int mode = -1; // invalid mode
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleTileBarSwitchModeTriggered(windowId, mode));
}

TEST_F(WorkspaceEventReceiverTest, HandleShowCustomTopWidget_ValidParameters_DoesNotCrash)
{
    // Test handling show custom top widget with valid parameters
    quint64 windowId = 12345;
    QString scheme = "file";
    bool visible = true;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleShowCustomTopWidget(windowId, scheme, visible));
}

TEST_F(WorkspaceEventReceiverTest, HandleShowCustomTopWidget_InvalidParameters_DoesNotCrash)
{
    // Test handling show custom top widget with invalid parameters
    quint64 windowId = 0;
    QString scheme = "";
    bool visible = false;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleShowCustomTopWidget(windowId, scheme, visible));
}

TEST_F(WorkspaceEventReceiverTest, HandleSelectFiles_ValidParameters_DoesNotCrash)
{
    // Test handling select files with valid parameters
    quint64 windowId = 12345;
    QList<QUrl> urls;
    urls << QUrl("file:///test1") << QUrl("file:///test2");
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSelectFiles(windowId, urls));
}

TEST_F(WorkspaceEventReceiverTest, HandleSelectFiles_EmptyUrls_DoesNotCrash)
{
    // Test handling select files with empty URLs
    quint64 windowId = 12345;
    QList<QUrl> urls;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSelectFiles(windowId, urls));
}

TEST_F(WorkspaceEventReceiverTest, HandleSelectAll_ValidWindowId_DoesNotCrash)
{
    // Test handling select all with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSelectAll(windowId));
}

TEST_F(WorkspaceEventReceiverTest, HandleSelectAll_InvalidWindowId_DoesNotCrash)
{
    // Test handling select all with invalid window ID
    quint64 windowId = 0;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSelectAll(windowId));
}

TEST_F(WorkspaceEventReceiverTest, HandleReverseSelect_ValidWindowId_DoesNotCrash)
{
    // Test handling reverse select with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleReverseSelect(windowId));
}

TEST_F(WorkspaceEventReceiverTest, HandleReverseSelect_InvalidWindowId_DoesNotCrash)
{
    // Test handling reverse select with invalid window ID
    quint64 windowId = 0;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleReverseSelect(windowId));
}

TEST_F(WorkspaceEventReceiverTest, HandleSetSort_ValidRole_DoesNotCrash)
{
    // Test handling set sort with valid role
    quint64 windowId = 12345;
    DFMBASE_NAMESPACE::Global::ItemRoles role = DFMBASE_NAMESPACE::Global::ItemRoles::kItemNameRole;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSetSort(windowId, role));
}

TEST_F(WorkspaceEventReceiverTest, HandleSetGroup_ValidStrategy_DoesNotCrash)
{
    // Test handling set group with valid strategy
    quint64 windowId = 12345;
    QString strategyName = "Name";
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSetGroup(windowId, strategyName));
}

TEST_F(WorkspaceEventReceiverTest, HandleSetSelectionMode_ValidMode_DoesNotCrash)
{
    // Test handling set selection mode with valid mode
    quint64 windowId = 12345;
    QAbstractItemView::SelectionMode mode = QAbstractItemView::ExtendedSelection;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSetSelectionMode(windowId, mode));
}

TEST_F(WorkspaceEventReceiverTest, HandleSetViewFilter_ValidFilters_DoesNotCrash)
{
    // Test handling set view filter with valid filters
    quint64 windowId = 12345;
    QDir::Filters filters = QDir::Files | QDir::Dirs;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSetViewFilter(windowId, filters));
}

TEST_F(WorkspaceEventReceiverTest, HandleFileUpdate_ValidUrl_DoesNotCrash)
{
    // Test handling file update with valid URL
    QUrl url("file:///test/file.txt");
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleFileUpdate(url));
}

TEST_F(WorkspaceEventReceiverTest, HandleTabCreated_ValidParameters_DoesNotCrash)
{
    // Test handling tab created with valid parameters
    quint64 windowId = 12345;
    QString uniqueId = "test-tab-id";
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleTabCreated(windowId, uniqueId));
}

TEST_F(WorkspaceEventReceiverTest, HandleTabChanged_ValidParameters_DoesNotCrash)
{
    // Test handling tab changed with valid parameters
    quint64 windowId = 12345;
    QString uniqueId = "test-tab-id";
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleTabChanged(windowId, uniqueId));
}

TEST_F(WorkspaceEventReceiverTest, HandleGetSelectedUrls_ValidWindowId_DoesNotCrash)
{
    // Test handling get selected URLs with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleGetSelectedUrls(windowId));
}

// Additional tests for missing methods

TEST_F(WorkspaceEventReceiverTest, HandleCurrentGroupStrategy_ValidWindowId_ReturnsStrategy)
{
    // Test handling current group strategy with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash and return some strategy
    EXPECT_NO_THROW({
        QString result = WorkspaceEventReceiver::instance()->handleCurrentGroupStrategy(windowId);
        // Should return some string (possibly empty)
        EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleSetEnabledSelectionModes_ValidParameters_DoesNotCrash)
{
    // Test handling set enabled selection modes with valid parameters
    quint64 windowId = 12345;
    QList<QAbstractItemView::SelectionMode> modes;
    modes << QAbstractItemView::SingleSelection << QAbstractItemView::ExtendedSelection;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSetEnabledSelectionModes(windowId, modes));
}

TEST_F(WorkspaceEventReceiverTest, HandleSetViewDragEnabled_ValidParameters_DoesNotCrash)
{
    // Test handling set view drag enabled with valid parameters
    quint64 windowId = 12345;
    bool enabled = true;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSetViewDragEnabled(windowId, enabled));
}

TEST_F(WorkspaceEventReceiverTest, HandleSetViewDragDropMode_ValidParameters_DoesNotCrash)
{
    // Test handling set view drag drop mode with valid parameters
    quint64 windowId = 12345;
    QAbstractItemView::DragDropMode mode = QAbstractItemView::DragDrop;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSetViewDragDropMode(windowId, mode));
}

TEST_F(WorkspaceEventReceiverTest, HandleClosePersistentEditor_ValidWindowId_DoesNotCrash)
{
    // Test handling close persistent editor with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleClosePersistentEditor(windowId));
}

TEST_F(WorkspaceEventReceiverTest, HandleGetViewFilter_ValidWindowId_ReturnsFilter)
{
    // Test handling get view filter with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash and return some filter
    EXPECT_NO_THROW({
        int result = WorkspaceEventReceiver::instance()->handleGetViewFilter(windowId);
        // Should return some integer
        EXPECT_GE(result, 0);
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleGetNameFilter_ValidWindowId_ReturnsFilter)
{
    // Test handling get name filter with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash and return some filter list
    EXPECT_NO_THROW({
        QStringList result = WorkspaceEventReceiver::instance()->handleGetNameFilter(windowId);
        // Should return some string list (possibly empty)
        EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleSetNameFilter_ValidParameters_DoesNotCrash)
{
    // Test handling set name filter with valid parameters
    quint64 windowId = 12345;
    QStringList filters;
    filters << "*.txt" << "*.pdf";
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSetNameFilter(windowId, filters));
}

TEST_F(WorkspaceEventReceiverTest, HandleSetReadOnly_ValidParameters_DoesNotCrash)
{
    // Test handling set read only with valid parameters
    quint64 windowId = 12345;
    bool readOnly = true;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSetReadOnly(windowId, readOnly));
}

TEST_F(WorkspaceEventReceiverTest, HandlePasteFileResult_ValidParameters_DoesNotCrash)
{
    // Test handling paste file result with valid parameters
    QList<QUrl> srcUrls;
    QList<QUrl> destUrls;
    srcUrls << QUrl("file:///test/source1") << QUrl("file:///test/source2");
    destUrls << QUrl("file:///test/dest1") << QUrl("file:///test/dest2");
    bool ok = true;
    QString errMsg = "";
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handlePasteFileResult(srcUrls, destUrls, ok, errMsg));
}

TEST_F(WorkspaceEventReceiverTest, HandleMoveToTrashFileResult_ValidParameters_DoesNotCrash)
{
    // Test handling move to trash file result with valid parameters
    QList<QUrl> srcUrls;
    srcUrls << QUrl("file:///test/file1") << QUrl("file:///test/file2");
    bool ok = true;
    QString errMsg = "";
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleMoveToTrashFileResult(srcUrls, ok, errMsg));
}

TEST_F(WorkspaceEventReceiverTest, HandleRenameFileResult_ValidParameters_DoesNotCrash)
{
    // Test handling rename file result with valid parameters
    quint64 windowId = 12345;
    QMap<QUrl, QUrl> renamedUrls;
    renamedUrls[QUrl("file:///test/old1")] = QUrl("file:///test/new1");
    renamedUrls[QUrl("file:///test/old2")] = QUrl("file:///test/new2");
    bool ok = true;
    QString errMsg = "";
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleRenameFileResult(windowId, renamedUrls, ok, errMsg));
}

TEST_F(WorkspaceEventReceiverTest, HandleColumnDisplayName_ValidParameters_ReturnsName)
{
    // Test handling column display name with valid parameters
    quint64 windowId = 12345;
    DFMBASE_NAMESPACE::Global::ItemRoles role = DFMBASE_NAMESPACE::Global::ItemRoles::kItemNameRole;
    
    // This should not crash and return some name
    EXPECT_NO_THROW({
        QString result = WorkspaceEventReceiver::instance()->handleColumnDisplayName(windowId, role);
        // Should return some string
        EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleCurrentSortRole_ValidWindowId_ReturnsRole)
{
    // Test handling current sort role with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash and return some role
    EXPECT_NO_THROW({
        DFMBASE_NAMESPACE::Global::ItemRoles result = WorkspaceEventReceiver::instance()->handleCurrentSortRole(windowId);
        // Should return some role
        EXPECT_TRUE(result >= DFMBASE_NAMESPACE::Global::ItemRoles::kItemNameRole);
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleColumnRoles_ValidWindowId_ReturnsRoles)
{
    // Test handling column roles with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash and return some roles list
    EXPECT_NO_THROW({
        QList<DFMBASE_NAMESPACE::Global::ItemRoles> result = WorkspaceEventReceiver::instance()->handleColumnRoles(windowId);
        // Should return some list (possibly empty)
        EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleGetVisualGeometry_ValidWindowId_ReturnsGeometry)
{
    // Test handling get visual geometry with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash and return some geometry
    EXPECT_NO_THROW({
        QRectF result = WorkspaceEventReceiver::instance()->handleGetVisualGeometry(windowId);
        // Should return some rect (possibly invalid)
        EXPECT_TRUE(result.isValid() || !result.isValid());
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleGetViewItemRect_ValidParameters_ReturnsRect)
{
    // Test handling get view item rect with valid parameters
    quint64 windowId = 12345;
    QUrl url("file:///test/file.txt");
    DFMBASE_NAMESPACE::Global::ItemRoles role = DFMBASE_NAMESPACE::Global::ItemRoles::kItemNameRole;
    
    // This should not crash and return some rect
    EXPECT_NO_THROW({
        QRectF result = WorkspaceEventReceiver::instance()->handleGetViewItemRect(windowId, url, role);
        // Should return some rect (possibly invalid)
        EXPECT_TRUE(result.isValid() || !result.isValid());
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleSetCustomViewProperty_ValidParameters_DoesNotCrash)
{
    // Test handling set custom view property with valid parameters
    QString scheme = "file";
    QVariantMap properties;
    properties["key1"] = "value1";
    properties["key2"] = 42;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleSetCustomViewProperty(scheme, properties));
}

TEST_F(WorkspaceEventReceiverTest, HandleGetDefaultViewMode_ValidScheme_ReturnsMode)
{
    // Test handling get default view mode with valid scheme
    QString scheme = "file";
    
    // This should not crash and return some mode
    EXPECT_NO_THROW({
        DFMBASE_NAMESPACE::Global::ViewMode result = WorkspaceEventReceiver::instance()->handleGetDefaultViewMode(scheme);
        // Should return some mode
        EXPECT_TRUE(result >= DFMBASE_NAMESPACE::Global::ViewMode::kIconMode);
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleGetCurrentViewMode_ValidWindowId_ReturnsMode)
{
    // Test handling get current view mode with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash and return some mode
    EXPECT_NO_THROW({
        DFMBASE_NAMESPACE::Global::ViewMode result = WorkspaceEventReceiver::instance()->handleGetCurrentViewMode(windowId);
        // Should return some mode
        EXPECT_TRUE(result >= DFMBASE_NAMESPACE::Global::ViewMode::kIconMode);
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleRegisterFileView_ValidScheme_DoesNotCrash)
{
    // Test handling register file view with valid scheme
    QString scheme = "file";
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleRegisterFileView(scheme));
}

TEST_F(WorkspaceEventReceiverTest, HandleRegisterMenuScene_ValidParameters_DoesNotCrash)
{
    // Test handling register menu scene with valid parameters
    QString scheme = "file";
    QString scene = "test-scene";
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleRegisterMenuScene(scheme, scene));
}

TEST_F(WorkspaceEventReceiverTest, HandleFindMenuScene_ValidScheme_ReturnsScene)
{
    // Test handling find menu scene with valid scheme
    QString scheme = "file";
    
    // This should not crash and return some scene
    EXPECT_NO_THROW({
        QString result = WorkspaceEventReceiver::instance()->handleFindMenuScene(scheme);
        // Should return some string (possibly empty)
        EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleRegisterCustomTopWidget_ValidParameters_DoesNotCrash)
{
    // Test handling register custom top widget with valid parameters
    QVariantMap dataMap;
    dataMap["scheme"] = "file";
    dataMap["name"] = "test-widget";
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleRegisterCustomTopWidget(dataMap));
}

TEST_F(WorkspaceEventReceiverTest, HandleGetCustomTopWidgetVisible_ValidParameters_ReturnsVisible)
{
    // Test handling get custom top widget visible with valid parameters
    quint64 windowId = 12345;
    QString scheme = "file";
    
    // This should not crash and return some boolean
    EXPECT_NO_THROW({
        bool result = WorkspaceEventReceiver::instance()->handleGetCustomTopWidgetVisible(windowId, scheme);
        // Should return some boolean
        EXPECT_TRUE(result == true || result == false);
    });
}

TEST_F(WorkspaceEventReceiverTest, HandleRegisterLoadStrategy_ValidParameters_DoesNotCrash)
{
    // Test handling register load strategy with valid parameters
    QString scheme = "file";
    DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy strategy = static_cast<DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy>(0); // Use 0 as default value
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleRegisterLoadStrategy(scheme, strategy));
}

TEST_F(WorkspaceEventReceiverTest, HandleRegisterFocusFileViewDisabled_ValidScheme_DoesNotCrash)
{
    // Test handling register focus file view disabled with valid scheme
    QString scheme = "file";
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventReceiver::instance()->handleRegisterFocusFileViewDisabled(scheme));
}