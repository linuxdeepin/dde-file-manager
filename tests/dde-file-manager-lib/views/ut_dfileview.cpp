/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include <QStandardPaths>
#include <QLabel>
#include <QTest>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QTimer>
#include <QMimeData>
#include <QRect>
#include <QScroller>
#include <QActionGroup>
#include <QFlags>
#include <QFlag>
#include <dfiledragclient.h>
#include <QDialog>
#include <DDialog>
#include "stub.h"
#include "../stub-ext/stubext.h"
#include "interfaces/diconitemdelegate.h"
#include "models/mergeddesktopfileinfo.h"
#include "models/masteredmediafileinfo.h"
#include "interfaces/dfilemenumanager.h"
#include "interfaces/dfileservices.h"
#include "interfaces/dfilemenu.h"
#include "controllers/appcontroller.h"
#include "controllers/vaultcontroller.h"

#define private public
#define protected public

#include "views/dfileview_p.h"
#include "views/dfileview.h"
#include "views/fileviewhelper.h"
#include "interfaces/dlistitemdelegate.h"
#include "views/windowmanager.h"
#include "dfmapplication.h"
#include "dfmsettings.h"

namespace {
    static DFileView *m_view;
    static FileViewHelper *m_fileViewHelper;
    class DFileViewTest : public testing::Test
    {
    public:
        static void SetUpTestCase()
        {
            m_view = new DFileView;
            m_fileViewHelper = m_view->fileViewHelper();
        }
        static void TearDownTestCase()
        {
            delete m_view;
            m_view = nullptr;
        }
        virtual void SetUp() override
        {

        }

        virtual void TearDown() override
        {

        }
    };
    class SelectWorkTest : public testing::Test
    {
    public:
        virtual void SetUp() override
        {
            m_selectWork = new SelectWork;
        }

        virtual void TearDown() override
        {
            delete m_selectWork;
            m_selectWork = nullptr;
        }
        SelectWork *m_selectWork;
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

    auto delegateOld = m_view->itemDelegate();

    DListItemDelegate delegate(m_fileViewHelper);
    m_view->setItemDelegate(&delegate);
    DFMStyledItemDelegate *result = m_view->itemDelegate();
    EXPECT_EQ(&delegate, result);

    m_view->setItemDelegate(delegateOld);
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

    Stub stub;
    QModelIndexList (*ut_selectedIndexs)() = [](){
        QModelIndexList list;
        QModelIndex index1;
        list.append(index1);
        return list;
    };

    typedef QModelIndexList (*fptr)(DFileView*);
    fptr  DFileView_selectedIndexs = (fptr)((&DFileView::selectedIndexes));
    stub.set(DFileView_selectedIndexs, ut_selectedIndexs);

    m_view->select({});
    QList<DUrl> result = m_view->selectedUrls();
    EXPECT_FALSE(result.isEmpty());
    stub.reset(DFileView_selectedIndexs);
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

    stub_ext::StubExt st;
    bool isCalled = false;
    st.set_lamda(ADDR(QHeaderView, resizeSection), [&isCalled](){isCalled = true;});

    m_view->switchViewMode(DFileView::ListMode);
    m_view->setColumnWidth(0, 100);
    EXPECT_TRUE(isCalled);

    isCalled = false;
    m_view->d_func()->headerView = nullptr;
    m_view->setColumnWidth(0, 100);
    EXPECT_FALSE(isCalled);
    st.reset(ADDR(QHeaderView, resizeSection));
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

    m_view->d_func()->currentViewMode = DFileView::IconMode;
    int result = m_view->itemCountForRow();
    int expectResult = m_view->d_func()->iconModeColumnCount();
    EXPECT_EQ(result, expectResult);

    m_view->d_func()->currentViewMode = DFileView::ExtendMode;
    result = m_view->itemCountForRow();
    expectResult = 1;
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

    m_view->setViewMode(DFileView::IconMode);
    {
        Stub stub;
        QModelIndexList (*ut_hasWidgetIndexs)() = [](){
            QModelIndexList list;
            QModelIndex index;
            list.append(index);
            return list;
        };
        typedef QModelIndexList (*fptr)(DIconItemDelegate*);
        fptr DIconItemDelegate_hasWidgetIndexs = (fptr)(&DIconItemDelegate::hasWidgetIndexs);
        stub.set(DIconItemDelegate_hasWidgetIndexs, ut_hasWidgetIndexs);

        int (*ut_spacing)() = [](){return 10000;};
        stub.set(ADDR(QListView, spacing), ut_spacing);
        result = m_view->indexAt(point);
        EXPECT_FALSE(result.isValid());

        stub.reset(DIconItemDelegate_hasWidgetIndexs);
        stub.reset(ADDR(QListView, spacing));
    }

//    point = QPoint(1, 1);
//    result = m_view->indexAt(point);
//    EXPECT_FALSE(result.isValid());

    {
        Stub stub;
        int (*ut_iconModeColumnCount)() = [](){return 0;};
        stub.set(ADDR(DFileViewPrivate, iconModeColumnCount), ut_iconModeColumnCount);
        point = QPoint(100, 100);
        result = m_view->indexAt(point);
        EXPECT_FALSE(result.isValid());

        stub.reset(ADDR(DFileViewPrivate, iconModeColumnCount));
    }

    {
        Stub stub;
        bool (*ut_contains)() = [](){return true;};
        stub.set((bool(QRect::*)(const QPoint &,bool )const )ADDR(QRect, contains), ut_contains);
        result = m_view->indexAt(point);
        EXPECT_FALSE(result.isValid());

        stub.reset((bool(QRect::*)(const QPoint &,bool )const )ADDR(QRect, contains));
    }
}

TEST_F(DFileViewTest,get_visual_rect)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static int myColumn = 1;
    int (*ut_column)() = [](){return myColumn;};
    stub.set(ADDR(QModelIndex, column), ut_column);

    QModelIndex index;
    QRect rect = m_view->visualRect(index);
    EXPECT_EQ(rect.x(), 0);

    myColumn = 0;
    static int myWidth = -1;
    int (*ut_width)() = [](){return myWidth;};
    stub.set(ADDR(QSize, width), ut_width);
    m_view->d_func()->allowedAdjustColumnSize = false;
    rect = m_view->visualRect(index);
    EXPECT_TRUE(rect.isValid());

    myWidth = 10;
    static int myColumnCount = 0;
    int (*ut_iconModeColumnCount)() = [](){return myColumnCount;};
    stub.set(ADDR(DFileViewPrivate, iconModeColumnCount), ut_iconModeColumnCount);
    rect = m_view->visualRect(index);
    EXPECT_TRUE(rect.isEmpty());

    myColumnCount = 10;
    rect = m_view->visualRect(index);
    EXPECT_TRUE(rect.isValid());

    stub.reset(ADDR(QModelIndex, column));
    stub.reset(ADDR(QSize, width));
    stub.reset(ADDR(DFileViewPrivate, iconModeColumnCount));
}

TEST_F(DFileViewTest,get_visible_indexes)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static int myWidth = -1;
    int (*ut_width)() = [](){return myWidth;};
    stub.set(ADDR(QSize, width), ut_width);

    QRect rect(200, 200, 50, 50);
    DFileView::RandeIndexList result = m_view->visibleIndexes(rect);
    EXPECT_FALSE(result.isEmpty());

    myWidth = 50;
    static int myColumnCount = 0;
    int (*ut_iconModeColumnCount)() = [](){return myColumnCount;};
    stub.set(ADDR(DFileViewPrivate, iconModeColumnCount), ut_iconModeColumnCount);
    DFileView::RandeIndexList result2 = m_view->visibleIndexes(rect);
    EXPECT_TRUE(result2.isEmpty());

    myColumnCount = 10;
    static int myCount = 0;
    int (*ut_count)() = [](){return myCount;};
    stub.set(ADDR(DFileView, count), ut_count);
    DFileView::RandeIndexList result3 = m_view->visibleIndexes(rect);
    EXPECT_TRUE(result3.isEmpty());

    myCount = 100;
    DFileView::RandeIndexList result4 = m_view->visibleIndexes(rect);
    EXPECT_TRUE(result4.isEmpty());

    stub.reset(ADDR(QSize, width));
    stub.reset(ADDR(DFileViewPrivate, iconModeColumnCount));
    stub.reset(ADDR(DFileView, count));
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

TEST_F(DFileViewTest,set_destroy_flag)
{
    ASSERT_NE(nullptr,m_view);

    m_view->setDestroyFlag(true);
    EXPECT_TRUE(m_view->m_destroyFlag);

    m_view->setDestroyFlag(false);
    EXPECT_FALSE(m_view->m_destroyFlag);
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

TEST_F(DFileViewTest,tst_edit)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndex index;
    QAbstractItemView::EditTrigger trigger = QAbstractItemView::SelectedClicked;
    QKeyEvent keyPressEvt_Tab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    bool resutl = m_view->edit(index, trigger, &keyPressEvt_Tab);
    EXPECT_FALSE(resutl);

    Stub stub;
    bool (*ut_isEmpty)() = [](){return false;};
    stub.set(ADDR(QUrl, isEmpty), ut_isEmpty);

    int (*ut_selectedIndexCount)() = [](){return 1;};
    stub.set(ADDR(DFileView, selectedIndexCount), ut_selectedIndexCount);

    typedef QRect (*fptr)(DFMStyledItemDelegate*,const QStyleOptionViewItem &, const QModelIndex &);
    fptr DFMStyledItemDelegate_fileNameRect = (fptr)(&DFMStyledItemDelegate::fileNameRect);

    QRect (*ut_fileNameRect)() = [](){return QRect(100, 100, 200, 200);};
    stub.set(DFMStyledItemDelegate_fileNameRect, ut_fileNameRect);

    m_view->switchViewMode(DFileView::ListMode);
    resutl = m_view->edit(index, trigger, &keyPressEvt_Tab);
    EXPECT_FALSE(resutl);

    m_view->switchViewMode(DFileView::IconMode);
    resutl = m_view->edit(index, trigger, &keyPressEvt_Tab);

    stub.reset(ADDR(QUrl, isEmpty));
    stub.reset(ADDR(DFileView, selectedIndexCount));
    stub.reset(DFMStyledItemDelegate_fileNameRect);
}

TEST_F(DFileViewTest,get_select)
{
    ASSERT_NE(nullptr,m_view);

    stub_ext::StubExt stub;
    bool isCalled = false;
    stub.set_lamda(ADDR(DFileView, clearSelection), [&isCalled](){isCalled = true;});

    DUrl url(QString("/test"));
    QList<DUrl> list;
    list << url;
    m_view->select(list);
    EXPECT_TRUE(isCalled);

    stub.reset(ADDR(DFileView, clearSelection));
}

TEST_F(DFileViewTest,select_cut_copy)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    void (*ut_startWork)() = [](){};
    stub.set(ADDR(SelectWork, startWork), ut_startWork);

    static bool isCalled = false;
    void (*ut_clearSelection)() = [](){isCalled = true;};
    stub.set(ADDR(DFileView, clearSelection), ut_clearSelection);

    DUrl url(QString("/test"));
    QList<DUrl> list;
    list << url;
    m_view->selectAllAfterCutOrCopy(list);
    EXPECT_TRUE(isCalled);

    stub.reset(ADDR(SelectWork, startWork));
    stub.reset(ADDR(DFileView, clearSelection));
}

TEST_F(DFileViewTest,slot_set_select)
{
    ASSERT_NE(nullptr,m_view);

    DUrl url(QString("/test"));
    m_view->slotSetSelect(url);
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

    Stub stub;
    static bool myIsValid = false;
    bool (*ut_isValid)() = [](){return myIsValid;};
    stub.set(ADDR(QUrl, isValid), ut_isValid);

    m_view->d_func()->allowedAdjustColumnSize = true;
    DFileView::ViewMode mode = DFileView::ListMode;
    m_view->setDefaultViewMode(mode);
    EXPECT_EQ(mode, m_view->d_func()->defaultViewMode);

    myIsValid = true;
    static QVariant myVariant;
    QVariant (*ut_fileViewStateValue)() = [](){return myVariant;};
    stub.set(ADDR(DFileViewPrivate, fileViewStateValue), ut_fileViewStateValue);
    m_view->setDefaultViewMode(mode);
    EXPECT_EQ(mode, m_view->d_func()->defaultViewMode);

    myVariant = QVariant("test");
    m_view->setDefaultViewMode(mode);
    EXPECT_EQ(mode, m_view->d_func()->defaultViewMode);

    stub.reset(ADDR(QUrl, isValid));
    stub.reset(ADDR(DFileViewPrivate, fileViewStateValue));
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

    Stub stub;
    static bool isCalled = false;
    void (*ut_sort)() = [](){isCalled = true;};
    stub.set((bool(DFileSystemModel::*)())ADDR(DFileSystemModel, sort), ut_sort);

    m_view->switchViewMode(DFileView::ListMode);
    m_view->sortByRole(0, Qt::AscendingOrder);
    EXPECT_TRUE(isCalled);

    isCalled = false;
    m_view->sortByRole(1, Qt::DescendingOrder);
    EXPECT_TRUE(isCalled);

    stub.reset((bool(DFileSystemModel::*)())ADDR(DFileSystemModel, sort));
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

TEST_F(DFileViewTest,set_advance_search_filter)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool mySet = false;
    void (*ut_setAdvanceSearchFilter)() = [](){mySet = true;};
    stub.set(ADDR(DFileSystemModel, setAdvanceSearchFilter), ut_setAdvanceSearchFilter);

    QMap<int, QVariant> formData;
    bool turnOn = true;
    m_view->setAdvanceSearchFilter(formData, turnOn);
    EXPECT_TRUE(mySet);

    stub.reset(ADDR(DFileSystemModel, setAdvanceSearchFilter));
}

TEST_F(DFileViewTest,clear_heard_view)
{
    ASSERT_NE(nullptr,m_view);

    m_view->switchViewMode(DFileView::ListMode);
    m_view->clearHeardView();
    EXPECT_EQ(nullptr, m_view->d_func()->headerView);
}

TEST_F(DFileViewTest,clear_selection)
{
    ASSERT_NE(nullptr,m_view);

    stub_ext::StubExt stub;
    bool isCalled = false;
    stub.set_lamda(ADDR(QListView, clearSelection), [&isCalled](){isCalled = true;});

    m_view->clearSelection();
    EXPECT_TRUE(isCalled);

    stub.reset(ADDR(QListView, clearSelection));
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

    Stub stub;
    static QWidget * w = m_view;
    QWidget* (*ut_focusWidget)() = [](){return w;};
    stub.set(ADDR(QWidget, focusWidget), ut_focusWidget);

    static bool mySet = false;
    void (*ut_setActionWhitelist)() = [](){mySet = true;};
    stub.set(ADDR(DFileMenuManager, setActionWhitelist), ut_setActionWhitelist);

    QSet<MenuAction> actionList;
    m_view->setMenuActionWhitelist(actionList);
    EXPECT_TRUE(m_view->d_func()->menuWhitelist.isEmpty());
    EXPECT_TRUE(mySet);

    stub.reset(ADDR(QWidget, focusWidget));
    stub.reset(ADDR(DFileMenuManager, setActionWhitelist));
}
TEST_F(DFileViewTest,set_menuAction_blacklist)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static QWidget * w = m_view;
    QWidget* (*ut_focusWidget)() = [](){return w;};
    stub.set(ADDR(QWidget, focusWidget), ut_focusWidget);

    static bool mySet = false;
    void (*ut_setActionBlacklist)() = [](){mySet = true;};
    stub.set(ADDR(DFileMenuManager, setActionBlacklist), ut_setActionBlacklist);

    QSet<MenuAction> actionList;
    m_view->setMenuActionBlacklist(actionList);
    EXPECT_TRUE(m_view->d_func()->menuWhitelist.isEmpty());

    stub.reset(ADDR(QWidget, focusWidget));
    stub.reset(ADDR(DFileMenuManager, setActionBlacklist));
}

TEST_F(DFileViewTest,delay_update_statusbar)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool isBusy = true;
    bool (*ut_checkGvfsMountfileBysy)() = [](){return isBusy;};
    stub.set((bool(DFileService::*)(const DUrl&, const bool))ADDR(DFileService, checkGvfsMountfileBusy), ut_checkGvfsMountfileBysy);

    m_view->m_destroyFlag = true;
    m_view->delayUpdateStatusBar();
    EXPECT_FALSE(m_view->d_func()->updateStatusBarTimer->isActive());

    m_view->m_destroyFlag = false;
    m_view->delayUpdateStatusBar();
    EXPECT_FALSE(m_view->d_func()->updateStatusBarTimer->isActive());

    isBusy = false;
    m_view->delayUpdateStatusBar();
    EXPECT_TRUE(m_view->d_func()->updateStatusBarTimer->isActive());

    stub.reset((bool(DFileService::*)(const DUrl&, const bool))ADDR(DFileService, checkGvfsMountfileBusy));
}

TEST_F(DFileViewTest,update_status_bar)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool isNotifyChanged = false;
    void (*ut_notifySelectUrlChanged)() = [](){isNotifyChanged = true;};
    stub.set(ADDR(DFileView, notifySelectUrlChanged), ut_notifySelectUrlChanged);

    static DFileSystemModel::State myState = DFileSystemModel::Unknow;
    DFileSystemModel::State (*ut_state)() = [](){return myState;};
    stub.set(ADDR(DFileSystemModel, state), ut_state);
    m_view->updateStatusBar();
    EXPECT_FALSE(isNotifyChanged);

    isNotifyChanged = false;
    myState = DFileSystemModel::Idle;
    static bool myHasScroller = true;
    bool (*ut_hasScroller)() = [](){return myHasScroller;};
    stub.set(ADDR(QScroller, hasScroller), ut_hasScroller);
    m_view->updateStatusBar();
    EXPECT_FALSE(isNotifyChanged);

    isNotifyChanged = false;
    myHasScroller = false;
    QList<DUrl> (*ut_selectedUrls)() = [](){
        QList<DUrl> list;
        DUrl urlSearch("search:/home");
        DUrl urlOther("file:/home");
        list << urlSearch << urlOther;
        return list;
    };
    stub.set(ADDR(DFileView, selectedUrls), ut_selectedUrls);

    static bool myBusy = true;
    bool (*ut_checkGvfsMountfileBysy)() = [](){return myBusy;};
    stub.set((bool(DFileService::*)(const DUrl&, const bool))ADDR(DFileService, checkGvfsMountfileBusy), ut_checkGvfsMountfileBysy);
    m_view->updateStatusBar();
    EXPECT_FALSE(isNotifyChanged);

    isNotifyChanged = false;
    myBusy = false;
    static int myCount = 0;
    int (*ut_selectedIndexCount)() = [](){return myCount;};
    stub.set(ADDR(DFileView, selectedIndexCount), ut_selectedIndexCount);
    m_view->updateStatusBar();
    EXPECT_TRUE(isNotifyChanged);

    isNotifyChanged = false;
    myCount = 1;
    m_view->updateStatusBar();
    EXPECT_TRUE(isNotifyChanged);

    stub.reset(ADDR(DFileView, notifySelectUrlChanged));
    stub.reset(ADDR(DFileSystemModel, state));
    stub.reset(ADDR(QScroller, hasScroller));
    stub.reset(ADDR(DFileView, selectedUrls));
    stub.reset((bool(DFileService::*)(const DUrl&, const bool))ADDR(DFileService, checkGvfsMountfileBusy));
    stub.reset(ADDR(DFileView, selectedIndexCount));
}

TEST_F(DFileViewTest,open_index_openAction)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    QVariant (*ut_appAttribute)() = [](){return QVariant(0);};
    stub.set(ADDR(DFMApplication, appAttribute), ut_appAttribute);

    int action = 0;
    QModelIndex index;
    m_view->openIndexByOpenAction(action, index);


    stub.reset(ADDR(DFMApplication, appAttribute));
}

TEST_F(DFileViewTest,set_icon_size_index)
{
    ASSERT_NE(nullptr,m_view);

    int sizeIndex = 0;
    m_view->setIconSizeBySizeIndex(sizeIndex);
}

/*TEST_F(DFileViewTest,set_root_url)
{
    ASSERT_NE(nullptr,m_view);

    DUrl url;
    bool result = m_view->setRootUrl(url);
    EXPECT_FALSE(result);

    url = DUrl("/test");
    result = m_view->setRootUrl(url);
    EXPECT_FALSE(result);

    Stub stub;
    static QString myScheme(BURN_SCHEME);
    QString (*ut_scheme)() = [](){return myScheme;};
    stub.set(ADDR(QUrl, scheme), ut_scheme);

    static bool myIsEmpty = true;
    bool (*ut_isEmpty)() = [](){return myIsEmpty;};
    stub.set(ADDR(QStringList, isEmpty), ut_isEmpty);

    bool (*ut_canRedirectionFileUrl)() = [](){return true;};
    stub.set(ADDR(MasteredMediaFileInfo, canRedirectionFileUrl), ut_canRedirectionFileUrl);

    QString (*ut_burnDestDevice)() = [](){return QString("testBurn");};
    stub.set(ADDR(DUrl, burnDestDevice), ut_burnDestDevice);

    QString path("/home");
    url = DUrl(path);
    m_view->switchViewMode(DFileView::ListMode);
    result = m_view->setRootUrl(url);
    EXPECT_FALSE(result);


    stub.reset(ADDR(QUrl, scheme));
    stub.reset(ADDR(QStringList, isEmpty));
    stub.reset(ADDR(MasteredMediaFileInfo, canRedirectionFileUrl));
    stub.reset(ADDR(DUrl, burnDestDevice));
}*/

TEST_F(DFileViewTest,display_action_triggered)
{
    ASSERT_NE(nullptr,m_view);

    QAction action;
    action.setCheckable(true);
    action.setData(MenuAction::IconView);
    m_view->dislpayAsActionTriggered(&action);
    EXPECT_TRUE(action.isChecked());

    action.setData(MenuAction::ListView);
    m_view->dislpayAsActionTriggered(&action);
    EXPECT_TRUE(action.isChecked());

    action.setData(MenuAction::ExtendView);
    m_view->dislpayAsActionTriggered(&action);
    EXPECT_TRUE(action.isChecked());

    action.setData(MenuAction::Open);
    m_view->dislpayAsActionTriggered(&action);
    EXPECT_TRUE(action.isChecked());
}

TEST_F(DFileViewTest,sort_action_triggerred)
{
    ASSERT_NE(nullptr,m_view);

    QAction action;
    m_view->sortByActionTriggered(&action);

    QString path("dfmroot:///desktop.userdir");
    DUrl url(path);
    m_view->setRootUrl(url);

    static QList<QAction*> list{&action};
    Stub stub;
    QList<QAction*> (*ut_actions)() = [](){return list;};
    stub.set(ADDR(QActionGroup, actions), ut_actions);

    static bool mySort = false;
    void (*ut_sortByRole)() = [](){mySort = true;};
    stub.set(ADDR(DFileView, sortByRole), ut_sortByRole);

    m_view->sortByActionTriggered(&action);
    EXPECT_TRUE(mySort);


    stub.reset(ADDR(QActionGroup, actions));
    stub.reset(ADDR(DFileView, sortByRole));
}

TEST_F(DFileViewTest,open_action_triggered)
{
    ASSERT_NE(nullptr,m_view);

    stub_ext::StubExt stub;
    bool isCallOpen = false;
    stub.set_lamda(ADDR(DFileService, openFileByApp), [&isCallOpen](){return isCallOpen=true;});

    QAction action;
    m_view->openWithActionTriggered(&action);
    EXPECT_TRUE(isCallOpen);

    stub.reset(ADDR(DFileService, openFileByApp));
}

TEST_F(DFileViewTest,row_count_changed)
{
    ASSERT_NE(nullptr,m_view);

    stub_ext::StubExt stub;
    bool isCallUpdate = false;
    stub.set_lamda(ADDR(DFileView, updateModelActiveIndex), [&isCallUpdate](){isCallUpdate = true;});

    m_view->onRowCountChanged();
    EXPECT_TRUE(isCallUpdate);

    stub.reset(ADDR(DFileView, updateModelActiveIndex));
}

TEST_F(DFileViewTest,update_model_index)
{
    ASSERT_NE(nullptr,m_view);

    m_view->m_isRemovingCase = false;
    m_view->updateModelActiveIndex();
    EXPECT_FALSE(m_view->m_isRemovingCase);
}

TEST_F(DFileViewTest,handle_data_changed)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool isCalledChanged = false;
    void (*ut_dataChanged)() = [](){isCalledChanged = true;};

    typedef void (*fptr)(QListView*,const QModelIndex&, const QModelIndex&, const QVector<int>&);
    fptr QListView_dataChanged = (fptr)(&QListView::dataChanged);
    stub.set(QListView_dataChanged, ut_dataChanged);

    QModelIndex topLeft,bottomRight;
    m_view->handleDataChanged(topLeft, bottomRight);
    EXPECT_TRUE(isCalledChanged);

    stub.reset(QListView_dataChanged);
}

TEST_F(DFileViewTest,root_url_deleted)
{
    ASSERT_NE(nullptr,m_view);

    stub_ext::StubExt stub;
    bool isCalledCdTo = false;
    stub.set_lamda(ADDR(DFMBaseView, requestCdTo), [&isCalledCdTo](){isCalledCdTo = true;});

    DUrl url;
    m_view->onRootUrlDeleted(url);
    EXPECT_TRUE(isCalledCdTo);

    stub.reset(ADDR(DFMBaseView, requestCdTo));
}

TEST_F(DFileViewTest,fresh_view)
{
    ASSERT_NE(nullptr,m_view);

    stub_ext::StubExt stub;
    bool isCalledRefresh = false;
    stub.set_lamda(ADDR(DFileSystemModel, refresh), [&isCalledRefresh](){isCalledRefresh = true;});

    m_view->freshView();
    EXPECT_TRUE(isCalledRefresh);

    stub.reset(ADDR(DFileSystemModel, refresh));
}

TEST_F(DFileViewTest,load_view_state)
{
    ASSERT_NE(nullptr,m_view);

    DFMApplication::appObtuselySetting()->setValue("ApplicationAttribute", "UseParentViewMode", true);
    QString path("dfmroot:///desktop.userdir");
    DUrl url(path);
    m_view->setRootUrl(url);

    stub_ext::StubExt stub;
    bool isCalledSwitchMode = false;
    stub.set_lamda(ADDR(DFileView, switchViewMode), [&isCalledSwitchMode](){isCalledSwitchMode = true;});

    m_view->loadViewState(url);
    EXPECT_TRUE(isCalledSwitchMode);

    stub.reset(ADDR(DFileView, switchViewMode));
}

TEST_F(DFileViewTest,save_view_state)
{
    ASSERT_NE(nullptr,m_view);

    m_view->saveViewState();
}

TEST_F(DFileViewTest,sort_indicator_changed)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool isCalled = false;
    void (*ut_sort)() = [](){isCalled = true;};
    stub.set((bool(DFileSystemModel::*)())ADDR(DFileSystemModel, sort), ut_sort);

    int logicalIndex = 0;
    Qt::SortOrder order = Qt::DescendingOrder;
    m_view->onSortIndicatorChanged(logicalIndex, order);
    EXPECT_TRUE(isCalled);

    stub.reset((bool(DFileSystemModel::*)())ADDR(DFileSystemModel, sort));
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

    stub_ext::StubExt stub;
    bool isCalledReset = false;

    typedef void (*fptr)(QListView*);
    fptr QListView_reset = (fptr)(&QListView::reset);

    stub.set_lamda(QListView_reset, [&isCalledReset](){isCalledReset = true;});

    m_view->reset();
    EXPECT_TRUE(isCalledReset);

    stub.reset(QListView_reset);
}

TEST_F(DFileViewTest,set_root_index)
{
    ASSERT_NE(nullptr,m_view);

    stub_ext::StubExt stub;
    bool isCalledSet = false;

    typedef void (*fptr)(QListView*, const QModelIndex&);
    fptr QListView_setRootIndex = (fptr)(&QListView::setRootIndex);

    stub.set_lamda(QListView_setRootIndex, [&isCalledSet](){isCalledSet = true;});

    QModelIndex index;
    m_view->setRootIndex(index);
    EXPECT_TRUE(isCalledSet);

    stub.reset(QListView_setRootIndex);
}

TEST_F(DFileViewTest,tst_wheel_event)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool myPressed = false;
    bool (*ut_keyCtrlIsPressed)() = [](){return myPressed;};
    stub.set(ADDR(DFMGlobal, keyCtrlIsPressed), ut_keyCtrlIsPressed);

    m_view->switchViewMode(DFileView::IconMode);
    QWheelEvent event1(QPointF(0,0), 1, Qt::MiddleButton, Qt::NoModifier);
    m_view->wheelEvent(&event1);

    myPressed = true;
    QWheelEvent event2(QPointF(0,0), 1, Qt::MiddleButton, Qt::ControlModifier);
    m_view->wheelEvent(&event2);

    m_view->switchViewMode(DFileView::ListMode);
    QWheelEvent event3(QPointF(0,0), 1, Qt::MiddleButton, Qt::AltModifier);
    m_view->wheelEvent(&event3);

    QWheelEvent event4(QPointF(0,0), 1, Qt::MiddleButton, Qt::NoModifier);
    m_view->wheelEvent(&event4);

    stub.reset(ADDR(DFMGlobal, keyCtrlIsPressed));
}

TEST_F(DFileViewTest,tst_keyPress_event)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool myCall = false;
    void (*ut_callFunction)() = [](){myCall = true;};
    stub.set(ADDR(AppController, actionNewFolder), ut_callFunction);

    QKeyEvent keyPressEvt_Space(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    m_view->keyPressEvent(&keyPressEvt_Space);
    QKeyEvent keyPressEvt_NA(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    m_view->keyPressEvent(&keyPressEvt_NA);

    QKeyEvent keyPressEvt_Enter(QEvent::KeyPress, Qt::Key_Enter, Qt::KeypadModifier);
    m_view->keyPressEvent(&keyPressEvt_Enter);
    QKeyEvent keyPressEvt_Backspace(QEvent::KeyPress, Qt::Key_Backspace, Qt::KeypadModifier);
    m_view->keyPressEvent(&keyPressEvt_Backspace);
    QKeyEvent keyPressEvt_Delete(QEvent::KeyPress, Qt::Key_Delete, Qt::KeypadModifier);
    m_view->keyPressEvent(&keyPressEvt_Delete);
    QKeyEvent keyPressEvt_End(QEvent::KeyPress, Qt::Key_End, Qt::KeypadModifier);
    m_view->keyPressEvent(&keyPressEvt_End);
    QKeyEvent keyPressEvt_KA(QEvent::KeyPress, Qt::Key_A, Qt::KeypadModifier);
    m_view->keyPressEvent(&keyPressEvt_KA);

    QKeyEvent keyPressEvt_N(QEvent::KeyPress, Qt::Key_N, Qt::ControlModifier);
    m_view->keyPressEvent(&keyPressEvt_N);
    QKeyEvent keyPressEvt_H(QEvent::KeyPress, Qt::Key_H, Qt::ControlModifier);
    m_view->keyPressEvent(&keyPressEvt_H);
    QKeyEvent keyPressEvt_I(QEvent::KeyPress, Qt::Key_I, Qt::ControlModifier);
    m_view->keyPressEvent(&keyPressEvt_I);
    QKeyEvent keyPressEvt_Up(QEvent::KeyPress, Qt::Key_Up, Qt::ControlModifier);
    m_view->keyPressEvent(&keyPressEvt_Up);
    QKeyEvent keyPressEvt_Down(QEvent::KeyPress, Qt::Key_Down, Qt::ControlModifier);
    m_view->keyPressEvent(&keyPressEvt_Down);
    QKeyEvent keyPressEvt_T(QEvent::KeyPress, Qt::Key_T, Qt::ControlModifier);
    m_view->keyPressEvent(&keyPressEvt_T);
    QKeyEvent keyPressEvt_CA(QEvent::KeyPress, Qt::Key_A, Qt::ControlModifier);
    m_view->keyPressEvent(&keyPressEvt_CA);

    QKeyEvent keyPressEvt_SDelete(QEvent::KeyPress, Qt::Key_Delete, Qt::ShiftModifier);
    m_view->keyPressEvent(&keyPressEvt_SDelete);
    QKeyEvent keyPressEvt_ST(QEvent::KeyPress, Qt::Key_T, Qt::ShiftModifier);
    m_view->keyPressEvent(&keyPressEvt_ST);
    QKeyEvent keyPressEvt_SA(QEvent::KeyPress, Qt::Key_A, Qt::ShiftModifier);
    m_view->keyPressEvent(&keyPressEvt_SA);

    QKeyEvent keyPressEvt_CSN(QEvent::KeyPress, Qt::Key_N, Qt::ControlModifier | Qt::ShiftModifier);
    m_view->keyPressEvent(&keyPressEvt_CSN);
    QKeyEvent keyPressEvt_CSA(QEvent::KeyPress, Qt::Key_A, Qt::ControlModifier | Qt::ShiftModifier);
    m_view->keyPressEvent(&keyPressEvt_CSA);

    QKeyEvent keyPressEvt_AUp(QEvent::KeyPress, Qt::Key_Up, Qt::AltModifier);
    m_view->keyPressEvent(&keyPressEvt_AUp);
    QKeyEvent keyPressEvt_ADown(QEvent::KeyPress, Qt::Key_Down, Qt::AltModifier);
    m_view->keyPressEvent(&keyPressEvt_ADown);
    QKeyEvent keyPressEvt_AHome(QEvent::KeyPress, Qt::Key_Home, Qt::AltModifier);
    m_view->keyPressEvent(&keyPressEvt_AHome);
    QKeyEvent keyPressEvt_AA(QEvent::KeyPress, Qt::Key_A, Qt::AltModifier);
    m_view->keyPressEvent(&keyPressEvt_AA);

    stub.reset(ADDR(AppController, actionNewFolder));
}

TEST_F(DFileViewTest,tst_show_event)
{
    Stub stub;
    typedef int(*fptr)(QDialog*);
    fptr pQDialogExec = (fptr)(&QDialog::exec);
    fptr pDDialogExec = (fptr)(&DDialog::exec);
    int (*stub_DDialog_exec)(void) = [](void)->int{return QDialog::Accepted;};
    stub.set(pQDialogExec, stub_DDialog_exec);
    stub.set(pDDialogExec, stub_DDialog_exec);

    ASSERT_NE(nullptr,m_view);

    m_view->show();
    ASSERT_TRUE(QTest::qWaitForWindowExposed(m_view));
    EXPECT_FALSE(m_view->isHidden());
    m_view->hide();

    stub.reset(pQDialogExec);
    stub.reset(pDDialogExec);
}

TEST_F(DFileViewTest,mouse_press_event)
{
    ASSERT_NE(nullptr,m_view);

    QPoint pos = m_view->rect().center();
    QMouseEvent event1(QEvent::MouseButtonPress, pos, Qt::BackButton, Qt::BackButton, Qt::NoModifier);
    m_view->mousePressEvent(&event1);

    QMouseEvent event2(QEvent::MouseButtonPress, pos, Qt::ForwardButton, Qt::ForwardButton, Qt::NoModifier);
    m_view->mousePressEvent(&event2);

    Stub stub;
    static bool myIsEmptyArea = true;
    bool (*ut_isEmptyArea)() = [](){return myIsEmptyArea;};
    stub.set(ADDR(DFileViewHelper, isEmptyArea), ut_isEmptyArea);

    static bool myCtrlPressed = false;
    bool (*ut_keyCtrlIsPressed)() = [](){return myCtrlPressed;};
    stub.set(ADDR(DFMGlobal, keyCtrlIsPressed), ut_keyCtrlIsPressed);

    QMouseEvent event3(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_view->mousePressEvent(&event3);

    myIsEmptyArea = false;
    myCtrlPressed = true;
    static bool mySelected = true;
    bool (*ut_isSelected)() = [](){return mySelected;};
    stub.set(ADDR(QItemSelectionModel, isSelected), ut_isSelected);

    QMouseEvent event4(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);
    m_view->mousePressEvent(&event4);

    myCtrlPressed = false;
    mySelected = false;
    static bool myShiftPressed = true;
    bool (*ut_keyShiftIsPressed)() = [](){return myShiftPressed;};
    stub.set(ADDR(DFMGlobal, keyShiftIsPressed), ut_keyShiftIsPressed);

    QMouseEvent event5(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
    m_view->mousePressEvent(&event5);

    QMouseEvent event6(QEvent::MouseButtonPress, pos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    m_view->mousePressEvent(&event6);

    QMouseEvent event7(QEvent::MouseButtonPress, pos, Qt::MidButton, Qt::MidButton, Qt::NoModifier);
    m_view->mousePressEvent(&event7);

    stub.reset(ADDR(DFileViewHelper, isEmptyArea));
    stub.reset(ADDR(DFMGlobal, keyCtrlIsPressed));
    stub.reset(ADDR(QItemSelectionModel, isSelected));
    stub.reset(ADDR(DFMGlobal, keyShiftIsPressed));
}

TEST_F(DFileViewTest,mouse_move_event)
{
    ASSERT_NE(nullptr,m_view);

    QPoint pos = m_view->rect().center();
    QMouseEvent event(QEvent::MouseMove, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_view->mouseMoveEvent(&event);
}

TEST_F(DFileViewTest,mouse_release_event)
{
    ASSERT_NE(nullptr,m_view);

    static QModelIndex index;
    m_view->d_func()->mouseLastPressedIndex = index;

    Stub stub;
    static bool myIsValid = true;
    bool (*ut_isValid)() = [](){return myIsValid;};
    stub.set(ADDR(QModelIndex, isValid), ut_isValid);

    static bool myCtrlPressed = true;
    bool (*ut_keyCtrlIsPressed)() = [](){return myCtrlPressed;};
    stub.set(ADDR(DFMGlobal, keyCtrlIsPressed), ut_keyCtrlIsPressed);

    QPoint pos = m_view->rect().center();
    QMouseEvent event(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_view->mouseReleaseEvent(&event);

    EXPECT_TRUE(m_view->d_func()->dragMoveHoverIndex.isValid());

    stub.reset(ADDR(QModelIndex, isValid));
    stub.reset(ADDR(DFMGlobal, keyCtrlIsPressed));
}

TEST_F(DFileViewTest,focus_in_event)
{
    ASSERT_NE(nullptr,m_view);

    QFocusEvent event(QEvent::FocusIn);
    m_view->focusInEvent(&event);
}

TEST_F(DFileViewTest,resize_event)
{
    ASSERT_NE(nullptr,m_view);

    QResizeEvent event(QSize(100,100), QSize(10,10));
    m_view->resizeEvent(&event);
}

TEST_F(DFileViewTest,context_menu_event)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool myIsValid = false;
    bool (*ut_isValid)() = [](){return myIsValid;};
    stub.set(ADDR(QModelIndex, isValid), ut_isValid);

    static bool myIsEmptyArea = true;
    bool (*ut_isEmptyArea)() = [](){return myIsEmptyArea;};
    stub.set(ADDR(DFileViewHelper, isEmptyArea), ut_isEmptyArea);

    static bool myShowEmpty = false;
    void (*ut_showEmptyAreMenu)() = [](){myShowEmpty = true;};
    stub.set(ADDR(DFileView, showEmptyAreaMenu), ut_showEmptyAreMenu);

    static bool myShowNormal = false;
    void (*ut_showNormalMenu)() = [](){myShowNormal = true;};
    stub.set(ADDR(DFileView, showNormalMenu), ut_showNormalMenu);

    static bool myBusy = true;
    bool (*ut_checkGvfsMountfileBysy)() = [](){return myBusy;};
    stub.set((bool(DFileService::*)(const DUrl&, const bool))ADDR(DFileService, checkGvfsMountfileBusy), ut_checkGvfsMountfileBysy);

    m_view->switchViewMode(DFileView::IconMode);
    QPoint pos = m_view->rect().center();
    QContextMenuEvent event(QContextMenuEvent::Mouse, pos);    
    m_view->contextMenuEvent(&event);

    myBusy = false;
    m_view->contextMenuEvent(&event);

    myIsValid = true;
    myIsEmptyArea = false;
    myBusy = true;
    m_view->contextMenuEvent(&event);

    myBusy = false;
    m_view->contextMenuEvent(&event);

    stub.reset(ADDR(QModelIndex, isValid));
    stub.reset(ADDR(DFileViewHelper, isEmptyArea));
    stub.reset(ADDR(DFileView, showEmptyAreaMenu));
    stub.reset(ADDR(DFileView, showNormalMenu));
    stub.reset((bool(DFileService::*)(const DUrl&, const bool))ADDR(DFileService, checkGvfsMountfileBusy));
}

TEST_F(DFileViewTest,drag_enter_event)
{
    ASSERT_NE(nullptr,m_view);

    QPoint pos = m_view->rect().center();
    QMimeData data;
    QDragEnterEvent event(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);

    Stub stub;
    static bool myCheckData = true;
    bool (*ut_checkMimeData)() = [](){return myCheckData;};
    stub.set(ADDR(DFileDragClient, checkMimeData), ut_checkMimeData);

    m_view->dragEnterEvent(&event);

    myCheckData = false;
    static bool myFetchMemory = false;
    bool (*ut_fetchDragEventUrlsFromSharedMemory)() = [](){return myFetchMemory;};
    stub.set(ADDR(DFileView, fetchDragEventUrlsFromSharedMemory), ut_fetchDragEventUrlsFromSharedMemory);
    m_view->dragEnterEvent(&event);

    myFetchMemory = true;
    m_view->m_urlsForDragEvent.append(QUrl("/home"));
    m_view->dragEnterEvent(&event);

    m_view->m_urlsForDragEvent.clear();
    m_view->dragEnterEvent(&event);

    stub.reset(ADDR(DFileDragClient, checkMimeData));
    stub.reset(ADDR(DFileView, fetchDragEventUrlsFromSharedMemory));
}

TEST_F(DFileViewTest,drag_move_event)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool myIsValid = true;
    bool (*ut_isValid)() = [](){return myIsValid;};
    stub.set(ADDR(QModelIndex, isValid), ut_isValid);

    QPoint pos = m_view->rect().center();
    QMimeData data;
    QDragMoveEvent event(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    m_view->dragMoveEvent(&event);

    stub.reset(ADDR(QModelIndex, isValid));
}

TEST_F(DFileViewTest,drag_leave_event)
{
    ASSERT_NE(nullptr,m_view);

    QDragLeaveEvent event;
    m_view->dragLeaveEvent(&event);
}

TEST_F(DFileViewTest,tst_drop_event)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool myCheckData = true;
    bool (*ut_checkMimeData)() = [](){return myCheckData;};
    stub.set(ADDR(DFileDragClient, checkMimeData), ut_checkMimeData);

    QString path("dfmroot:///desktop.userdir");
    DUrl url(path);
    m_view->setRootUrl(url);

    QPoint pos = m_view->rect().center();
    QMimeData data;
    QDropEvent event1(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    m_view->dropEvent(&event1);

    data.setProperty("IsDirectSaveMode", true);
    QDropEvent event2(pos, Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    m_view->dropEvent(&event2);

    stub.reset(ADDR(DFileDragClient, checkMimeData));
}

TEST_F(DFileViewTest,set_selection)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool myShiftPressed = true;
    bool (*ut_keyShiftIsPressed)() = [](){return myShiftPressed;};
    stub.set(ADDR(DFMGlobal, keyShiftIsPressed), ut_keyShiftIsPressed);

    static bool myIsValid = false;
    bool (*ut_isValid)() = [](){return myIsValid;};
    stub.set(ADDR(QModelIndex, isValid), ut_isValid);

    m_view->switchViewMode(DFileView::IconMode);
    QRect rect = m_view->rect();
    QItemSelectionModel::SelectionFlags flags;
    m_view->setSelection(rect, flags);

    myIsValid = true;
    m_view->setSelection(rect, flags);

    myShiftPressed = false;
    flags = QItemSelectionModel::Current | QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect;
    m_view->setSelection(rect, flags);

    m_view->switchViewMode(DFileView::ListMode);
    m_view->setSelection(rect, flags);

    stub.reset(ADDR(DFMGlobal, keyShiftIsPressed));
    stub.reset(ADDR(QModelIndex, isValid));
}

TEST_F(DFileViewTest,move_cursor)
{
    ASSERT_NE(nullptr,m_view);    

    QString path("dfmroot:///desktop.userdir");
    DUrl url(path);
    m_view->setRootUrl(url);

    Stub stub;
    static bool myIsValid = false;
    bool (*ut_isValid)() = [](){return myIsValid;};
    stub.set(ADDR(QModelIndex, isValid), ut_isValid);

    QAbstractItemView::CursorAction cursorAction = QAbstractItemView::MoveLeft;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;

    QModelIndex index = m_view->moveCursor(cursorAction, modifiers);
    EXPECT_EQ(index, m_view->d_func()->lastCursorIndex);

    myIsValid = true;
    static bool myIsEmpty = true;
    bool (*ut_isEmpty)() = [](){return myIsEmpty;};
    stub.set(ADDR(QRect, isEmpty), ut_isEmpty);

    static Qt::ItemFlags myFlags = index.flags() | Qt::ItemIsSelectable;
    Qt::ItemFlags (*ut_flags)() = [](){return myFlags;};
    stub.set(ADDR(QModelIndex, flags), ut_flags);

    index = m_view->moveCursor(cursorAction, modifiers);
    EXPECT_EQ(index, m_view->d_func()->lastCursorIndex);

    myIsEmpty = false;
    static bool myShiftPressed = true;
    bool (*ut_keyShiftIsPressed)() = [](){return myShiftPressed;};
    stub.set(ADDR(DFMGlobal, keyShiftIsPressed), ut_keyShiftIsPressed);//switch iconmode
    m_view->switchViewMode(DFileView::IconMode);
    index = m_view->moveCursor(cursorAction, modifiers);
    EXPECT_TRUE(index.isValid());

    myShiftPressed = false;
    index = m_view->moveCursor(cursorAction, modifiers);
    EXPECT_TRUE(index.isValid());

    cursorAction = QAbstractItemView::MoveRight;
    myShiftPressed = true;
    index = m_view->moveCursor(cursorAction, modifiers);
    EXPECT_TRUE(index.isValid());

    myShiftPressed = false;
    index = m_view->moveCursor(cursorAction, modifiers);
    EXPECT_TRUE(index.isValid());

    cursorAction = QAbstractItemView::MoveUp;
    index = m_view->moveCursor(cursorAction, modifiers);
    EXPECT_TRUE(index.isValid());

    stub.reset(ADDR(QModelIndex, isValid));
    stub.reset(ADDR(QRect, isEmpty));
    stub.reset(ADDR(QModelIndex, flags));
    stub.reset(ADDR(DFMGlobal, keyShiftIsPressed));
}

TEST_F(DFileViewTest,rows_about_removed)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndex index;
    int start = -10;
    int end = -5;
    m_view->rowsAboutToBeRemoved(index, start, end);

    start = -10;
    end = 5;
    m_view->rowsAboutToBeRemoved(index, start, end);
}

TEST_F(DFileViewTest,row_inserted)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndex index;
    int start = 0;
    int end = 10;
    m_view->rowsInserted(index, start, end);
}

TEST_F(DFileViewTest,data_changed)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndex topLeft, bottomRight;
    QVector<int> roles = QVector<int>();
    m_view->dataChanged(topLeft, bottomRight, roles);
}

TEST_F(DFileViewTest,tst_event)
{
    ASSERT_NE(nullptr,m_view);

    QKeyEvent event1(QEvent::KeyPress, Qt::Key_Tab, Qt::ControlModifier);

    QKeyEvent event2(QEvent::KeyPress, Qt::Key_Tab, Qt::ShiftModifier);
    bool result = m_view->event(&event2);
    EXPECT_TRUE(result);

    QKeyEvent event3(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    result = m_view->event(&event3);
    EXPECT_TRUE(result);

    QResizeEvent event4(QSize(100, 100), QSize(10, 10));
    m_view->event(&event4);

    QEvent event5(QEvent::ParentChange);
    m_view->event(&event5);

    QEvent event6(QEvent::FontChange);
    m_view->event(&event6);

}

TEST_F(DFileViewTest,update_geome_tries)
{
    ASSERT_NE(nullptr,m_view);

    m_view->updateGeometries();

    m_view->switchViewMode(DFileView::ListMode);
    m_view->d_func()->allowedAdjustColumnSize = true;
    m_view->updateGeometries();
}

TEST_F(DFileViewTest,event_filter)
{
    ASSERT_NE(nullptr,m_view);

    QObject *obj = nullptr;
    QEvent event(QEvent::Move);
    m_view->eventFilter(obj, &event);

    obj = m_view->horizontalScrollBar()->parentWidget();
    m_view->eventFilter(obj, &event);

    event = QEvent(QEvent::WindowStateChange);
    m_view->eventFilter(obj, &event);

    event = QEvent(QEvent::ShowToParent);
    m_view->eventFilter(obj, &event);

    event = QEvent(QEvent::Wheel);
    m_view->eventFilter(obj, &event);
}

TEST_F(DFileViewTest,show_hidden_changed)
{
    ASSERT_NE(nullptr,m_view);

    QDir::Filters filters;

    if (DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedHiddenFiles).toBool())
        filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
    else
        filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;

    m_view->onShowHiddenFileChanged();

    EXPECT_EQ(filters, m_view->model()->filters());
}

TEST_F(DFileViewTest,in_crease_icon)
{
    ASSERT_NE(nullptr,m_view);

    int oldLevel = m_view->itemDelegate()->iconSizeLevel();
    m_view->increaseIcon();

    int newLevel = m_view->itemDelegate()->iconSizeLevel();
    EXPECT_LE(oldLevel, newLevel);
}

TEST_F(DFileViewTest,de_crease_icon)
{
    ASSERT_NE(nullptr,m_view);

    int oldLevel = m_view->itemDelegate()->iconSizeLevel();
    m_view->decreaseIcon();

    int newLevel = m_view->itemDelegate()->iconSizeLevel();
    EXPECT_GE(oldLevel, newLevel);
}

TEST_F(DFileViewTest,open_index)
{
    ASSERT_NE(nullptr,m_view);

    QModelIndex index;
    m_view->openIndex(index);
}

TEST_F(DFileViewTest,key_board_search)
{
    ASSERT_NE(nullptr,m_view);

    QString search;
    m_view->keyboardSearch(search);

    search = QString("test");
    m_view->keyboardSearch(search);
}

TEST_F(DFileViewTest,update_horizontal_offset)
{
    ASSERT_NE(nullptr,m_view);

    m_view->switchViewMode(DFileView::ListMode);
    m_view->updateHorizontalOffset();

    m_view->switchViewMode(DFileView::IconMode);
    m_view->updateHorizontalOffset();
}

TEST_F(DFileViewTest,switch_view_mode)
{
    ASSERT_NE(nullptr,m_view);

    m_view->d_func()->currentViewMode = DFileView::IconMode;
    m_view->switchViewMode(DFileView::IconMode);
    DFileView::ViewMode result = m_view->viewMode();
    EXPECT_EQ(result, DFileView::IconMode);

    m_view->d_func()->allowedAdjustColumnSize = true;
    m_view->switchViewMode(DFileView::ListMode);
    result = m_view->viewMode();
    EXPECT_EQ(result, DFileView::ListMode);

    m_view->d_func()->currentViewMode = DFileView::IconMode;
    m_view->d_func()->allowedAdjustColumnSize = false;
    m_view->switchViewMode(DFileView::ListMode);
    result = m_view->viewMode();
    EXPECT_EQ(result, DFileView::ListMode);

    m_view->d_func()->currentViewMode = DFileView::ListMode;
    m_view->switchViewMode(DFileView::IconMode);
    result = m_view->viewMode();
    EXPECT_EQ(result, DFileView::IconMode);

    m_view->switchViewMode(DFileView::ExtendMode);
    result = m_view->viewMode();
    EXPECT_EQ(result, DFileView::ExtendMode);
}

TEST_F(DFileViewTest,update_listheader_viewproperty)
{
    ASSERT_NE(nullptr,m_view);

    m_view->switchViewMode(DFileView::ListMode);
    m_view->updateListHeaderViewProperty();
}
#ifndef __arm__
TEST_F(DFileViewTest,show_emptyare_menu)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool myCall = false;
    void (*ut_exec)() = [](){myCall = true;};
    stub.set((QAction*(DFileMenu::*)())ADDR(DFileMenu, exec), ut_exec);

    QString path("dfmroot:///desktop.userdir");
    DUrl url(path);
    m_view->setRootUrl(url);

    QMap<MenuAction, QVector<MenuAction> > (*ut_subMenuActionList)() = [](){
        QMap<MenuAction, QVector<MenuAction> > map;
        return map;
    };
    typedef QMap<MenuAction, QVector<MenuAction> > (*fptr)(DAbstractFileInfo*);
    fptr DAbstractFileInof_subMenuActionList = (fptr)(&DAbstractFileInfo::subMenuActionList);
    stub.set(DAbstractFileInof_subMenuActionList, ut_subMenuActionList);

    Qt::ItemFlags indexFlags = Qt::NoItemFlags;
    m_view->showEmptyAreaMenu(indexFlags);
    EXPECT_TRUE(myCall);

    stub.reset((QAction*(DFileMenu::*)())ADDR(DFileMenu, exec));
    stub.reset(DAbstractFileInof_subMenuActionList);
}
#endif
TEST_F(DFileViewTest,show_normal_menu)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool myCall = false;
    void (*ut_exec)() = [](){myCall = true;};
    stub.set((QAction*(DFileMenu::*)())ADDR(DFileMenu, exec), ut_exec);

    QString path("dfmroot:///desktop.userdir");
    DUrl url(path);
    m_view->setRootUrl(url);

    QModelIndex index;
    Qt::ItemFlags indexFlags = Qt::NoItemFlags;
    m_view->showNormalMenu(index, indexFlags);

    static bool myIsValid = true;
    bool (*ut_isValid)() = [](){return myIsValid;};
    stub.set(ADDR(QModelIndex, isValid), ut_isValid);

    m_view->showNormalMenu(index, indexFlags);

    stub.reset((QAction*(DFileMenu::*)())ADDR(DFileMenu, exec));
    stub.reset(ADDR(QModelIndex, isValid));
}

TEST_F(DFileViewTest,update_extenheader_viewproperty)
{
    ASSERT_NE(nullptr,m_view);

    m_view->d_func()->headerView = nullptr;
    m_view->updateExtendHeaderViewProperty();

    m_view->switchViewMode(DFileView::ListMode);

    Stub stub;
    static bool myCall = false;
    void (*ut_setSectionResizeMode)() = [](){myCall = true;};
    stub.set((void(QHeaderView::*)(int, QHeaderView::ResizeMode))ADDR(QHeaderView, setSectionResizeMode), ut_setSectionResizeMode);

    m_view->d_func()->allowedAdjustColumnSize = false;
    m_view->updateExtendHeaderViewProperty();
    EXPECT_TRUE(myCall);

    stub.reset((void(QHeaderView::*)(int, QHeaderView::ResizeMode))ADDR(QHeaderView, setSectionResizeMode));
}

TEST_F(DFileViewTest,update_column_width)
{
    ASSERT_NE(nullptr,m_view);

    QString path("/usr/lib");
    DUrl url(path);
    m_view->setRootUrl(url);
    m_view->switchViewMode(DFileView::ListMode);

    Stub stub;
    int (*ut_count)() = [](){return 3;};
    stub.set(ADDR(QHeaderView, count), ut_count);

    static bool myHidden = true;
    bool (*ut_isSectionHidden)() = [](){return myHidden;};
    stub.set(ADDR(QHeaderView, isSectionHidden), ut_isSectionHidden);

    static bool mySection = false;
    void (*ut_resizeSection)() = [](){mySection = true;};
    stub.set(ADDR(QHeaderView, resizeSection), ut_resizeSection);

    int (*ut_columnWidthByRole)() = [](){return 10;};
    stub.set(ADDR(DFileSystemModel, columnWidthByRole), ut_columnWidthByRole);

    m_view->d_func()->allowedAdjustColumnSize = false;
    m_view->d_func()->firstVisibleColumn = 1;
    m_view->d_func()->lastVisibleColumn = 1;
    m_view->updateColumnWidth();
    EXPECT_TRUE(mySection);

    myHidden = false;
    m_view->updateColumnWidth();
    EXPECT_TRUE(mySection);

    stub.reset(ADDR(QHeaderView, count));
    stub.reset(ADDR(QHeaderView, isSectionHidden));
    stub.reset(ADDR(QHeaderView, resizeSection));
    stub.reset(ADDR(DFileSystemModel, columnWidthByRole));
}

TEST_F(DFileViewTest,popup_header_view_contextmenu)
{
    ASSERT_NE(nullptr,m_view);

    QString path("dfmroot:///desktop.userdir");
    DUrl url(path);
    m_view->setRootUrl(url);
    m_view->switchViewMode(DFileView::ListMode);

    Stub stub;
    static bool myCallExec = false;
    void (*ut_exec)(void *obj, const QPoint&, QAction*) = [](void *obj, const QPoint&, QAction*){
        myCallExec = true;
        DFileMenu *menu = (DFileMenu*)obj;
        QList<QAction*> actions = menu->actions();
        for(auto action : actions) {
            action->trigger();
        }
    };
    stub.set((QAction*(DFileMenu::*)(const QPoint&, QAction*))ADDR(DFileMenu, exec), ut_exec);

    static bool myIsCompact = true;
    bool (*ut_columnIsCompact)() = [](){return myIsCompact;};
    stub.set(ADDR(DAbstractFileInfo, columnIsCompact), ut_columnIsCompact);

    typedef QList<int> (*fptr)(DAbstractFileInfo*, int);
    fptr DAbstractFileInfo_userColumnChildRoles = (fptr)(&DAbstractFileInfo::userColumnChildRoles);
    static QList<int> myList;
    QList<int> (*ut_userColumnChildRoles)() = [](){return myList;};
    stub.set(DAbstractFileInfo_userColumnChildRoles, ut_userColumnChildRoles);

    QPoint pos(10, 10);
    m_view->popupHeaderViewContextMenu(pos);
    EXPECT_FALSE(myCallExec);

    static bool myHidden = false;
    bool (*ut_isSectionHidden)() = [](){return myHidden;};
    stub.set(ADDR(QHeaderView, isSectionHidden), ut_isSectionHidden);

    static int myRole = 1;
    int (*ut_sortRole)() = [](){return myRole++;};
    stub.set(ADDR(DFileSystemModel, sortRole), ut_sortRole);

    myList << 1 << 2 << 3;
    m_view->popupHeaderViewContextMenu(pos);
    EXPECT_TRUE(myCallExec);

    myIsCompact = false;
    myList.clear();
    m_view->popupHeaderViewContextMenu(pos);
    EXPECT_TRUE(myCallExec);

    stub.reset((QAction*(DFileMenu::*)(const QPoint&, QAction*))ADDR(DFileMenu, exec));
    stub.reset(ADDR(DAbstractFileInfo, columnIsCompact));
    stub.reset(DAbstractFileInfo_userColumnChildRoles);
    stub.reset(ADDR(QHeaderView, isSectionHidden));
    stub.reset(ADDR(DFileSystemModel, sortRole));
}

TEST_F(DFileViewTest,on_model_statechanged)
{
    ASSERT_NE(nullptr,m_view);

    QString path("dfmroot:///desktop.userdir");
    DUrl url(path);
    m_view->setRootUrl(url);
    m_view->switchViewMode(DFileView::ListMode);

    Stub stub;
    static bool myCallChanged = false;
    void (*ut_notifyStateChanged)() = [](){myCallChanged = true;};
    stub.set(ADDR(DFMBaseView, notifyStateChanged), ut_notifyStateChanged);

    int state = DFileSystemModel::Busy;
    m_view->onModelStateChanged(state);
    EXPECT_TRUE(myCallChanged);

    myCallChanged = false;
    state = DFileSystemModel::Idle;
    m_view->d_func()->preSelectionUrls.append(DUrl("/home"));
    m_view->onModelStateChanged(state);
    EXPECT_TRUE(myCallChanged);

    stub.reset(ADDR(DFMBaseView, notifyStateChanged));
}

TEST_F(DFileViewTest,update_content_label)
{
    ASSERT_NE(nullptr,m_view);

    DFileSystemModel::State state = DFileSystemModel::Busy;
    m_view->model()->setState(state);
    m_view->updateContentLabel();

    state = DFileSystemModel::Idle;
    m_view->model()->setState(state);
    m_view->updateContentLabel();
}

TEST_F(DFileViewTest,update_toolbar_actions)
{
    ASSERT_NE(nullptr,m_view);

    QActionGroup actions(nullptr);
    m_view->d_func()->toolbarActionGroup = &actions;
    m_view->updateToolBarActions(m_view, QString());
    EXPECT_FALSE(actions.actions().isEmpty());

    QAction *actionA = new QAction;
    QAction *actionB = new QAction;
    actions.addAction(actionA);
    actions.addAction(actionB);
    m_view->updateToolBarActions(m_view, QString());
    EXPECT_FALSE(actions.actions().isEmpty());
    actions.removeAction(actionA);
    actions.removeAction(actionB);
    actionA->deleteLater();
    actionB->deleteLater();
}

TEST_F(DFileViewTest,re_fresh_mode)
{
    ASSERT_NE(nullptr,m_view);

    m_view->refresh();
}

TEST_F(DFileViewTest,fetch_drag_memory)
{
    ASSERT_NE(nullptr,m_view);

    Stub stub;
    static bool myAttach = false;
    bool (*ut_isAttached)() = [](){return myAttach;};
    stub.set(ADDR(QSharedMemory, isAttached), ut_isAttached);
    stub.set(ADDR(QSharedMemory, attach), ut_isAttached);
    bool result = m_view->fetchDragEventUrlsFromSharedMemory();
    EXPECT_FALSE(result);

    myAttach = true;
    result = m_view->fetchDragEventUrlsFromSharedMemory();
    EXPECT_TRUE(result);

    stub.reset(ADDR(QSharedMemory, isAttached));
    stub.reset(ADDR(QSharedMemory, attach));
}

TEST_F(SelectWorkTest,set_init_data)
{
    ASSERT_NE(nullptr,m_selectWork);

    Stub stub;
    bool (*ut_isValutFile)() = [](){return true;};
    stub.set(ADDR(VaultController, isVaultFile), ut_isValutFile);

    DUrl (*ut_localToVault)() = [](){return DUrl("/home");};
    stub.set(ADDR(VaultController, localToVault), ut_localToVault);

    QList<DUrl> lst;
    lst.append(DUrl("/home"));

    m_selectWork->setInitData(lst, nullptr);
    EXPECT_EQ(m_selectWork->m_lstNoValid, lst);
    EXPECT_EQ(m_selectWork->m_pModel, nullptr);

    stub.reset(ADDR(VaultController, isVaultFile));
    stub.reset(ADDR(VaultController, localToVault));
}

TEST_F(SelectWorkTest,start_work)
{
    ASSERT_NE(nullptr,m_selectWork);

    Stub stub;
    static bool myCallStart = false;
    void (*ut_start)() = [](){myCallStart = true;};
    stub.set(ADDR(QThread, start), ut_start);

    m_selectWork->startWork();
    EXPECT_TRUE(myCallStart);
    EXPECT_FALSE(m_selectWork->m_bStop);

    stub.reset(ADDR(QThread, start));
}

TEST_F(SelectWorkTest,stop_work)
{
    ASSERT_NE(nullptr,m_selectWork);

    m_selectWork->stopWork();
    EXPECT_TRUE(m_selectWork->m_bStop);
}

TEST_F(SelectWorkTest,tst_run)
{
    ASSERT_NE(nullptr,m_selectWork);

    Stub stub;
    static bool myUpdate = false;
    void (*ut_update)() = [](){myUpdate = true;};
    stub.set(ADDR(DFileSystemModel, update), ut_update);

    static bool myValid = true;
    bool (*ut_isValid)() = [](){return myValid;};
    stub.set(ADDR(QModelIndex, isValid), ut_isValid);

    QList<DUrl> lst;
    lst.append(DUrl("/home"));
    lst.append(DUrl("/usr/bin"));
    m_selectWork->setInitData(lst, nullptr);

    m_selectWork->m_bStop = true;
    m_selectWork->run();
    EXPECT_FALSE(myUpdate);

    myUpdate = false;
    m_selectWork->m_bStop = false;
    m_selectWork->run();
    EXPECT_FALSE(myUpdate);

    myUpdate = false;
    m_selectWork->run();
    EXPECT_FALSE(myUpdate);

    stub.reset(ADDR(DFileSystemModel, update));
    stub.reset(ADDR(QModelIndex, isValid));
}
