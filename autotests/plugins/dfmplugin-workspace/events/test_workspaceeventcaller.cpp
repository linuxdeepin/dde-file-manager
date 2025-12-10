// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "events/workspaceeventcaller.h"
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QItemSelection>
#include <QVariantMap>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

class WorkspaceEventCallerTest : public ::testing::Test
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

TEST_F(WorkspaceEventCallerTest, SendChangeCurrentUrl_ValidUrl_DoesNotCrash)
{
    // Test sending change current URL with valid URL
    QUrl url("file:///test/path");
    quint64 windowId = 12345;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendChangeCurrentUrl(windowId, url));
}

TEST_F(WorkspaceEventCallerTest, SendChangeCurrentUrl_EmptyUrl_DoesNotCrash)
{
    // Test sending change current URL with empty URL
    QUrl url;
    quint64 windowId = 12345;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendChangeCurrentUrl(windowId, url));
}

TEST_F(WorkspaceEventCallerTest, SendOpenAsAdmin_ValidUrl_DoesNotCrash)
{
    // Test sending open as admin with valid URL
    QUrl url("file:///test/file.txt");
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendOpenAsAdmin(url));
}

TEST_F(WorkspaceEventCallerTest, SendOpenAsAdmin_EmptyUrl_DoesNotCrash)
{
    // Test sending open as admin with empty URL
    QUrl url;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendOpenAsAdmin(url));
}

TEST_F(WorkspaceEventCallerTest, SendShowCustomTopWidget_ValidParameters_DoesNotCrash)
{
    // Test sending show custom top widget with valid parameters
    quint64 windowId = 12345;
    QString scheme = "file";
    bool visible = true;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendShowCustomTopWidget(windowId, scheme, visible));
}

TEST_F(WorkspaceEventCallerTest, SendShowCustomTopWidget_InvalidParameters_DoesNotCrash)
{
    // Test sending show custom top widget with invalid parameters
    quint64 windowId = 0;
    QString scheme = "";
    bool visible = false;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendShowCustomTopWidget(windowId, scheme, visible));
}

TEST_F(WorkspaceEventCallerTest, SendPaintEmblems_ValidParameters_DoesNotCrash)
{
    // Test sending paint emblems with valid parameters
    QPainter *painter = nullptr;
    QRectF rect(0, 0, 100, 100);
    FileInfoPointer info;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendPaintEmblems(painter, rect, info));
}

TEST_F(WorkspaceEventCallerTest, SendPaintEmblems_NullParameters_DoesNotCrash)
{
    // Test sending paint emblems with null parameters
    QPainter *painter = nullptr;
    QRectF rect;
    FileInfoPointer info;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendPaintEmblems(painter, rect, info));
}

TEST_F(WorkspaceEventCallerTest, SendViewSelectionChanged_ValidParameters_DoesNotCrash)
{
    // Test sending view selection changed with valid parameters
    quint64 windowId = 12345;
    QItemSelection selected;
    QItemSelection deselected;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendViewSelectionChanged(windowId, selected, deselected));
}

TEST_F(WorkspaceEventCallerTest, SendViewSelectionChanged_EmptySelection_DoesNotCrash)
{
    // Test sending view selection changed with empty selection
    quint64 windowId = 12345;
    QItemSelection selected;
    QItemSelection deselected;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendViewSelectionChanged(windowId, selected, deselected));
}

TEST_F(WorkspaceEventCallerTest, SendRenameStartEdit_ValidParameters_DoesNotCrash)
{
    // Test sending rename start edit with valid parameters
    quint64 windowId = 12345;
    QUrl url("file:///test/file.txt");
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendRenameStartEdit(windowId, url));
}

TEST_F(WorkspaceEventCallerTest, SendRenameStartEdit_EmptyUrl_DoesNotCrash)
{
    // Test sending rename start edit with empty URL
    quint64 windowId = 12345;
    QUrl url;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendRenameStartEdit(windowId, url));
}

TEST_F(WorkspaceEventCallerTest, SendRenameEndEdit_ValidParameters_DoesNotCrash)
{
    // Test sending rename end edit with valid parameters
    quint64 windowId = 12345;
    QUrl url("file:///test/file.txt");
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendRenameEndEdit(windowId, url));
}

TEST_F(WorkspaceEventCallerTest, SendRenameEndEdit_EmptyUrl_DoesNotCrash)
{
    // Test sending rename end edit with empty URL
    quint64 windowId = 12345;
    QUrl url;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendRenameEndEdit(windowId, url));
}

TEST_F(WorkspaceEventCallerTest, SendViewItemClicked_ValidData_DoesNotCrash)
{
    // Test sending view item clicked with valid data
    QVariantMap data;
    data["url"] = QUrl("file:///test/file.txt");
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendViewItemClicked(data));
}

TEST_F(WorkspaceEventCallerTest, SendViewItemClicked_EmptyUrl_DoesNotCrash)
{
    // Test sending view item clicked with empty URL
    QVariantMap data;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendViewItemClicked(data));
}

TEST_F(WorkspaceEventCallerTest, SendEnterDirReportLog_ValidData_DoesNotCrash)
{
    // Test sending enter dir report log with valid data
    QVariantMap data;
    data["url"] = QUrl("file:///test/directory");
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendEnterDirReportLog(data));
}

TEST_F(WorkspaceEventCallerTest, SendEnterDirReportLog_EmptyUrl_DoesNotCrash)
{
    // Test sending enter dir report log with empty URL
    QVariantMap data;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendEnterDirReportLog(data));
}

TEST_F(WorkspaceEventCallerTest, SendModelFilesEmpty_DoesNotCrash)
{
    // Test sending model files empty
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendModelFilesEmpty());
}

TEST_F(WorkspaceEventCallerTest, SendCheckTabAddable_ValidWindowId_DoesNotCrash)
{
    // Test sending check tab addable with valid window ID
    quint64 windowId = 12345;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendCheckTabAddable(windowId));
}

TEST_F(WorkspaceEventCallerTest, SendCheckTabAddable_InvalidWindowId_DoesNotCrash)
{
    // Test sending check tab addable with invalid window ID
    quint64 windowId = 0;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendCheckTabAddable(windowId));
}

TEST_F(WorkspaceEventCallerTest, SendCloseTab_ValidUrl_DoesNotCrash)
{
    // Test sending close tab with valid URL
    QUrl url("file:///test");
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendCloseTab(url));
}

TEST_F(WorkspaceEventCallerTest, SendCloseTab_EmptyUrl_DoesNotCrash)
{
    // Test sending close tab with empty URL
    QUrl url;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendCloseTab(url));
}

TEST_F(WorkspaceEventCallerTest, SendViewModeChanged_ValidParameters_DoesNotCrash)
{
    // Test sending view mode changed with valid parameters
    quint64 windowId = 12345;
    ViewMode mode = ViewMode::kIconMode;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendViewModeChanged(windowId, mode));
}

TEST_F(WorkspaceEventCallerTest, SendViewModeChanged_InvalidParameters_DoesNotCrash)
{
    // Test sending view mode changed with invalid parameters
    quint64 windowId = 0;
    ViewMode mode = ViewMode::kIconMode;
    
    // This should not crash
    EXPECT_NO_THROW(WorkspaceEventCaller::sendViewModeChanged(windowId, mode));
}