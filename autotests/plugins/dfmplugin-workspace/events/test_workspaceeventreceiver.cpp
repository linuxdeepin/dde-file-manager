// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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