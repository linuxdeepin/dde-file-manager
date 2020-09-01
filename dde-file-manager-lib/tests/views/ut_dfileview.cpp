#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

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


