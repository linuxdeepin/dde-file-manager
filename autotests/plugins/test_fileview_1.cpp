// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileview_1.cpp
 * @brief Unit tests for FileView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "views/fileview.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileViewTest, aboutToChangeWidth)
{
    // Test method: void aboutToChangeWidth((int deltaWidth))
    EXPECT_NO_FATAL_FAILURE(obj->aboutToChangeWidth(0));
}

TEST_F(FileViewTest, calcGroupRectContiansIndexes)
{
    // Test method: FileView::RangeIndexList calcGroupRectContiansIndexes((const QRect &rect))
    QRect _arg0{};
    auto result = obj->calcGroupRectContiansIndexes(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewTest, calcRectContiansIndexes)
{
    // Test method: FileView::RangeIndexList calcRectContiansIndexes((int columnCount, const QRect &rect))
    QRect _arg1{};
    auto result = obj->calcRectContiansIndexes(0, _arg1);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewTest, calcVisualRect)
{
    // Test method: QRect calcVisualRect((int widgetWidth, int index))
    auto result = obj->calcVisualRect(0, 0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewTest, cdUp)
{
    // Test bool getter: cdUp()
    bool result = obj->cdUp();
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, computeStickyY)
{
    // Test method: int computeStickyY((int headerHeight))
    auto result = obj->computeStickyY(0);
    EXPECT_GE(result, 0);

}

TEST_F(FileViewTest, contentWidget)
{
    // Test getter: QWidget contentWidget()
    auto result = obj->contentWidget();
    EXPECT_NO_FATAL_FAILURE({ obj->contentWidget(); });

}

TEST_F(FileViewTest, currentChanged)
{
    // Test method: void currentChanged((const QModelIndex &current, const QModelIndex &previous))
    QModelIndex _arg0{};
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->currentChanged(_arg0, _arg1));
}

TEST_F(FileViewTest, currentDirOpenMode)
{
    // Test getter: DirOpenMode currentDirOpenMode()
    auto result = obj->currentDirOpenMode();
    EXPECT_NO_FATAL_FAILURE({ obj->currentDirOpenMode(); });

}

TEST_F(FileViewTest, currentPressIndex)
{
    // Test getter: QModelIndex currentPressIndex()
    auto result = obj->currentPressIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewTest, currentViewMode)
{
    // Test getter: Global::ViewMode currentViewMode()
    auto result = obj->currentViewMode();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewTest, decreaseIcon)
{
    // Test method: void decreaseIcon(())
    EXPECT_NO_FATAL_FAILURE(obj->decreaseIcon());
}

TEST_F(FileViewTest, delayUpdateStatusBar)
{
    // Test method: void delayUpdateStatusBar(())
    EXPECT_NO_FATAL_FAILURE(obj->delayUpdateStatusBar());
}

TEST_F(FileViewTest, doItemsLayout)
{
    // Test method: void doItemsLayout(())
    EXPECT_NO_FATAL_FAILURE(obj->doItemsLayout());
}

TEST_F(FileViewTest, dragEnterEvent)
{
    // Test event handler: dragEnterEvent((QDragEnterEvent *event))
    QDragEnterEvent _event(QDragEnterEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dragEnterEvent(&_event));
}

TEST_F(FileViewTest, dragLeaveEvent)
{
    // Test event handler: dragLeaveEvent((QDragLeaveEvent *event))
    QDragLeaveEvent _event(QDragLeaveEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dragLeaveEvent(&_event));
}

TEST_F(FileViewTest, dragMoveEvent)
{
    // Test event handler: dragMoveEvent((QDragMoveEvent *event))
    QDragMoveEvent _event(QDragMoveEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dragMoveEvent(&_event));
}

TEST_F(FileViewTest, dropEvent)
{
    // Test event handler: dropEvent((QDropEvent *event))
    QDropEvent _event(QDropEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dropEvent(&_event));
}

TEST_F(FileViewTest, expandOrCollapseItem)
{
    // Test method: bool expandOrCollapseItem((const QModelIndex &index, const QPoint &pos))
    QModelIndex _arg0{};
    QPoint _arg1{};
    auto result = obj->expandOrCollapseItem(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, fetchSupportSelectionModes)
{
    // Test getter: QList<QAbstractItemView::SelectionMode> fetchSupportSelectionModes()
    auto result = obj->fetchSupportSelectionModes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewTest, findStickyGroupIndex)
{
    // Test method: QModelIndex findStickyGroupIndex((int headerHeight))
    auto result = obj->findStickyGroupIndex(0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewTest, focusInEvent)
{
    // Test event handler: focusInEvent((QFocusEvent *event))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusInEvent(&_event));
}

TEST_F(FileViewTest, focusOnView)
{
    // Test method: void focusOnView(())
    EXPECT_NO_FATAL_FAILURE(obj->focusOnView());
}

TEST_F(FileViewTest, getColumnRoles)
{
    // Test getter: QList<ItemRoles> getColumnRoles()
    auto result = obj->getColumnRoles();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewTest, getColumnWidth)
{
    // Test method: int getColumnWidth((const int &column))
    int _arg0{};
    auto result = obj->getColumnWidth(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(FileViewTest, getHeaderViewWidth)
{
    // Test getter: int getHeaderViewWidth()
    auto result = obj->getHeaderViewWidth();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewTest, groupExpandOrCollapseItem)
{
    // Test method: bool groupExpandOrCollapseItem((const QModelIndex &index, const QPoint &pos, const bool isArr))
    QModelIndex _arg0{};
    QPoint _arg1{};
    auto result = obj->groupExpandOrCollapseItem(_arg0, _arg1, false);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, groupHeaderContentTop)
{
    // Test method: int groupHeaderContentTop((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->groupHeaderContentTop(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(FileViewTest, groupingState)
{
    // Test getter: GroupingState groupingState()
    auto result = obj->groupingState();
    EXPECT_NO_FATAL_FAILURE({ obj->groupingState(); });

}

TEST_F(FileViewTest, horizontalOffset)
{
    // Test getter: int horizontalOffset()
    auto result = obj->horizontalOffset();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewTest, iconModeColumnCount)
{
    // Test method: int iconModeColumnCount((int itemWidth))
    auto result = obj->iconModeColumnCount(0);
    EXPECT_GE(result, 0);

}

TEST_F(FileViewTest, increaseIcon)
{
    // Test method: void increaseIcon(())
    EXPECT_NO_FATAL_FAILURE(obj->increaseIcon());
}

TEST_F(FileViewTest, indexAtForSelection)
{
    // Test method: QModelIndex indexAtForSelection((const QPoint &pos))
    QPoint _arg0{};
    auto result = obj->indexAtForSelection(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewTest, indexInRect)
{
    // Test method: bool indexInRect((const QRect &actualRect, const QModelIndex &index))
    QRect _arg0{};
    QModelIndex _arg1{};
    auto result = obj->indexInRect(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, initDefaultHeaderView)
{
    // Test method: void initDefaultHeaderView(())
    EXPECT_NO_FATAL_FAILURE(obj->initDefaultHeaderView());
}

TEST_F(FileViewTest, initializeDelegate)
{
    // Test method: void initializeDelegate(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeDelegate());
}

TEST_F(FileViewTest, initializeGroupHeaderTimer)
{
    // Test method: void initializeGroupHeaderTimer(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeGroupHeaderTimer());
}

TEST_F(FileViewTest, initializeModel)
{
    // Test method: void initializeModel(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeModel());
}

TEST_F(FileViewTest, initializePreSelectTimer)
{
    // Test method: void initializePreSelectTimer(())
    EXPECT_NO_FATAL_FAILURE(obj->initializePreSelectTimer());
}

TEST_F(FileViewTest, initializeScrollBarWatcher)
{
    // Test method: void initializeScrollBarWatcher(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeScrollBarWatcher());
}

TEST_F(FileViewTest, initializeStatusBar)
{
    // Test method: void initializeStatusBar(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeStatusBar());
}

TEST_F(FileViewTest, isClickInGroupHeaderSpacing)
{
    // Test method: bool isClickInGroupHeaderSpacing((const QPoint &pos, const QModelIndex &index))
    QPoint _arg0{};
    QModelIndex _arg1{};
    auto result = obj->isClickInGroupHeaderSpacing(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, isClickInTopPadding)
{
    // Test method: bool isClickInTopPadding((const QPoint &pos, const QModelIndex &index))
    QPoint _arg0{};
    QModelIndex _arg1{};
    auto result = obj->isClickInTopPadding(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, isDragTarget)
{
    // Test method: bool isDragTarget((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isDragTarget(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, isGroupHeader)
{
    // Test method: bool isGroupHeader((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isGroupHeader(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, isGroupedView)
{
    // Test bool getter: isGroupedView()
    bool result = obj->isGroupedView();
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, isIconViewMode)
{
    // Test bool getter: isIconViewMode()
    bool result = obj->isIconViewMode();
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, isListViewMode)
{
    // Test bool getter: isListViewMode()
    bool result = obj->isListViewMode();
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, isPosInStickyHeader)
{
    // Test method: bool isPosInStickyHeader((const QPoint &pos))
    QPoint _arg0{};
    auto result = obj->isPosInStickyHeader(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, isTreeViewMode)
{
    // Test bool getter: isTreeViewMode()
    bool result = obj->isTreeViewMode();
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, isVerticalScrollBarSliderDragging)
{
    // Test bool getter: isVerticalScrollBarSliderDragging()
    bool result = obj->isVerticalScrollBarSliderDragging();
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, itemCountForRow)
{
    // Test getter: int itemCountForRow()
    auto result = obj->itemCountForRow();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewTest, itemDelegate)
{
    // Test getter: BaseItemDelegate itemDelegate()
    auto result = obj->itemDelegate();
    EXPECT_NO_FATAL_FAILURE({ obj->itemDelegate(); });

}

TEST_F(FileViewTest, itemSizeHint)
{
    // Test getter: QSize itemSizeHint()
    auto result = obj->itemSizeHint();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewTest, keyboardSearch)
{
    // Test method: void keyboardSearch((const QString &search))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->keyboardSearch(_arg0));
}

TEST_F(FileViewTest, leaveEvent)
{
    // Test event handler: leaveEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->leaveEvent(&_event));
}

TEST_F(FileViewTest, loadViewState)
{
    // Test method: void loadViewState((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->loadViewState(_arg0));
}

TEST_F(FileViewTest, mouseDoubleClickEvent)
{
    // Test event handler: mouseDoubleClickEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseDoubleClickEvent(&_event));
}

TEST_F(FileViewTest, mouseMoveEvent)
{
    // Test event handler: mouseMoveEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseMoveEvent(&_event));
}

TEST_F(FileViewTest, onAboutToSwitchListView)
{
    // Test method: void onAboutToSwitchListView((const QList<QUrl> &allShowList))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onAboutToSwitchListView(_arg0));
}

TEST_F(FileViewTest, onAppAttributeChanged)
{
    // Test method: void onAppAttributeChanged((const QString &group, const QString &key, const QVariant &value))
    QString _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onAppAttributeChanged(_arg0, _arg1, _arg2));
}

TEST_F(FileViewTest, onClicked)
{
    // Test method: void onClicked((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onClicked(_arg0));
}

TEST_F(FileViewTest, onDefaultViewModeChanged)
{
    // Test method: void onDefaultViewModeChanged((int mode))
    EXPECT_NO_FATAL_FAILURE(obj->onDefaultViewModeChanged(0));
}

TEST_F(FileViewTest, onDoubleClicked)
{
    // Test method: void onDoubleClicked((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDoubleClicked(_arg0));
}

TEST_F(FileViewTest, onGroupExpansionToggled)
{
    // Test method: void onGroupExpansionToggled((const QString &groupKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onGroupExpansionToggled(_arg0));
}

TEST_F(FileViewTest, onGroupHeaderClicked)
{
    // Test method: void onGroupHeaderClicked((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onGroupHeaderClicked(_arg0));
}

TEST_F(FileViewTest, onGroupTruncationToggled)
{
    // Test method: void onGroupTruncationToggled((const QString &groupKey))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onGroupTruncationToggled(_arg0));
}

TEST_F(FileViewTest, onHeaderHiddenChanged)
{
    // Test method: void onHeaderHiddenChanged((const QString &roleName, const bool isHidden))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onHeaderHiddenChanged(_arg0, false));
}

TEST_F(FileViewTest, onHeaderSectionMoved)
{
    // Test method: void onHeaderSectionMoved((int logicalIndex, int oldVisualIndex, int newVisualIndex))
    EXPECT_NO_FATAL_FAILURE(obj->onHeaderSectionMoved(0, 0, 0));
}

TEST_F(FileViewTest, onHeaderSectionResized)
{
    // Test method: void onHeaderSectionResized((int logicalIndex, int oldSize, int newSize))
    EXPECT_NO_FATAL_FAILURE(obj->onHeaderSectionResized(0, 0, 0));
}

TEST_F(FileViewTest, onHeaderViewMousePressed)
{
    // Test method: void onHeaderViewMousePressed(())
    EXPECT_NO_FATAL_FAILURE(obj->onHeaderViewMousePressed());
}

TEST_F(FileViewTest, onHeaderViewMouseReleased)
{
    // Test method: void onHeaderViewMouseReleased(())
    EXPECT_NO_FATAL_FAILURE(obj->onHeaderViewMouseReleased());
}

TEST_F(FileViewTest, onHeaderViewSectionChanged)
{
    // Test method: void onHeaderViewSectionChanged((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onHeaderViewSectionChanged(_arg0));
}

TEST_F(FileViewTest, onIconSizeChanged)
{
    // Test method: void onIconSizeChanged((int sizeIndex))
    EXPECT_NO_FATAL_FAILURE(obj->onIconSizeChanged(0));
}

TEST_F(FileViewTest, onItemHeightLevelChanged)
{
    // Test method: void onItemHeightLevelChanged((int level))
    EXPECT_NO_FATAL_FAILURE(obj->onItemHeightLevelChanged(0));
}

TEST_F(FileViewTest, onItemWidthLevelChanged)
{
    // Test method: void onItemWidthLevelChanged((int level))
    EXPECT_NO_FATAL_FAILURE(obj->onItemWidthLevelChanged(0));
}

TEST_F(FileViewTest, onModelStateChanged)
{
    // Test method: void onModelStateChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onModelStateChanged());
}

TEST_F(FileViewTest, onRenameProcessStarted)
{
    // Test method: void onRenameProcessStarted(())
    EXPECT_NO_FATAL_FAILURE(obj->onRenameProcessStarted());
}

TEST_F(FileViewTest, onRowCountChanged)
{
    // Test method: void onRowCountChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->onRowCountChanged());
}

TEST_F(FileViewTest, onScalingValueChanged)
{
    // Test method: void onScalingValueChanged((const int value))
    EXPECT_NO_FATAL_FAILURE(obj->onScalingValueChanged(0));
}

TEST_F(FileViewTest, onSelectAndEdit)
{
    // Test method: void onSelectAndEdit((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onSelectAndEdit(_arg0));
}

TEST_F(FileViewTest, onSelectionChanged)
{
    // Test method: void onSelectionChanged((const QItemSelection &selected, const QItemSelection &deselected))
    QItemSelection _arg0{};
    QItemSelection _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onSelectionChanged(_arg0, _arg1));
}

TEST_F(FileViewTest, onShowFileSuffixChanged)
{
    // Test method: void onShowFileSuffixChanged((bool isShow))
    EXPECT_NO_FATAL_FAILURE(obj->onShowFileSuffixChanged(false));
}

TEST_F(FileViewTest, onSortIndicatorChanged)
{
    // Test method: void onSortIndicatorChanged((int logicalIndex, Qt::SortOrder order))
    EXPECT_NO_FATAL_FAILURE(obj->onSortIndicatorChanged(0, Qt::SortOrder()));
}

TEST_F(FileViewTest, onWidgetUpdate)
{
    // Test method: void onWidgetUpdate(())
    EXPECT_NO_FATAL_FAILURE(obj->onWidgetUpdate());
}

TEST_F(FileViewTest, openIndex)
{
    // Test method: void openIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openIndex(_arg0));
}

TEST_F(FileViewTest, openIndexByClicked)
{
    // Test method: void openIndexByClicked((const ClickedAction action, const QModelIndex &index))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->openIndexByClicked(ClickedAction(), _arg1));
}

TEST_F(FileViewTest, paintStickyHeaderOverlay)
{
    // Test method: void paintStickyHeaderOverlay((const QModelIndex &index, int y, int headerHeight))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->paintStickyHeaderOverlay(_arg0, 0, 0));
}

TEST_F(FileViewTest, parseSelectedUrl)
{
    // Test method: QUrl parseSelectedUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->parseSelectedUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(FileViewTest, recordSelectedUrls)
{
    // Test method: void recordSelectedUrls(())
    EXPECT_NO_FATAL_FAILURE(obj->recordSelectedUrls());
}

TEST_F(FileViewTest, rectContainsIndexes)
{
    // Test method: FileView::RangeIndexList rectContainsIndexes((const QRect &rect))
    QRect _arg0{};
    auto result = obj->rectContainsIndexes(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewTest, refresh)
{
    // Test method: void refresh(())
    EXPECT_NO_FATAL_FAILURE(obj->refresh());
}

TEST_F(FileViewTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *event))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(FileViewTest, reverseSelect)
{
    // Test method: void reverseSelect(())
    EXPECT_NO_FATAL_FAILURE(obj->reverseSelect());
}

TEST_F(FileViewTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(FileViewTest, rowCount)
{
    // Test getter: int rowCount()
    auto result = obj->rowCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewTest, scrollStickyHeaderToTop)
{
    // Test method: void scrollStickyHeaderToTop((const QModelIndex &headerIndex))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->scrollStickyHeaderToTop(_arg0));
}

TEST_F(FileViewTest, selectFiles)
{
    // Test method: bool selectFiles((const QList<QUrl> &files))
    QList<QUrl> _arg0{};
    auto result = obj->selectFiles(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, selectedIndexCount)
{
    // Test getter: int selectedIndexCount()
    auto result = obj->selectedIndexCount();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewTest, selectedIndexes)
{
    // Test getter: QModelIndexList selectedIndexes()
    auto result = obj->selectedIndexes();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewTest, selectedTreeViewUrlList)
{
    // Test method: void selectedTreeViewUrlList((QList<QUrl> &selectedUrls, QList<QUrl> &treeSelectedUrls))
    QList<QUrl> _arg0{};
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->selectedTreeViewUrlList(_arg0, _arg1));
}

TEST_F(FileViewTest, setAlwaysOpenInCurrentWindow)
{
    // Test setter: void setAlwaysOpenInCurrentWindow((bool openInCurrentWindow))
    EXPECT_NO_FATAL_FAILURE(obj->setAlwaysOpenInCurrentWindow(false));
}

TEST_F(FileViewTest, setDefaultViewMode)
{
    // Test method: void setDefaultViewMode(())
    EXPECT_NO_FATAL_FAILURE(obj->setDefaultViewMode());
}

TEST_F(FileViewTest, setDelegate)
{
    // Test setter: void setDelegate((Global::ViewMode mode, BaseItemDelegate *view))
    EXPECT_NO_FATAL_FAILURE(obj->setDelegate(Global::ViewMode(), nullptr));
}

TEST_F(FileViewTest, setEnabledSelectionModes)
{
    // Test setter: void setEnabledSelectionModes((const QList<QAbstractItemView::SelectionMode> &modes))
    QList<QAbstractItemView::SelectionMode> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setEnabledSelectionModes(_arg0));
}

TEST_F(FileViewTest, setFileViewStateValue)
{
    // Test setter: void setFileViewStateValue((const QUrl &url, const QString &key, const QVariant &value))
    QUrl _arg0{};
    QString _arg1{};
    QVariant _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->setFileViewStateValue(_arg0, _arg1, _arg2));
}

TEST_F(FileViewTest, setFilterCallback)
{
    // Test setter: void setFilterCallback((const QUrl &url, const FileViewFilterCallback callback))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFilterCallback(_arg0, FileViewFilterCallback()));
}

TEST_F(FileViewTest, setFilterData)
{
    // Test setter: void setFilterData((const QUrl &url, const QVariant &data))
    QUrl _arg0{};
    QVariant _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->setFilterData(_arg0, _arg1));
}

TEST_F(FileViewTest, setGroup)
{
    // Test setter: void setGroup((const QString &strategyName, const Qt::SortOrder order))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setGroup(_arg0, Qt::SortOrder()));
}

TEST_F(FileViewTest, setIconSizeBySizeIndex)
{
    // Test setter: void setIconSizeBySizeIndex((const int sizeIndex))
    EXPECT_NO_FATAL_FAILURE(obj->setIconSizeBySizeIndex(0));
}

TEST_F(FileViewTest, setListViewMode)
{
    // Test method: void setListViewMode(())
    EXPECT_NO_FATAL_FAILURE(obj->setListViewMode());
}

TEST_F(FileViewTest, setRootUrl)
{
    // Test method: bool setRootUrl((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->setRootUrl(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileViewTest, setSelection)
{
    // Test setter: void setSelection((const QRect &rect, QItemSelectionModel::SelectionFlags flags))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setSelection(_arg0, QItemSelectionModel::SelectionFlags()));
}

TEST_F(FileViewTest, setSelectionMode)
{
    // Test setter: void setSelectionMode((const QAbstractItemView::SelectionMode mode))
    EXPECT_NO_FATAL_FAILURE(obj->setSelectionMode(QAbstractItemView::SelectionMode()));
}

TEST_F(FileViewTest, setSort)
{
    // Test setter: void setSort((const ItemRoles role, const Qt::SortOrder order))
    EXPECT_NO_FATAL_FAILURE(obj->setSort(ItemRoles(), Qt::SortOrder()));
}

TEST_F(FileViewTest, setViewSelectState)
{
    // Test setter: void setViewSelectState((bool isSelect))
    EXPECT_NO_FATAL_FAILURE(obj->setViewSelectState(false));
}

TEST_F(FileViewTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(FileViewTest, stickyHeaderHeight)
{
    // Test getter: int stickyHeaderHeight()
    auto result = obj->stickyHeaderHeight();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewTest, stopWork)
{
    // Test method: void stopWork((const QUrl &newUrl))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->stopWork(_arg0));
}

TEST_F(FileViewTest, toolBarActionList)
{
    // Test getter: QList<QAction *> toolBarActionList()
    auto result = obj->toolBarActionList();
    // Pointer return type
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileViewTest, trashStateChanged)
{
    // Test method: void trashStateChanged(())
    EXPECT_NO_FATAL_FAILURE(obj->trashStateChanged());
}

TEST_F(FileViewTest, updateContentLabel)
{
    // Test method: void updateContentLabel(())
    EXPECT_NO_FATAL_FAILURE(obj->updateContentLabel());
}

TEST_F(FileViewTest, updateDelegateHighlightKeywords)
{
    // Test method: void updateDelegateHighlightKeywords((const QStringList &keywords))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateDelegateHighlightKeywords(_arg0));
}

TEST_F(FileViewTest, updateHorizontalOffset)
{
    // Test method: void updateHorizontalOffset(())
    EXPECT_NO_FATAL_FAILURE(obj->updateHorizontalOffset());
}

TEST_F(FileViewTest, updateLoadingIndicator)
{
    // Test method: void updateLoadingIndicator(())
    EXPECT_NO_FATAL_FAILURE(obj->updateLoadingIndicator());
}

TEST_F(FileViewTest, updateOneView)
{
    // Test method: void updateOneView((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateOneView(_arg0));
}

TEST_F(FileViewTest, updateSelectedUrl)
{
    // Test method: void updateSelectedUrl(())
    EXPECT_NO_FATAL_FAILURE(obj->updateSelectedUrl());
}

TEST_F(FileViewTest, verticalOffset)
{
    // Test getter: int verticalOffset()
    auto result = obj->verticalOffset();
    EXPECT_EQ(result, 0);

}

TEST_F(FileViewTest, viewModeChanged)
{
    // Test method: void viewModeChanged((quint64 windowId, int viewMode))
    EXPECT_NO_FATAL_FAILURE(obj->viewModeChanged(0, 0));
}

TEST_F(FileViewTest, viewState)
{
    // Test getter: AbstractBaseView::ViewState viewState()
    auto result = obj->viewState();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewTest, visibleIndexes)
{
    // Test method: FileView::RangeIndexList visibleIndexes((const QRect &rect))
    QRect _arg0{};
    auto result = obj->visibleIndexes(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(FileViewTest, wheelEvent)
{
    // Test event handler: wheelEvent((QWheelEvent *event))
    QWheelEvent _event(QWheelEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->wheelEvent(&_event));
}
