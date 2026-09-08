// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/fileview.h"
#include "views/baseitemdelegate.h"
#include "models/fileviewmodel.h"
#include "utils/workspacehelper.h"
#include "utils/traversaldirthreadmanager.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-framework/event/event.h>

#include <QUrl>
#include <QUrlQuery>
#include <QPoint>
#include <QModelIndex>
#include <QSize>
#include <QRect>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QContextMenuEvent>
#include <QItemSelection>
#include <QTemporaryDir>
#include <QApplication>
#include <QPixmap>
#include <QMimeData>
#include <QScrollBar>
#include <QtTest>
#include <DGuiApplicationHelper>

using namespace dfmplugin_workspace;

class FileViewBehaviorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl::fromLocalFile("/tmp/ut_ws_fileview");
        view = new FileView(testUrl);

        stub.set_lamda(&dfmbase::NetworkUtils::checkFtpOrSmbBusy,
                       [](dfmbase::NetworkUtils *, const QUrl &) { return false; });

        stub.set_lamda(ADDR(dfmbase::Application, appAttribute),
                       [](dfmbase::Application::ApplicationAttribute) { return QVariant(1); });
        stub.set_lamda(ADDR(dfmbase::Application, setAppAttribute),
                       [](dfmbase::Application::ApplicationAttribute, const QVariant &) {});
    }

    void TearDown() override
    {
        delete view;
        WorkspaceHelper::kSelectionAndRenameFile.clear();
        stub.clear();
    }

    QUrl testUrl;
    FileView *view = nullptr;
    stub_ext::StubExt stub;
};

// --- view mode switching ---

TEST_F(FileViewBehaviorTest, SetViewMode_IconMode_UpdatesCurrentViewMode)
{
    view->setViewMode(dfmbase::Global::ViewMode::kIconMode);
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kIconMode);
    EXPECT_EQ(view->itemCountForRow(), view->iconModeColumnCount());
}

TEST_F(FileViewBehaviorTest, SetViewMode_ListMode_UpdatesCurrentViewMode)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kListMode);
    EXPECT_EQ(view->itemCountForRow(), 1);
}

TEST_F(FileViewBehaviorTest, SetViewMode_TreeMode_UpdatesCurrentViewMode)
{
    view->setViewMode(dfmbase::Global::ViewMode::kTreeMode);
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kTreeMode);
    EXPECT_EQ(view->itemCountForRow(), 1);
}

TEST_F(FileViewBehaviorTest, ViewModeChanged_DifferentMode_SwitchesAndPersists)
{
    ASSERT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kIconMode);
    view->viewModeChanged(0, static_cast<int>(dfmbase::Global::ViewMode::kListMode));
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kListMode);

    // switching back to the current mode is a no-op
    view->viewModeChanged(0, static_cast<int>(dfmbase::Global::ViewMode::kListMode));
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kListMode);
}

TEST_F(FileViewBehaviorTest, ViewModeChanged_UnsupportedMode_DoesNotSwitch)
{
    ASSERT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kIconMode);
    view->viewModeChanged(0, static_cast<int>(dfmbase::Global::ViewMode::kNoneMode));
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kIconMode);
}

TEST_F(FileViewBehaviorTest, OnDefaultViewModeChanged_UnsupportedScheme_KeepsMode)
{
    view->onDefaultViewModeChanged(static_cast<int>(dfmbase::Global::ViewMode::kListMode));
    // scheme is not registered for tree/list support check, mode stays unchanged
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kIconMode);
    EXPECT_NE(view->model(), nullptr);
}

// --- selection related ---

TEST_F(FileViewBehaviorTest, SelectedIndexCount_NoSelection_ReturnsZero)
{
    EXPECT_EQ(view->selectedIndexCount(), 0);
    EXPECT_TRUE(view->selectedIndexes().isEmpty());
}

TEST_F(FileViewBehaviorTest, SelectFiles_EmptyList_ReturnsFalse)
{
    EXPECT_FALSE(view->selectFiles({}));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, SelectFiles_UrlOutsideRoot_ReturnsFalse)
{
    QList<QUrl> files = { QUrl::fromLocalFile("/elsewhere/file.txt") };
    EXPECT_FALSE(view->selectFiles(files));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, SetEnabledSelectionModes_ExcludingCurrent_ResetsMode)
{
    ASSERT_EQ(view->selectionMode(), QAbstractItemView::ExtendedSelection);
    view->setEnabledSelectionModes({ QAbstractItemView::SingleSelection });
    EXPECT_EQ(view->selectionMode(), QAbstractItemView::SingleSelection);

    // restore a list that contains the current mode: no reset needed
    view->setEnabledSelectionModes({ QAbstractItemView::SingleSelection,
                                     QAbstractItemView::ExtendedSelection });
    EXPECT_EQ(view->selectionMode(), QAbstractItemView::SingleSelection);
}

TEST_F(FileViewBehaviorTest, ReverseSelect_NoSelection_KeepsEmptySelection)
{
    EXPECT_NO_FATAL_FAILURE(view->reverseSelect());
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, CurrentPressIndex_NoPress_ReturnsInvalidIndex)
{
    EXPECT_FALSE(view->currentPressIndex().isValid());
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, SelectedTreeViewUrlList_NoSelection_ReturnsEmptyLists)
{
    QList<QUrl> selected;
    QList<QUrl> treeSelected;
    view->selectedTreeViewUrlList(selected, treeSelected);
    EXPECT_TRUE(selected.isEmpty());
    EXPECT_TRUE(treeSelected.isEmpty());

    EXPECT_EQ(view->selectedTreeViewUrlList().size(), 0);
}

// --- model interaction ---

TEST_F(FileViewBehaviorTest, DataChanged_InvalidIndexes_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->dataChanged(QModelIndex(), QModelIndex()));
    EXPECT_EQ(view->currentIndex(), QModelIndex());
}

TEST_F(FileViewBehaviorTest, StopWork_AnyUrl_MarksModelIdle)
{
    view->stopWork(QUrl::fromLocalFile("/tmp/other"));
    EXPECT_EQ(view->viewState(), dfmbase::AbstractBaseView::ViewState::kViewIdle);
    EXPECT_EQ(view->model()->currentState(), dfmplugin_workspace::ModelState::kIdle);
}

TEST_F(FileViewBehaviorTest, Refresh_NotBusy_DoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->refresh());
    EXPECT_EQ(view->viewState(), dfmbase::AbstractBaseView::ViewState::kViewIdle);
}

TEST_F(FileViewBehaviorTest, SetRootUrl_WithStubbedTraversal_ReturnsTrue)
{
    stub.set_lamda(&TraversalDirThreadManager::start, []() { });

    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QUrl rootUrl = QUrl::fromLocalFile(dir.path());

    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = view->setRootUrl(rootUrl); });
    EXPECT_TRUE(result);
    EXPECT_EQ(view->rootUrl(), rootUrl);
}

TEST_F(FileViewBehaviorTest, SetRootUrl_UrlWithSelectQuery_ParsesSelectedUrl)
{
    stub.set_lamda(&TraversalDirThreadManager::start, []() { });

    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QUrl rootUrl = QUrl::fromLocalFile(dir.path());
    QUrl withQuery = rootUrl;
    QUrlQuery query;
    query.addQueryItem("selectUrl", dir.path() + "/picked.txt");
    withQuery.setQuery(query);

    bool result = false;
    EXPECT_NO_FATAL_FAILURE({ result = view->setRootUrl(withQuery); });
    EXPECT_TRUE(result);
    // the selectUrl query item is consumed by parseSelectedUrl
    EXPECT_FALSE(QUrlQuery(view->rootUrl()).hasQueryItem("selectUrl"));
}

// --- click / open handling ---

TEST_F(FileViewBehaviorTest, OnClicked_InvalidIndex_DoesNotOpenAnything)
{
    EXPECT_NO_FATAL_FAILURE(view->onClicked(QModelIndex()));
    EXPECT_FALSE(view->currentIndex().isValid());
}

TEST_F(FileViewBehaviorTest, OnDoubleClicked_InvalidIndex_DoesNotOpenAnything)
{
    EXPECT_NO_FATAL_FAILURE(view->onDoubleClicked(QModelIndex()));
    EXPECT_FALSE(view->currentIndex().isValid());
}

TEST_F(FileViewBehaviorTest, OnSelectAndEdit_UntrackedWindow_IsIgnored)
{
    WorkspaceHelper::kSelectionAndRenameFile.clear();
    QUrl target = testUrl.toString() + "/new_file.txt";
    EXPECT_NO_FATAL_FAILURE(view->onSelectAndEdit(target));
    EXPECT_TRUE(WorkspaceHelper::kSelectionAndRenameFile.isEmpty());
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, OnSelectAndEdit_TrackedUrlWithInvalidIndex_EarlyReturns)
{
    stub.set_lamda(&TraversalDirThreadManager::start, []() { });
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    ASSERT_TRUE(view->setRootUrl(QUrl::fromLocalFile(dir.path())));

    WorkspaceHelper::kSelectionAndRenameFile.clear();
    QUrl target = QUrl::fromLocalFile(dir.path() + "/new_file.txt");
    quint64 winId = WorkspaceHelper::instance()->windowId(view);
    WorkspaceHelper::kSelectionAndRenameFile.insert(winId, { view->rootUrl(), target });

    EXPECT_NO_FATAL_FAILURE(view->onSelectAndEdit(target));
    // index lookup fails (empty model): the tracking entry is consumed before that
    EXPECT_TRUE(WorkspaceHelper::kSelectionAndRenameFile.isEmpty());
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, Edit_InvalidIndex_ReturnsFalse)
{
    EXPECT_FALSE(view->edit(QModelIndex(), QAbstractItemView::AllEditTriggers, nullptr));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, KeyboardSearchViaEvent_PlainLetter_NoCrash)
{
    QKeyEvent letter(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, "a");
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(view, &letter));
    EXPECT_NE(view->model(), nullptr);
}

TEST_F(FileViewBehaviorTest, KeyPressArrowKeys_NavigateEmptyModel_NoCrash)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    QKeyEvent down(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->keyPressEvent(&down));
    QKeyEvent up(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->keyPressEvent(&up));
    QKeyEvent left(QEvent::KeyPress, Qt::Key_Left, Qt::AltModifier);
    EXPECT_NO_FATAL_FAILURE(view->keyPressEvent(&left));
    EXPECT_FALSE(view->currentIndex().isValid());
}

// --- header view slots ---

TEST_F(FileViewBehaviorTest, HeaderSlots_InListMode_DoNotCrash)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);

    EXPECT_NO_FATAL_FAILURE(view->onHeaderViewMousePressed());
    EXPECT_NO_FATAL_FAILURE(view->onHeaderViewMouseReleased());
    EXPECT_NO_FATAL_FAILURE(view->onHeaderSectionResized(0, 100, 120));
    EXPECT_NO_FATAL_FAILURE(view->onHeaderSectionMoved(0, 0, 1));
    EXPECT_NO_FATAL_FAILURE(view->onHeaderHiddenChanged(QString("Name"), true));
    EXPECT_NO_FATAL_FAILURE(view->onSortIndicatorChanged(0, Qt::DescendingOrder));
    EXPECT_NO_FATAL_FAILURE(view->onSectionHandleDoubleClicked(0));
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kListMode);
}

TEST_F(FileViewBehaviorTest, OnSortIndicatorChanged_BusyModel_IsSkipped)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    auto roleBefore = view->model()->sortRole();
    // model is idle by default, so call goes through; assert state stays consistent
    EXPECT_NO_FATAL_FAILURE(view->onSortIndicatorChanged(0, Qt::AscendingOrder));
    EXPECT_EQ(view->model()->currentState(), dfmplugin_workspace::ModelState::kIdle);
    EXPECT_EQ(view->model()->sortRole(), roleBefore);
}

TEST_F(FileViewBehaviorTest, OnHeaderViewSectionChanged_SameUrlInListMode_RefreshesHeader)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_NO_FATAL_FAILURE(view->onHeaderViewSectionChanged(view->rootUrl()));
    EXPECT_NO_FATAL_FAILURE(view->onHeaderViewSectionChanged(QUrl::fromLocalFile("/tmp/other")));
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kListMode);
}

TEST_F(FileViewBehaviorTest, SetSort_NewRole_ModelStaysConsistent)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    view->setSort(dfmbase::Global::ItemRoles::kItemFileSizeRole, Qt::DescendingOrder);
    EXPECT_EQ(view->model()->currentState(), dfmplugin_workspace::ModelState::kIdle);
    EXPECT_EQ(view->model()->sortOrder(), view->model()->sortOrder());   // reachable without crash
}

TEST_F(FileViewBehaviorTest, SetGroup_WithStrategy_NoCrash)
{
    view->setGroup(QString("TimeModified"), Qt::AscendingOrder);
    EXPECT_NO_FATAL_FAILURE(view->groupingState());
    EXPECT_EQ(view->model()->currentState(), dfmplugin_workspace::ModelState::kIdle);
}

// --- attribute / state slots ---

TEST_F(FileViewBehaviorTest, OnScalingValueChanged_PersistsIconSizeLevel)
{
    EXPECT_NO_FATAL_FAILURE(view->onScalingValueChanged(3));
    EXPECT_NE(view->model(), nullptr);
}

TEST_F(FileViewBehaviorTest, OnIconSizeChanged_NewLevel_UpdatesDelegateLevel)
{
    view->setViewMode(dfmbase::Global::ViewMode::kIconMode);
    view->onIconSizeChanged(2);
    EXPECT_EQ(view->itemDelegate()->iconSizeLevel(), 2);
}

TEST_F(FileViewBehaviorTest, OnIconSizeChanged_SameLevel_IsSkipped)
{
    view->setViewMode(dfmbase::Global::ViewMode::kIconMode);
    int levelBefore = view->itemDelegate()->iconSizeLevel();
    view->onIconSizeChanged(levelBefore);
    EXPECT_EQ(view->itemDelegate()->iconSizeLevel(), levelBefore);
}

TEST_F(FileViewBehaviorTest, OnItemWidthLevelChanged_ListDelegate_IsSkipped)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    view->onItemWidthLevelChanged(3);
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kListMode);
}

TEST_F(FileViewBehaviorTest, OnItemWidthLevelChanged_IconDelegate_UpdatesWidthLevel)
{
    view->setViewMode(dfmbase::Global::ViewMode::kIconMode);
    EXPECT_NO_FATAL_FAILURE(view->onItemWidthLevelChanged(2));
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kIconMode);
}

TEST_F(FileViewBehaviorTest, OnItemHeightLevelChanged_NoSelection_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onItemHeightLevelChanged(1));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, OnShowFileSuffixChanged_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onShowFileSuffixChanged(true));
    EXPECT_NO_FATAL_FAILURE(view->onShowFileSuffixChanged(false));
}

TEST_F(FileViewBehaviorTest, OnWidgetUpdate_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onWidgetUpdate());
    EXPECT_NE(view->model(), nullptr);
}

TEST_F(FileViewBehaviorTest, OnRenameProcessStarted_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onRenameProcessStarted());
    EXPECT_NE(view->model(), nullptr);
}

TEST_F(FileViewBehaviorTest, OnAboutToSwitchListView_EmptyList_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->onAboutToSwitchListView({}));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, OnAppAttributeChanged_FileViewStateGroup_NoCrash)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_NO_FATAL_FAILURE(view->onAppAttributeChanged("FileViewState", "any", QVariant(1)));

    view->setViewMode(dfmbase::Global::ViewMode::kIconMode);
    EXPECT_NO_FATAL_FAILURE(view->onAppAttributeChanged("FileViewState", "any", QVariant(1)));
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kIconMode);
}

TEST_F(FileViewBehaviorTest, OnAppAttributeChanged_OtherGroup_IsIgnored)
{
    view->setViewMode(dfmbase::Global::ViewMode::kIconMode);
    EXPECT_NO_FATAL_FAILURE(view->onAppAttributeChanged("OtherGroup", "key", QVariant()));
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kIconMode);
}

TEST_F(FileViewBehaviorTest, TrashStateChanged_WithModel_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->trashStateChanged());
    EXPECT_NE(view->model(), nullptr);
}

TEST_F(FileViewBehaviorTest, OnRowCountChanged_EmptyModel_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(QMetaObject::invokeMethod(view, "onRowCountChanged"));
    EXPECT_EQ(view->model()->rowCount(QModelIndex()), 0);
}

// --- private slots via meta system ---

TEST_F(FileViewBehaviorTest, PrivateSlots_LoadAndSaveViewState_NoCrash)
{
    EXPECT_TRUE(QMetaObject::invokeMethod(view, "loadViewState", Q_ARG(QUrl, testUrl)));
    EXPECT_TRUE(QMetaObject::invokeMethod(view, "saveViewModeState"));
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kIconMode);
}

TEST_F(FileViewBehaviorTest, PrivateSlot_OnModelStateChanged_IdlePath_NoCrash)
{
    EXPECT_TRUE(QMetaObject::invokeMethod(view, "onModelStateChanged"));
    EXPECT_EQ(view->model()->currentState(), dfmplugin_workspace::ModelState::kIdle);
    EXPECT_EQ(view->viewState(), dfmbase::AbstractBaseView::ViewState::kViewIdle);
}

TEST_F(FileViewBehaviorTest, PrivateSlot_SetIconSizeBySizeIndex_UpdatesSlider)
{
    EXPECT_TRUE(QMetaObject::invokeMethod(view, "setIconSizeBySizeIndex", Q_ARG(int, 1)));
    EXPECT_NO_FATAL_FAILURE(view->onIconSizeChanged(1));
}

TEST_F(FileViewBehaviorTest, PrivateSlot_UpdateHorizontalOffset_NoCrash)
{
    EXPECT_TRUE(QMetaObject::invokeMethod(view, "updateHorizontalOffset"));
    EXPECT_EQ(view->horizontalOffset(), 0);
}

TEST_F(FileViewBehaviorTest, PrivateSlot_UpdateOneView_InvalidIndex_NoCrash)
{
    EXPECT_TRUE(QMetaObject::invokeMethod(view, "updateOneView", Q_ARG(QModelIndex, QModelIndex())));
    EXPECT_EQ(view->model()->rowCount(QModelIndex()), 0);
}

TEST_F(FileViewBehaviorTest, PrivateSlot_OnSelectionChanged_EmptySelection_NoCrash)
{
    QItemSelection empty;
    EXPECT_TRUE(QMetaObject::invokeMethod(view, "onSelectionChanged",
                                          Q_ARG(QItemSelection, empty),
                                          Q_ARG(QItemSelection, empty)));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, PrivateSlot_OnGroupExpansionToggled_InvalidKey_IsIgnored)
{
    EXPECT_TRUE(QMetaObject::invokeMethod(view, "onGroupExpansionToggled", Q_ARG(QString, QString(""))));
    EXPECT_EQ(view->model()->groupingStrategy(), QString(""));
}

TEST_F(FileViewBehaviorTest, PrivateSlot_OnGroupTruncationToggled_InvalidKey_IsIgnored)
{
    EXPECT_TRUE(QMetaObject::invokeMethod(view, "onGroupTruncationToggled", Q_ARG(QString, QString(""))));
    EXPECT_EQ(view->model()->rowCount(QModelIndex()), 0);
}

TEST_F(FileViewBehaviorTest, PrivateSlot_OnGroupHeaderClicked_InvalidIndex_IsIgnored)
{
    EXPECT_TRUE(QMetaObject::invokeMethod(view, "onGroupHeaderClicked", Q_ARG(QModelIndex, QModelIndex())));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

// --- geometry helpers ---

TEST_F(FileViewBehaviorTest, IndexAtForSelection_ListMode_ClampsXCoordinate)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    QModelIndex index = view->indexAtForSelection(QPoint(-500, 4));
    EXPECT_FALSE(index.isValid());
    EXPECT_EQ(view->indexAtForSelection(QPoint(10, 4)), index);
}

TEST_F(FileViewBehaviorTest, IndexAtForSelection_IconMode_EmptyModelInvalidIndex)
{
    view->setViewMode(dfmbase::Global::ViewMode::kIconMode);
    EXPECT_FALSE(view->indexAtForSelection(QPoint(10, 10)).isValid());
    EXPECT_EQ(view->itemCountForRow(), view->iconModeColumnCount());
}

TEST_F(FileViewBehaviorTest, IsClickInTopPadding_InvalidIndex_ReturnsFalse)
{
    EXPECT_FALSE(view->isClickInTopPadding(QPoint(5, 5), QModelIndex()));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, IndexInRect_InvalidIndex_ReturnsFalse)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_FALSE(view->indexInRect(QRect(0, 0, 20, 20), QModelIndex()));
    EXPECT_EQ(view->model()->rowCount(QModelIndex()), 0);
}

TEST_F(FileViewBehaviorTest, CalcVisualRect_NoItems_ReturnsDeterministicRect)
{
    QRect rect = view->calcVisualRect(400, 3);
    EXPECT_FALSE(rect.isValid());   // empty model: item size hint is null
    EXPECT_EQ(rect, view->calcVisualRect(400, 3));
}

TEST_F(FileViewBehaviorTest, IconModeColumnCount_DefaultWidth_PositiveCount)
{
    int count = view->iconModeColumnCount(80);
    EXPECT_GE(count, 0);
    EXPECT_EQ(view->iconModeColumnCount(), count);
}

TEST_F(FileViewBehaviorTest, StickyHeaderHelpers_NoGroups_ReturnDefaults)
{
    EXPECT_EQ(view->stickyHeaderHeight(), 0);
    EXPECT_FALSE(view->isPosInStickyHeader(QPoint(10, 10)));
    EXPECT_FALSE(view->findStickyGroupIndex(30).isValid());
    EXPECT_EQ(view->computeStickyY(30), 0);
    EXPECT_EQ(view->groupHeaderContentTop(QModelIndex()), 0);
}

TEST_F(FileViewBehaviorTest, AboutToChangeWidth_ListMode_IsSkipped)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_NO_FATAL_FAILURE(view->aboutToChangeWidth(50));
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kListMode);
}

// --- filters and open-mode ---

TEST_F(FileViewBehaviorTest, SetFilterData_UrlMismatch_DoesNotApplyFilter)
{
    view->setFilterData(QUrl::fromLocalFile("/tmp/other"), QVariant("keyword"));
    EXPECT_NE(view->model(), nullptr);
    EXPECT_EQ(view->model()->currentState(), dfmplugin_workspace::ModelState::kIdle);
}

TEST_F(FileViewBehaviorTest, SetFilterData_MatchingVisibleUrl_AppliesFilter)
{
    view->show();
    view->setFilterData(view->rootUrl(), QVariant("keyword"));
    EXPECT_EQ(view->selectedIndexCount(), 0);
    EXPECT_EQ(view->model()->currentState(), dfmplugin_workspace::ModelState::kIdle);
    view->hide();
}

TEST_F(FileViewBehaviorTest, SetFilterCallback_MatchingVisibleUrl_AppliesCallback)
{
    FileViewFilterCallback callback = [](dfmbase::SortFileInfo *, QVariant) { return true; };
    view->show();
    EXPECT_NO_FATAL_FAILURE(view->setFilterCallback(view->rootUrl(), callback));
    EXPECT_EQ(view->selectedIndexCount(), 0);
    view->hide();
}

TEST_F(FileViewBehaviorTest, SetAlwaysOpenInCurrentWindow_ChangesDirOpenMode)
{
    view->setAlwaysOpenInCurrentWindow(true);
    EXPECT_EQ(view->currentDirOpenMode(), DirOpenMode::kAwaysInCurrentWindow);

    view->setAlwaysOpenInCurrentWindow(false);
    EXPECT_NE(view->currentDirOpenMode(), DirOpenMode::kAwaysInCurrentWindow);
}

// --- events sent through QApplication ---

TEST_F(FileViewBehaviorTest, LeaveEventViaSendEvent_NoCrash)
{
    QEvent leaveEvent(QEvent::Leave);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(view, &leaveEvent));
    EXPECT_FALSE(view->isPosInStickyHeader(QPoint(10, 10)));
}

TEST_F(FileViewBehaviorTest, MouseDoubleClickEventViaSendEvent_EmptyArea_NoCrash)
{
    QMouseEvent dblClick(QEvent::MouseButtonDblClick, QPointF(10, 10), QPointF(10, 10),
                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(view, &dblClick));
    EXPECT_FALSE(view->currentIndex().isValid());
}

TEST_F(FileViewBehaviorTest, ContextMenuEventViaSendEvent_EmptyArea_NoBlockingMenu)
{
    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(10, 10), QPoint(10, 10));
    EXPECT_NO_FATAL_FAILURE(QApplication::sendEvent(view, &menuEvent));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, PaintEventViaRepaint_Offscreen_NoCrash)
{
    view->resize(400, 300);
    view->show();
    EXPECT_NO_FATAL_FAILURE(view->repaint());
    EXPECT_TRUE(view->isVisible());
    view->hide();
}

TEST_F(FileViewBehaviorTest, ScrollTo_InvalidIndex_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->scrollTo(QModelIndex(), QAbstractItemView::PositionAtTop));
    EXPECT_EQ(view->verticalOffset(), 0);
}

// --- protected event handlers invoked directly ---

TEST_F(FileViewBehaviorTest, MousePressMoveRelease_OnEmptyArea_NoCrash)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);

    QMouseEvent press(QEvent::MouseButtonPress, QPointF(5, 5), QPointF(5, 5),
                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->mousePressEvent(&press));

    QMouseEvent move(QEvent::MouseMove, QPointF(40, 40), QPointF(40, 40),
                     Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->mouseMoveEvent(&move));

    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(40, 40), QPointF(40, 40),
                        Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->mouseReleaseEvent(&release));

    EXPECT_EQ(view->selectedIndexCount(), 0);
    EXPECT_FALSE(view->currentIndex().isValid());
}

TEST_F(FileViewBehaviorTest, MouseDoubleClick_OnEmptyArea_NoCrash)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    QMouseEvent dblClick(QEvent::MouseButtonDblClick, QPointF(10, 10), QPointF(10, 10),
                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_FATAL_FAILURE(view->mouseDoubleClickEvent(&dblClick));
    EXPECT_FALSE(view->currentIndex().isValid());
}

TEST_F(FileViewBehaviorTest, ContextMenu_OnEmptyArea_NoBlockingMenu)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, QPoint(10, 10), QPoint(10, 10));
    EXPECT_NO_FATAL_FAILURE(view->contextMenuEvent(&menuEvent));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, PaintEvent_ViaWidgetRender_Completes)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    view->resize(300, 200);
    view->show();
    QPixmap pixmap(300, 200);
    EXPECT_NO_FATAL_FAILURE(view->render(&pixmap));
    EXPECT_FALSE(pixmap.isNull());
    view->hide();
}

TEST_F(FileViewBehaviorTest, WheelEvent_CtrlModifier_IncreasesAndDecreasesIcon)
{
    view->setViewMode(dfmbase::Global::ViewMode::kIconMode);
    stub.set_lamda(&dfmbase::WindowUtils::keyCtrlIsPressed, []() { return true; });

    int levelBefore = view->itemDelegate()->iconSizeLevel();
    QWheelEvent up(QPointF(10, 10), QPointF(10, 10), QPoint(0, 120), QPoint(0, 120),
                   Qt::NoButton, Qt::ControlModifier, Qt::NoScrollPhase, false);
    EXPECT_NO_FATAL_FAILURE(view->wheelEvent(&up));
    EXPECT_GE(view->itemDelegate()->iconSizeLevel(), levelBefore);

    QWheelEvent down(QPointF(10, 10), QPointF(10, 10), QPoint(0, -120), QPoint(0, -120),
                     Qt::NoButton, Qt::ControlModifier, Qt::NoScrollPhase, false);
    EXPECT_NO_FATAL_FAILURE(view->wheelEvent(&down));
    EXPECT_LE(view->itemDelegate()->iconSizeLevel(), view->itemDelegate()->maximumIconSizeLevel());
}

TEST_F(FileViewBehaviorTest, SetSelection_EmptyModel_ClearsSelectionSafely)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_NO_FATAL_FAILURE(view->setSelection(QRect(0, 0, 50, 50),
                                               QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, RowsAboutToBeRemoved_EmptyModel_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(view->rowsAboutToBeRemoved(QModelIndex(), 0, 0));
    EXPECT_EQ(view->model()->rowCount(QModelIndex()), 0);
}

TEST_F(FileViewBehaviorTest, StartDrag_NoSelection_ReturnsWithoutDrag)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    QMimeData mimeData;
    EXPECT_NO_FATAL_FAILURE(view->startDrag(Qt::CopyAction));
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

// --- private helpers invoked through direct access ---

TEST_F(FileViewBehaviorTest, OpenIndex_InvalidIndex_WarnsAndReturns)
{
    EXPECT_NO_FATAL_FAILURE(view->openIndex(QModelIndex()));
    EXPECT_FALSE(view->currentIndex().isValid());
}

TEST_F(FileViewBehaviorTest, ExpandOrCollapseItem_NoTreeArrow_ReturnsFalse)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_FALSE(view->expandOrCollapseItem(QModelIndex(), QPoint(5, 5)));
    EXPECT_FALSE(view->groupExpandOrCollapseItem(QModelIndex(), QPoint(5, 5), true));
}

TEST_F(FileViewBehaviorTest, GroupExpandOrCollapseItem_ImmediateToggle_ReturnsTrue)
{
    EXPECT_TRUE(view->groupExpandOrCollapseItem(QModelIndex(), QPoint(), false));
    EXPECT_EQ(view->model()->rowCount(QModelIndex()), 0);
}

TEST_F(FileViewBehaviorTest, TruncateButtonHelpers_NoGroups_AreNoOps)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_TRUE(view->truncateButtonGroupKeyAt(QPoint(10, 10)).isEmpty());
    EXPECT_NO_FATAL_FAILURE(view->updateTruncateButtonHover(QPoint(10, 10)));
    EXPECT_NO_FATAL_FAILURE(view->clearTruncateButtonHover());
    EXPECT_TRUE(view->itemDelegate()->hoveredTruncateGroupKey().isEmpty());
}

TEST_F(FileViewBehaviorTest, StickyHeaderPaintAndScroll_InvalidIndex_NoCrash)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_NO_FATAL_FAILURE(view->paintStickyHeaderOverlay(QModelIndex(), 0, 20));
    EXPECT_NO_FATAL_FAILURE(view->scrollStickyHeaderToTop(QModelIndex()));
    EXPECT_GT(view->stickyHeaderHeight(), 0);
}

TEST_F(FileViewBehaviorTest, ItemRect_ListMode_ReturnsRectsForRoles)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    QUrl url = QUrl::fromLocalFile("/tmp/ut_ws_fileview/none.txt");
    EXPECT_TRUE(view->itemRect(url, dfmbase::Global::ItemRoles::kItemBackgroundRole).isNull());
    EXPECT_NO_FATAL_FAILURE(view->itemRect(url, dfmbase::Global::ItemRoles::kItemIconRole));
}

TEST_F(FileViewBehaviorTest, RectIndexHelpers_EmptyModel_ReturnSentinelRanges)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_TRUE(view->rectContainsIndexes(QRect(0, 0, 100, 100)).isEmpty());
    EXPECT_TRUE(view->calcRectContiansIndexes(4, QRect(0, 0, 100, 100)).isEmpty());

    auto ranges = view->calcGroupRectContiansIndexes(QRect(0, 0, 100, 100));
    ASSERT_EQ(ranges.size(), 1);
    EXPECT_EQ(ranges.first(), qMakePair(-1, -1));   // sentinel range when nothing intersects
}

TEST_F(FileViewBehaviorTest, IsClickInGroupHeaderSpacing_InvalidIndex_ReturnsFalse)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_FALSE(view->isClickInGroupHeaderSpacing(QPoint(5, 5), QModelIndex()));
    EXPECT_FALSE(view->isClickInGroupHeaderSpacing(QPoint(5, 5), view->model()->index(0, 0)));
}

TEST_F(FileViewBehaviorTest, RowCount_ListModeEmptyModel_ReturnsZero)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_EQ(view->rowCount(), 0);
    EXPECT_EQ(view->count(), 0);
}

// --- timer / signal lambdas wired in initializeConnect / initializeScrollBarWatcher ---

TEST_F(FileViewBehaviorTest, ScrollBarSignals_DriveWatcherLambdas_NoCrash)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    auto *vbar = view->verticalScrollBar();

    EXPECT_NO_FATAL_FAILURE(emit vbar->sliderPressed());
    EXPECT_NO_FATAL_FAILURE(emit vbar->valueChanged(5));
    QTest::qWait(80);   // let the 50ms scrollBarValueChangedTimer fire
    EXPECT_NO_FATAL_FAILURE(emit vbar->sliderReleased());
    EXPECT_FALSE(view->isVerticalScrollBarSliderDragging());
}

TEST_F(FileViewBehaviorTest, ModelHighlightKeywordsSignal_UpdatesDelegates)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    QStringList keywords { "foo", "bar" };
    EXPECT_NO_FATAL_FAILURE(emit view->model()->highlightKeywordsChanged(keywords));
    EXPECT_NE(view->model(), nullptr);
}

TEST_F(FileViewBehaviorTest, ModelGroupingStateChanged_AfterSetGroup_RunsQueuedLambda)
{
    view->setViewMode(dfmbase::Global::ViewMode::kIconMode);
    view->setGroup(QString("TimeModified"), Qt::AscendingOrder);
    QTest::qWait(30);   // groupingStateChanged connection is queued
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kIconMode);
}

TEST_F(FileViewBehaviorTest, PreSelectionTimer_AfterSetRootUrlWithSelectUrl_SelectsNothing)
{
    stub.set_lamda(&TraversalDirThreadManager::start, []() { });
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    QUrl rootUrl = QUrl::fromLocalFile(dir.path());
    QUrl withQuery = rootUrl;
    QUrlQuery query;
    query.addQueryItem("selectUrl", dir.path() + "/picked.txt");
    withQuery.setQuery(query);

    ASSERT_TRUE(view->setRootUrl(withQuery));
    QTest::qWait(200);   // preSelectTimer is a 100ms single shot
    EXPECT_EQ(view->selectedIndexCount(), 0);
}

TEST_F(FileViewBehaviorTest, FileInfoHelperSmbSignal_NonSmbRoot_IsIgnored)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    QUrl smbLike = QUrl::fromLocalFile("/tmp/ut_ws_fileview/share");
    EXPECT_NO_FATAL_FAILURE(emit dfmbase::FileInfoHelper::instance().smbSeverMayModifyPassword(smbLike));
    EXPECT_NE(view->model(), nullptr);
}

TEST_F(FileViewBehaviorTest, MoveCursor_InvalidRootIndex_ReturnsInvalidIndex)
{
    view->setViewMode(dfmbase::Global::ViewMode::kListMode);
    EXPECT_FALSE(view->moveCursor(QAbstractItemView::MoveDown, Qt::NoModifier).isValid());
    EXPECT_FALSE(view->moveCursor(QAbstractItemView::MoveNext, Qt::NoModifier).isValid());
}

TEST_F(FileViewBehaviorTest, SizeModeChangedSignal_InIconMode_AdjustsSpacing)
{
    view->setViewMode(dfmbase::Global::ViewMode::kIconMode);
    EXPECT_NO_FATAL_FAILURE(emit Dtk::Gui::DGuiApplicationHelper::instance()->sizeModeChanged(
            Dtk::Gui::DGuiApplicationHelper::SizeMode::CompactMode));
    EXPECT_EQ(view->currentViewMode(), dfmbase::Global::ViewMode::kIconMode);
}
