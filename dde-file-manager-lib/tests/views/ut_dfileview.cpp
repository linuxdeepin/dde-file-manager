#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QStandardPaths>
#include <QLabel>
#include <QTest>
#include <QKeyEvent>
#include <QMouseEvent>

#define private public
#include "views/dfileview.h"
#include "views/fileviewhelper.h"
#include "interfaces/dlistitemdelegate.h"

#include "views/dfileview.cpp"
namespace  {
    class DFileViewTest : public testing::Test
    {
    public:        
        virtual void SetUp() override
        {
            m_view = new DFileView;
            m_fileViewHelper = m_view->fileViewHelper();
        }

        virtual void TearDown() override
        {
            delete m_view;
            m_view = nullptr;
        }
        DFileView *m_view;
        FileViewHelper *m_fileViewHelper;
    };
}

TEST_F(DFileViewTest,get_the_model)
{
    ASSERT_NE(nullptr,m_view);

    DFileSystemModel *model = m_view->model();
    EXPECT_NE(nullptr, model);
}

TEST_F(DFileViewTest,get_item_delegate)
{
    ASSERT_NE(nullptr,m_view);

    DFMStyledItemDelegate *delegate = m_view->itemDelegate();
    EXPECT_NE(nullptr, delegate);
}

TEST_F(DFileViewTest,set_item_delegate)
{
    ASSERT_NE(nullptr,m_view);

    DListItemDelegate delegate(m_fileViewHelper);
    m_view->setItemDelegate(&delegate);
    DFMStyledItemDelegate *result = m_view->itemDelegate();
    EXPECT_EQ(&delegate, result);
}

TEST_F(DFileViewTest,get_status_bar)
{
    ASSERT_NE(nullptr,m_view);

    DStatusBar *result = m_view->statusBar();
    EXPECT_EQ(result, m_view->d_func()->statusBar);
}

TEST_F(DFileViewTest,get_file_helper)
{
    ASSERT_NE(nullptr,m_view);

    FileViewHelper *result = m_view->fileViewHelper();
    EXPECT_EQ(result, m_view->d_func()->fileViewHelper);
}

TEST_F(DFileViewTest,get_root_url)
{
    ASSERT_NE(nullptr,m_view);

    DUrl result = m_view->rootUrl();
    EXPECT_EQ(result, m_view->model()->rootUrl());
}

TEST_F(DFileViewTest,get_view_state)
{
    ASSERT_NE(nullptr,m_view);

    DFMBaseView::ViewState result = m_view->viewState();
    DFMBaseView::ViewState expectResult = m_view->model()->state() == DFileSystemModel::Busy ? DFMBaseView::ViewBusy : DFMBaseView::ViewIdle;
    EXPECT_EQ(result, expectResult);
}

TEST_F(DFileViewTest,get_selected_urls)
{
    ASSERT_NE(nullptr,m_view);

    m_view->select({});
    QList<DUrl> result = m_view->selectedUrls();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(DFileViewTest,get_icon_mode)
{
    ASSERT_NE(nullptr,m_view);

    bool result = m_view->isIconViewMode();
    bool expectResult = m_view->d_func()->currentViewMode == DFileView::IconMode;
    EXPECT_EQ(result, expectResult);
}

TEST_F(DFileViewTest,get_column_width)
{
    ASSERT_NE(nullptr,m_view);

    int result = m_view->columnWidth(0);
    int expectResult = 100;
    if (m_view->d_func()->headerView) {
        expectResult = m_view->d_func()->headerView->sectionSize(0);
    }
    EXPECT_EQ(result, expectResult);
}

TEST_F(DFileViewTest,set_column_width)
{
    ASSERT_NE(nullptr,m_view);

    m_view->setColumnWidth(0, 100);
    int result = m_view->columnWidth(0);
    EXPECT_EQ(result, 100);
}

TEST_F(DFileViewTest,get_column_count)
{
    ASSERT_NE(nullptr,m_view);

    int result = m_view->columnCount();
    int expectResult = 1;
    if (m_view->d_func()->headerView) {
        expectResult = m_view->d_func()->headerView->count();
    }
    EXPECT_EQ(result, expectResult);
}

TEST_F(DFileViewTest,get_row_count)
{
    ASSERT_NE(nullptr,m_view);

    int result = m_view->rowCount();

    int tmpCount = m_view->count();
    int tmpItemCountForRow = m_view->itemCountForRow();
    int expectResult = tmpCount / tmpItemCountForRow + int(tmpCount % tmpItemCountForRow > 0);

    EXPECT_EQ(result, expectResult);
}

TEST_F(DFileViewTest,get_itemcount_row)
{
    ASSERT_NE(nullptr,m_view);

    int result = m_view->itemCountForRow();
    int expectResult = 1;
    if (m_view->isIconViewMode()) {
        expectResult = m_view->d_func()->iconModeColumnCount();
    }
    EXPECT_EQ(result, expectResult);
}

TEST_F(DFileViewTest,get_index_row)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndex index;
    int result = m_view->indexOfRow(index);
    int expectResult = index.row() / m_view->itemCountForRow();
    EXPECT_EQ(result, expectResult);
}

TEST_F(DFileViewTest,get_columnrole_list)
{
    ASSERT_NE(nullptr,m_view);

    QList<int> result = m_view->columnRoleList();
    EXPECT_EQ(result, m_view->d_func()->columnRoles);
}

TEST_F(DFileViewTest,get_window_id)
{
    ASSERT_NE(nullptr,m_view);

    quint64 result = m_view->windowId();
    EXPECT_EQ(result, WindowManager::getWindowId(m_view));
}

TEST_F(DFileViewTest,set_icon_size)
{
    ASSERT_NE(nullptr,m_view);

    QSize size(50,50);
    m_view->setIconSize(size);
    QSize result = m_view->iconSize();
    EXPECT_EQ(size, result);
}

TEST_F(DFileViewTest,get_default_mode)
{
    ASSERT_NE(nullptr,m_view);

    DFileView::ViewMode mode = m_view->getDefaultViewMode();
    EXPECT_EQ(mode, m_view->d_func()->defaultViewMode);
}

TEST_F(DFileViewTest,get_view_mode)
{
    ASSERT_NE(nullptr,m_view);

    DFileView::ViewMode mode = m_view->viewMode();
    EXPECT_EQ(mode, m_view->d_func()->currentViewMode);
}

TEST_F(DFileViewTest,test_view_mode)
{
    ASSERT_NE(nullptr,m_view);

    DFileView::ViewMode mode = DFileView::IconMode;
    DFileView::ViewModes modes = DFileView::ListMode | DFileView::ExtendMode;

    bool result = m_view->testViewMode(modes, mode);
    EXPECT_FALSE(result);

    mode = DFileView::ListMode;
    result = m_view->testViewMode(modes, mode);
    EXPECT_TRUE(result);
}

TEST_F(DFileViewTest,get_horizontal_offset)
{
    ASSERT_NE(nullptr,m_view);

    int result = m_view->horizontalOffset();
    EXPECT_EQ(result, m_view->d_func()->horizontalOffset);
}

TEST_F(DFileViewTest,get_is_selected)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndex index;
    bool result = m_view->isSelected(index);
    EXPECT_FALSE(result);
}

TEST_F(DFileViewTest,get_selectedindex_count)
{
    ASSERT_NE(nullptr,m_view);

    int result = m_view->selectedIndexCount();
    EXPECT_EQ(result, 0);
}

TEST_F(DFileViewTest,get_selected_indexes)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndexList result = m_view->selectedIndexes();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(DFileViewTest,get_index_at)
{
    ASSERT_NE(nullptr,m_view);

    QPoint point(100,100);
    m_view->setViewMode(DFileView::IconMode);

    QModelIndex result = m_view->indexAt(point);
    EXPECT_FALSE(result.isValid());

    m_view->setViewMode(DFileView::ListMode);
    result = m_view->indexAt(point);
    EXPECT_FALSE(result.isValid());
}

TEST_F(DFileViewTest,get_visible_indexes)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndex index;
    QRect rect = m_view->visualRect(index);
    DFileView::RandeIndexList result = m_view->visibleIndexes(rect);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(DFileViewTest,get_item_size)
{
    ASSERT_NE(nullptr,m_view);

    QSize result = m_view->itemSizeHint();
    EXPECT_TRUE(result.isValid());
}

TEST_F(DFileViewTest,get_is_droptarget)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndex index;
    bool result = m_view->isDropTarget(index);
    bool expectResult = m_view->d_func()->dragMoveHoverIndex == index;
    EXPECT_EQ(result, expectResult);
}

TEST_F(DFileViewTest,get_name_filters)
{
    ASSERT_NE(nullptr,m_view);

    QStringList result = m_view->nameFilters();
    EXPECT_EQ(result, m_view->model()->nameFilters());
}

TEST_F(DFileViewTest,get_all_filters)
{
    ASSERT_NE(nullptr,m_view);

    QDir::Filters result = m_view->filters();
    EXPECT_EQ(result, m_view->model()->filters());
}

TEST_F(DFileViewTest,set_enabled_selectionmodes)
{
    ASSERT_NE(nullptr,m_view);

    QSet<QAbstractItemView::SelectionMode> result;
    m_view->setEnabledSelectionModes(result);

    EXPECT_EQ(result, m_view->d_func()->enabledSelectionModes);
}

TEST_F(DFileViewTest,get_enabled_selectionmodes)
{
    ASSERT_NE(nullptr,m_view);

    QSet<QAbstractItemView::SelectionMode> result = m_view->enabledSelectionModes();
    EXPECT_EQ(result, m_view->d_func()->enabledSelectionModes);
}

TEST_F(DFileViewTest,get_widget)
{
    ASSERT_NE(nullptr,m_view);

    QWidget *result = m_view->widget();
    EXPECT_NE(result, nullptr);
}

TEST_F(DFileViewTest,get_toolbar_action_list)
{
    ASSERT_NE(nullptr,m_view);

    QList<QAction *> result = m_view->toolBarActionList();
    EXPECT_EQ(result, m_view->d_func()->toolbarActionGroup->actions());
}

TEST_F(DFileViewTest,tst_cd)
{
    ASSERT_NE(nullptr,m_view);

    QString path("/test");
    DUrl url(path);
    bool result = m_view->cd(url);

    EXPECT_EQ(false, result);
}

TEST_F(DFileViewTest,tst_cd_up)
{
    ASSERT_NE(nullptr,m_view);

    bool result = m_view->cdUp();
    EXPECT_EQ(false, result);
}

TEST_F(DFileViewTest,get_select)
{
    ASSERT_NE(nullptr,m_view);

    DUrl url(QString("/test"));
    QList<DUrl> list;
    list << url;
    m_view->select(list);
}

TEST_F(DFileViewTest,set_viewMode_list)
{
    ASSERT_NE(nullptr,m_view);

    m_view->setViewModeToList();
    EXPECT_EQ(m_view->d_func()->currentViewMode, DFileView::ListMode);
}

TEST_F(DFileViewTest,set_viewMode_icon)
{
    ASSERT_NE(nullptr,m_view);

    m_view->setViewModeToIcon();
    EXPECT_EQ(m_view->d_func()->currentViewMode, DFileView::IconMode);
}

TEST_F(DFileViewTest,set_viewMode_extend)
{
    ASSERT_NE(nullptr,m_view);

    m_view->setViewModeToExtend();
    EXPECT_EQ(m_view->d_func()->currentViewMode, DFileView::ExtendMode);
}

TEST_F(DFileViewTest,set_default_viewMode)
{
    ASSERT_NE(nullptr,m_view);

    DFileView::ViewMode mode = DFileView::ListMode;
    m_view->setDefaultViewMode(mode);
    EXPECT_EQ(mode, m_view->d_func()->defaultViewMode);
}

TEST_F(DFileViewTest,set_view_mode)
{
    ASSERT_NE(nullptr,m_view);

    DFileView::ViewMode mode = DFileView::ListMode;
    m_view->setViewMode(mode);
    EXPECT_EQ(mode, m_view->d_func()->currentViewMode);
}

TEST_F(DFileViewTest,sort_by_role)
{
    ASSERT_NE(nullptr,m_view);

    m_view->sortByRole(0, Qt::AscendingOrder);
    m_view->sortByRole(1, Qt::DescendingOrder);
}

TEST_F(DFileViewTest,set_name_filters)
{
    ASSERT_NE(nullptr,m_view);

    QStringList nameFilters;
    nameFilters <<".a"<<".o";

    m_view->setNameFilters(nameFilters);
    QStringList result = m_view->nameFilters();
    EXPECT_EQ(result, nameFilters);
}

TEST_F(DFileViewTest,set_all_filters)
{
    ASSERT_NE(nullptr,m_view);

    QDir::Filters filters = QDir::AllEntries;
    m_view->setFilters(filters);
    QDir::Filters result = m_view->filters();
    EXPECT_EQ(result, filters);
}

TEST_F(DFileViewTest,clear_heard_view)
{
    ASSERT_NE(nullptr,m_view);

    m_view->clearHeardView();
    EXPECT_EQ(nullptr, m_view->d_func()->headerView);
}

TEST_F(DFileViewTest,clear_selection)
{
    ASSERT_NE(nullptr,m_view);

    m_view->clearSelection();
}

TEST_F(DFileViewTest,set_content_label)
{
    ASSERT_NE(nullptr,m_view);

    QString text("text");
    m_view->setContentLabel(text);
    EXPECT_EQ(text, m_view->d_func()->contentLabel->text());
}

TEST_F(DFileViewTest,set_menuAction_whitelist)
{
    ASSERT_NE(nullptr,m_view);

    QSet<MenuAction> actionList;
    m_view->setMenuActionWhitelist(actionList);
    EXPECT_TRUE(m_view->d_func()->menuWhitelist.isEmpty());
}
TEST_F(DFileViewTest,set_menuAction_blacklist)
{
    ASSERT_NE(nullptr,m_view);

    QSet<MenuAction> actionList;
    m_view->setMenuActionBlacklist(actionList);
    EXPECT_TRUE(m_view->d_func()->menuWhitelist.isEmpty());
}

TEST_F(DFileViewTest,delay_update_statusbar)
{
    ASSERT_NE(nullptr,m_view);

    m_view->delayUpdateStatusBar();
}

TEST_F(DFileViewTest,update_status_bar)
{
    ASSERT_NE(nullptr,m_view);

    m_view->updateStatusBar();
}

TEST_F(DFileViewTest,open_index_openAction)
{
    ASSERT_NE(nullptr,m_view);

    int action = DFMApplication::instance()->appAttribute(DFMApplication::AA_OpenFileMode).toInt();
    QModelIndex index;
    m_view->openIndexByOpenAction(action, index);
}

TEST_F(DFileViewTest,set_icon_size_index)
{
    ASSERT_NE(nullptr,m_view);

    int sizeIndex = 0;
    m_view->setIconSizeBySizeIndex(sizeIndex);
}

TEST_F(DFileViewTest,set_root_url)
{
    ASSERT_NE(nullptr,m_view);

    DUrl url;
    m_view->setRootUrl(url);

    url = DUrl("/test");
    m_view->setRootUrl(url);

    QString path = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    url = DUrl(path);
    m_view->setRootUrl(url);
}

TEST_F(DFileViewTest,display_action_triggered)
{
    ASSERT_NE(nullptr,m_view);

    QAction action;
    action.setData(MenuAction::IconView);
    m_view->dislpayAsActionTriggered(&action);

    action.setData(MenuAction::ListView);
    m_view->dislpayAsActionTriggered(&action);

    action.setData(MenuAction::ExtendView);
    m_view->dislpayAsActionTriggered(&action);
}

TEST_F(DFileViewTest,sort_action_triggerred)
{
    ASSERT_NE(nullptr,m_view);

    QAction action;
    m_view->sortByActionTriggered(&action);
}

TEST_F(DFileViewTest,open_action_triggered)
{
    ASSERT_NE(nullptr,m_view);

    QAction action;
    m_view->openWithActionTriggered(&action);
}

TEST_F(DFileViewTest,row_count_changed)
{
    ASSERT_NE(nullptr,m_view);

    m_view->onRowCountChanged();
}

TEST_F(DFileViewTest,update_model_index)
{
    ASSERT_NE(nullptr,m_view);

    m_view->updateModelActiveIndex();
}

TEST_F(DFileViewTest,handle_data_changed)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndex topLeft,bottomRight;
    m_view->handleDataChanged(topLeft, bottomRight);
}

TEST_F(DFileViewTest,root_url_deleted)
{
    ASSERT_NE(nullptr,m_view);

    DUrl url;
    m_view->onRootUrlDeleted(url);
}

TEST_F(DFileViewTest,fresh_view)
{
    ASSERT_NE(nullptr,m_view);

    m_view->freshView();
}

TEST_F(DFileViewTest,load_view_state)
{
    ASSERT_NE(nullptr,m_view);

    DUrl url;
    m_view->loadViewState(url);
}

TEST_F(DFileViewTest,save_view_state)
{
    ASSERT_NE(nullptr,m_view);

    m_view->saveViewState();
}

TEST_F(DFileViewTest,sort_indicator_changed)
{
    ASSERT_NE(nullptr,m_view);

    int logicalIndex = 0;
    Qt::SortOrder order = Qt::DescendingOrder;
    m_view->onSortIndicatorChanged(logicalIndex, order);
}

TEST_F(DFileViewTest,drive_optical_changed)
{
    ASSERT_NE(nullptr,m_view);

    QString path("/test");
    m_view->onDriveOpticalChanged(path);
}

TEST_F(DFileViewTest,re_set)
{
    ASSERT_NE(nullptr,m_view);

    m_view->reset();
}

TEST_F(DFileViewTest,set_root_index)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndex index;
    m_view->setRootIndex(index);
}

TEST_F(DFileViewTest,tst_wheel_event)
{
    ASSERT_NE(nullptr,m_view);


}

TEST_F(DFileViewTest,tst_keyPress_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,tst_show_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,mouse_press_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,mouse_move_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,mouse_release_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,focus_in_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,resize_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,context_menu_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,drag_enter_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,drag_move_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,drag_leave_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,tst_drop_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,set_selection)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,move_cursor)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,rows_about_removed)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,row_inserted)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,data_changed)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,tst_event)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,update_geome_tries)
{
    ASSERT_NE(nullptr,m_view);
}

TEST_F(DFileViewTest,event_filter)
{
    ASSERT_NE(nullptr,m_view);
}
