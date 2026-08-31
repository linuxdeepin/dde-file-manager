// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_workspaceeventreceiver_1.cpp
 * @brief Unit tests for WorkspaceEventReceiver methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/workspaceeventreceiver.h"

#include <QTest>

using namespace dfmplugin_workspace;

class WorkspaceEventReceiverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new WorkspaceEventReceiver();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    WorkspaceEventReceiver *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(WorkspaceEventReceiverTest, WorkspaceEventReceiver)
{
    // Test constructor: WorkspaceEventReceiver((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(WorkspaceEventReceiverTest, handleAboutToChangeViewWidth)
{
    // Test method: void handleAboutToChangeViewWidth((const quint64 windowID, int deltaWidth))
    EXPECT_NO_FATAL_FAILURE(obj->handleAboutToChangeViewWidth(0, 0));
}

TEST_F(WorkspaceEventReceiverTest, handleCheckSchemeViewIsFileView)
{
    // Test method: bool handleCheckSchemeViewIsFileView((const QString &scheme))
    QString _arg0{};
    auto result = obj->handleCheckSchemeViewIsFileView(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventReceiverTest, handleColumnDisplayName)
{
    // Test method: QString handleColumnDisplayName((quint64 windowId, dfmbase::Global::ItemRoles role))
    auto result = obj->handleColumnDisplayName(0, {});
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceEventReceiverTest, handleColumnRoles)
{
    // Test method: QList<ItemRoles> handleColumnRoles((quint64 windowId))
    auto result = obj->handleColumnRoles(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceEventReceiverTest, handleCurrentGroupStrategy)
{
    // Test method: QString handleCurrentGroupStrategy((quint64 windowId))
    auto result = obj->handleCurrentGroupStrategy(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceEventReceiverTest, handleCurrentSortRole)
{
    // Test method: ItemRoles handleCurrentSortRole((quint64 windowId))
    auto result = obj->handleCurrentSortRole(0);
    EXPECT_NO_FATAL_FAILURE({ obj->handleCurrentSortRole(0); });

}

TEST_F(WorkspaceEventReceiverTest, handleFileUpdate)
{
    // Test method: void handleFileUpdate((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileUpdate(_arg0));
}

TEST_F(WorkspaceEventReceiverTest, handleFindMenuScene)
{
    // Test method: QString handleFindMenuScene((const QString &scheme))
    QString _arg0{};
    auto result = obj->handleFindMenuScene(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceEventReceiverTest, handleGetColumnWidth)
{
    // Test method: int handleGetColumnWidth((quint64 windowId, DFMBASE_NAMESPACE::Global::ItemRoles role))
    auto result = obj->handleGetColumnWidth(0, DFMBASE_NAMESPACE::Global::ItemRoles());
    EXPECT_GE(result, 0);

}

TEST_F(WorkspaceEventReceiverTest, handleGetCurrentModelBusy)
{
    // Test method: bool handleGetCurrentModelBusy((quint64 windowId))
    auto result = obj->handleGetCurrentModelBusy(0);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventReceiverTest, handleGetCurrentViewMode)
{
    // Test method: ViewMode handleGetCurrentViewMode((const quint64 windowID))
    auto result = obj->handleGetCurrentViewMode(0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(WorkspaceEventReceiverTest, handleGetCustomTopWidgetVisible)
{
    // Test method: bool handleGetCustomTopWidgetVisible((const quint64 windowID, const QString &scheme))
    QString _arg1{};
    auto result = obj->handleGetCustomTopWidgetVisible(0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventReceiverTest, handleGetDefaultViewMode)
{
    // Test method: ViewMode handleGetDefaultViewMode((const QString &scheme))
    QString _arg0{};
    auto result = obj->handleGetDefaultViewMode(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(WorkspaceEventReceiverTest, handleGetNameFilter)
{
    // Test method: QStringList handleGetNameFilter((const quint64 windowId))
    auto result = obj->handleGetNameFilter(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceEventReceiverTest, handleGetSelectedUrls)
{
    // Test method: QList<QUrl> handleGetSelectedUrls((const quint64 windowID))
    auto result = obj->handleGetSelectedUrls(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(WorkspaceEventReceiverTest, handleGetViewFilter)
{
    // Test method: int handleGetViewFilter((const quint64 windowId))
    auto result = obj->handleGetViewFilter(0);
    EXPECT_GE(result, 0);

}

TEST_F(WorkspaceEventReceiverTest, handleGetViewItemRect)
{
    // Test method: QRectF handleGetViewItemRect((const quint64 windowID, const QUrl &url, const ItemRoles role))
    QUrl _arg1{};
    auto result = obj->handleGetViewItemRect(0, _arg1, ItemRoles());
    EXPECT_FALSE(result.isValid());

}

TEST_F(WorkspaceEventReceiverTest, handleGetVisualGeometry)
{
    // Test method: QRectF handleGetVisualGeometry((const quint64 windowID))
    auto result = obj->handleGetVisualGeometry(0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(WorkspaceEventReceiverTest, handleMoveToTrashFileResult)
{
    // Test method: void handleMoveToTrashFileResult((const QList<QUrl> &srcUrls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleMoveToTrashFileResult(_arg0, false, _arg2));
}

TEST_F(WorkspaceEventReceiverTest, handlePasteFileResult)
{
    // Test method: void handlePasteFileResult((const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg))
    QList<QUrl> _arg0{};
    QList<QUrl> _arg1{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handlePasteFileResult(_arg0, _arg1, false, _arg3));
}

TEST_F(WorkspaceEventReceiverTest, handleRegisterCustomTopWidget)
{
    // Test method: void handleRegisterCustomTopWidget((const QVariantMap &dataMap))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRegisterCustomTopWidget(_arg0));
}

TEST_F(WorkspaceEventReceiverTest, handleRegisterDataCache)
{
    // Test method: void handleRegisterDataCache((const QString &scheme))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRegisterDataCache(_arg0));
}

TEST_F(WorkspaceEventReceiverTest, handleRegisterFileView)
{
    // Test method: void handleRegisterFileView((const QString &scheme))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRegisterFileView(_arg0));
}

TEST_F(WorkspaceEventReceiverTest, handleRegisterFocusFileViewDisabled)
{
    // Test method: void handleRegisterFocusFileViewDisabled((const QString &scheme))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRegisterFocusFileViewDisabled(_arg0));
}

TEST_F(WorkspaceEventReceiverTest, handleRegisterGroupStrategy)
{
    // Test method: void handleRegisterGroupStrategy((const QVariantMap &dataMap))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRegisterGroupStrategy(_arg0));
}

TEST_F(WorkspaceEventReceiverTest, handleRegisterLoadStrategy)
{
    // Test method: void handleRegisterLoadStrategy((const QString &scheme, DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy strategy))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRegisterLoadStrategy(_arg0, DFMGLOBAL_NAMESPACE::DirectoryLoadStrategy()));
}

TEST_F(WorkspaceEventReceiverTest, handleRegisterMenuScene)
{
    // Test method: void handleRegisterMenuScene((const QString &scheme, const QString &scene))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRegisterMenuScene(_arg0, _arg1));
}

TEST_F(WorkspaceEventReceiverTest, handleRegisterRoutePrehandle)
{
    // Test method: bool handleRegisterRoutePrehandle((const QString &scheme, const FileViewRoutePrehaldler &prehandler))
    QString _arg0{};
    FileViewRoutePrehaldler _arg1{};
    auto result = obj->handleRegisterRoutePrehandle(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(WorkspaceEventReceiverTest, handleRegisterViewHint)
{
    // Test method: void handleRegisterViewHint((const QVariantMap &dataMap))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRegisterViewHint(_arg0));
}

TEST_F(WorkspaceEventReceiverTest, handleRenameFileResult)
{
    // Test method: void handleRenameFileResult((const quint64 windowId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg))
    QMap<QUrl, QUrl> _arg1{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleRenameFileResult(0, _arg1, false, _arg3));
}

TEST_F(WorkspaceEventReceiverTest, handleReverseSelect)
{
    // Test method: void handleReverseSelect((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->handleReverseSelect(0));
}

TEST_F(WorkspaceEventReceiverTest, handleSelectAll)
{
    // Test method: void handleSelectAll((quint64 windowId))
    EXPECT_NO_FATAL_FAILURE(obj->handleSelectAll(0));
}

TEST_F(WorkspaceEventReceiverTest, handleSelectFiles)
{
    // Test method: void handleSelectFiles((quint64 windowId, const QList<QUrl> &files))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSelectFiles(0, _arg1));
}

TEST_F(WorkspaceEventReceiverTest, handleSetAlwaysOpenInCurrentWindow)
{
    // Test method: void handleSetAlwaysOpenInCurrentWindow((const quint64 windowID))
    EXPECT_NO_FATAL_FAILURE(obj->handleSetAlwaysOpenInCurrentWindow(0));
}

TEST_F(WorkspaceEventReceiverTest, handleSetCustomFilterCallback)
{
    // Test method: void handleSetCustomFilterCallback((quint64 windowID, const QUrl &url, const QVariant callback))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSetCustomFilterCallback(0, _arg1, QVariant()));
}

TEST_F(WorkspaceEventReceiverTest, handleSetCustomFilterData)
{
    // Test method: void handleSetCustomFilterData((quint64 windowID, const QUrl &url, const QVariant &data))
    QUrl _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSetCustomFilterData(0, _arg1, _arg2));
}

TEST_F(WorkspaceEventReceiverTest, handleSetCustomViewProperty)
{
    // Test method: void handleSetCustomViewProperty((const QString &scheme, const QVariantMap &properties))
    QString _arg0{};
    QVariantMap _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSetCustomViewProperty(_arg0, _arg1));
}

TEST_F(WorkspaceEventReceiverTest, handleSetEnabledSelectionModes)
{
    // Test method: void handleSetEnabledSelectionModes((const quint64 windowId, const QList<QAbstractItemView::SelectionMode> &modes))
    QList<QAbstractItemView::SelectionMode> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSetEnabledSelectionModes(0, _arg1));
}

TEST_F(WorkspaceEventReceiverTest, handleSetGroup)
{
    // Test method: void handleSetGroup((quint64 windowId, const QString &strategyName))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSetGroup(0, _arg1));
}

TEST_F(WorkspaceEventReceiverTest, handleSetNameFilter)
{
    // Test method: void handleSetNameFilter((const quint64 windowId, const QStringList &filters))
    QStringList _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSetNameFilter(0, _arg1));
}

TEST_F(WorkspaceEventReceiverTest, handleSetReadOnly)
{
    // Test method: void handleSetReadOnly((const quint64 windowId, const bool readOnly))
    EXPECT_NO_FATAL_FAILURE(obj->handleSetReadOnly(0, false));
}

TEST_F(WorkspaceEventReceiverTest, handleSetSelectionMode)
{
    // Test method: void handleSetSelectionMode((const quint64 windowId, const QAbstractItemView::SelectionMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->handleSetSelectionMode(0, QAbstractItemView::SelectionMode()));
}

TEST_F(WorkspaceEventReceiverTest, handleSetSort)
{
    // Test method: void handleSetSort((quint64 windowId, ItemRoles role))
    EXPECT_NO_FATAL_FAILURE(obj->handleSetSort(0, ItemRoles()));
}

TEST_F(WorkspaceEventReceiverTest, handleSetViewDragDropMode)
{
    // Test method: void handleSetViewDragDropMode((const quint64 windowId, const QAbstractItemView::DragDropMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->handleSetViewDragDropMode(0, QAbstractItemView::DragDropMode()));
}

TEST_F(WorkspaceEventReceiverTest, handleSetViewDragEnabled)
{
    // Test method: void handleSetViewDragEnabled((const quint64 windowId, const bool enabled))
    EXPECT_NO_FATAL_FAILURE(obj->handleSetViewDragEnabled(0, false));
}

TEST_F(WorkspaceEventReceiverTest, handleSetViewFilter)
{
    // Test method: void handleSetViewFilter((const quint64 windowId, const QDir::Filters &filters))
    QDir::Filters _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleSetViewFilter(0, _arg1));
}

TEST_F(WorkspaceEventReceiverTest, handleShowCustomTopWidget)
{
    // Test method: void handleShowCustomTopWidget((quint64 windowId, const QString &scheme, bool visible))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleShowCustomTopWidget(0, _arg1, false));
}

TEST_F(WorkspaceEventReceiverTest, handleTabChanged)
{
    // Test method: void handleTabChanged((const quint64 windowId, const QString &uniqueId))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleTabChanged(0, _arg1));
}

TEST_F(WorkspaceEventReceiverTest, handleTabCreated)
{
    // Test method: void handleTabCreated((const quint64 windowId, const QString &uniqueId))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleTabCreated(0, _arg1));
}

TEST_F(WorkspaceEventReceiverTest, handleTileBarSwitchModeTriggered)
{
    // Test method: void handleTileBarSwitchModeTriggered((quint64 windowId, int mode))
    EXPECT_NO_FATAL_FAILURE(obj->handleTileBarSwitchModeTriggered(0, 0));
}

TEST_F(WorkspaceEventReceiverTest, instance)
{
    // Test getter: WorkspaceEventReceiver instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(WorkspaceEventReceiverTest, WorkspaceEventReceiver_Destructor)
{
    // Test method:  ~WorkspaceEventReceiver(())
    EXPECT_NO_FATAL_FAILURE({ WorkspaceEventReceiver *tmp = new WorkspaceEventReceiver(); delete tmp; });
}
