/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "headerview.h"
#include "fileview.h"
#include "private/fileview_p.h"
#include "models/filesortfilterproxymodel.h"
#include "models/fileselectionmodel.h"
#include "models/fileviewmodel.h"
#include "baseitemdelegate.h"
#include "iconitemdelegate.h"
#include "listitemdelegate.h"
#include "fileviewstatusbar.h"
#include "events/workspaceeventcaller.h"
#include "utils/workspacehelper.h"
#include "utils/fileviewhelper.h"
#include "utils/dragdrophelper.h"
#include "utils/viewdrawhelper.h"
#include "utils/selecthelper.h"
#include "utils/shortcuthelper.h"
#include "utils/fileviewmenuhelper.h"
#include "utils/fileoperatorhelper.h"
#include "events/workspaceeventsequence.h"
#include "views/private/delegatecommon.h"

#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/utils/windowutils.h"

#include <QResizeEvent>
#include <QScrollBar>
#include <QScroller>
#include <QTimer>
#include <QDrag>

DPWORKSPACE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

FileView::FileView(const QUrl &url, QWidget *parent)
    : DListView(parent), d(new FileViewPrivate(this))
{
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QListView::EditKeyPressed | QListView::SelectedClicked);
    setTextElideMode(Qt::ElideMiddle);
    setAlternatingRowColors(false);
    setSelectionRectVisible(true);
    setDefaultDropAction(Qt::CopyAction);
    setDragDropOverwriteMode(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setDragEnabled(true);

    initializeModel();
    initializeDelegate();
    initializeStatusBar();
    initializeConnect();

    setRootUrl(url);
}

FileView::~FileView()
{
    disconnect(sourceModel(), &FileViewModel::updateFiles, this, &FileView::updateView);
    disconnect(sourceModel(), &FileViewModel::stateChanged, this, &FileView::onModelStateChanged);
    disconnect(sourceModel(), &FileViewModel::modelChildrenUpdated, this, &FileView::onChildrenChanged);
    disconnect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileView::onSelectionChanged);
}

QWidget *FileView::widget() const
{
    return const_cast<FileView *>(this);
}

void FileView::setViewMode(Global::ViewMode mode)
{
    if (itemDelegate())
        itemDelegate()->hideAllIIndexWidget();

    setItemDelegate(d->delegates[static_cast<int>(mode)]);

    switch (mode) {
    case Global::ViewMode::kIconMode:
        setUniformItemSizes(false);
        setResizeMode(Adjust);
        setOrientation(QListView::LeftToRight, true);
        setSpacing(kIconViewSpacing);

        d->initIconModeView();
        break;
    case Global::ViewMode::kListMode:
        setUniformItemSizes(true);
        setResizeMode(Fixed);
        setOrientation(QListView::TopToBottom, false);
        setSpacing(kListViewSpacing);

        if (model())
            setMinimumWidth(model()->columnCount() * GlobalPrivate::kListViewMinimumWidth);
        d->initListModeView();

        if (d->allowedAdjustColumnSize) {
            horizontalScrollBar()->parentWidget()->installEventFilter(this);

            d->cachedViewWidth = this->width();
            d->adjustFileNameColumn = true;
            updateListHeaderView();
        }
        break;
    case Global::ViewMode::kExtendMode:
        break;
    case Global::ViewMode::kAllViewMode:
        break;
    default:
        break;
    }

    d->currentViewMode = mode;
}

Global::ViewMode FileView::currentViewMode()
{
    return d->currentViewMode;
}

void FileView::setDelegate(Global::ViewMode mode, BaseItemDelegate *view)
{
    if (!view)
        return;

    auto delegate = d->delegates[static_cast<int>(mode)];
    if (delegate) {
        if (delegate->parent())
            delegate->setParent(nullptr);
        delete delegate;
    }

    d->delegates[static_cast<int>(mode)] = view;
}

bool FileView::setRootUrl(const QUrl &url)
{
    clearSelection();

    //Todo(yanghao&lzj):!url.isSearchFile()
    setFocus();

    model()->setRootUrl(url);

    loadViewState(url);
    delayUpdateStatusBar();
    setDefaultViewMode();

    resetSelectionModes();
    updateListHeaderView();

    if (d->sortTimer)
        d->sortTimer->start();

    return true;
}

QUrl FileView::rootUrl() const
{
    return model()->rootUrl();
}

AbstractBaseView::ViewState FileView::viewState() const
{
    // TODO(liuyangming): return model state
    return AbstractBaseView::viewState();
}

QList<QAction *> FileView::toolBarActionList() const
{
    // TODO(liuyangming): impl me
    return QList<QAction *>();
}

QList<QUrl> FileView::selectedUrlList() const
{
    QModelIndex rootIndex = this->rootIndex();
    QList<QUrl> list;

    for (const QModelIndex &index : selectedIndexes()) {
        if (index.parent() != rootIndex)
            continue;
        list << model()->getUrlByIndex(index);
    }

    return list;
}

void FileView::refresh()
{
    model()->fetchMore(rootIndex());
}

FileSortFilterProxyModel *FileView::model() const
{
    return qobject_cast<FileSortFilterProxyModel *>(QAbstractItemView::model());
}

void FileView::setModel(QAbstractItemModel *model)
{
    if (model->parent() != this)
        model->setParent(this);
    auto curr = FileView::model();
    if (curr)
        delete curr;
    DListView::setModel(model);
}

void FileView::stopWork()
{
    auto model = sourceModel();
    if (model)
        model->stopTraversWork();
}

int FileView::getColumnWidth(const int &column) const
{
    if (d->headerView)
        return d->headerView->sectionSize(column);

    return GlobalPrivate::kListViewDefaultWidth;
}

int FileView::getHeaderViewWidth() const
{
    if (d->headerView)
        return d->headerView->length();

    return 0;
}

void FileView::setAlwaysOpenInCurrentWindow(bool openInCurrentWindow)
{
    // for dialog
    d->isAlwaysOpenInCurrentWindow = openInCurrentWindow;
}

void FileView::onHeaderViewMouseReleased()
{
    if (d->headerView->width() != width()) {
        d->adjustFileNameColumn = false;
    }

    QList<ItemRoles> roleList = d->columnRoles;
    QVariantMap state;
    for (const ItemRoles role : roleList) {
        int colWidth = getColumnWidth(model()->getColumnByRole(role));

        if (colWidth > 0)
            state[QString::number(role)] = colWidth;
    }
    Application::appObtuselySetting()->setValue("WindowManager", "ViewColumnState", state);
}

void FileView::onHeaderSectionResized(int logicalIndex, int oldSize, int newSize)
{
    Q_UNUSED(logicalIndex)
    Q_UNUSED(oldSize)
    Q_UNUSED(newSize)

    d->headerView->adjustSize();
    updateGeometries();
    update();
}

void FileView::onSectionHandleDoubleClicked(int logicalIndex)
{
    if (sourceModel()->state() != FileViewModel::Idle)
        return;

    int rowCount = model()->rowCount();

    if (rowCount < 1)
        return;

    QStyleOptionViewItem option = viewOptions();

    option.rect.setWidth(QWIDGETSIZE_MAX);
    option.rect.setHeight(itemSizeHint().height());

    int columnMaxWidth = 0;

    for (int i = 0; i < rowCount; ++i) {
        const QModelIndex &index = model()->index(i, 0);
        const QList<QRect> &list = itemDelegate()->paintGeomertys(option, index, true);

        // 第0列为文件名列，此列比较特殊，因为前面还有文件图标占用了一部分空间
        int width = 0;

        if (logicalIndex == 0) {
            width = list.at(1).right() + kColumnPadding / 2;
        } else {
            width = list.at(logicalIndex + 1).width() + kColumnPadding * 2;
        }

        if (width > columnMaxWidth) {
            columnMaxWidth = width;
        }
    }

    for (int i = d->headerView->count() - 1; i >= 0; --i) {
        if (d->headerView->isSectionHidden(i))
            continue;

        // 最后一列要多加上视图的右margin
        if (i == logicalIndex)
            columnMaxWidth += kListModeRightMargin;

        break;
    }

    d->headerView->resizeSection(logicalIndex, columnMaxWidth);
}

void FileView::onHeaderSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex)
{
    Q_UNUSED(logicalIndex)
    Q_UNUSED(oldVisualIndex)
    Q_UNUSED(newVisualIndex)

    QVariantList logicalIndexList;
    for (int i = 0; i < d->headerView->count(); ++i) {
        int logicalIndex = d->headerView->logicalIndex(i);
        logicalIndexList << model()->getRoleByColumn(logicalIndex);
    }

    QUrl rootUrl = this->rootUrl();

    setFileViewStateValue(rootUrl, "headerList", logicalIndexList);
    // sync data to config file.
    Application::appObtuselySetting()->sync();

    // refresh
    updateListHeaderView();
    update();
}

void FileView::onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    model()->setSortRole(model()->getRoleByColumn(logicalIndex));
    model()->sort(logicalIndex, order);

    const QUrl &url = rootUrl();

    setFileViewStateValue(url, "sortRole", model()->getRoleByColumn(logicalIndex));
    setFileViewStateValue(url, "sortOrder", static_cast<int>(order));
}

void FileView::onClicked(const QModelIndex &index)
{
    openIndexByClicked(ClickedAction::kClicked, index);
}

void FileView::onDoubleClicked(const QModelIndex &index)
{
    openIndexByClicked(ClickedAction::kDoubleClicked, index);
}

void FileView::wheelEvent(QWheelEvent *event)
{
    if (isIconViewMode()) {
        if (WindowUtils::keyCtrlIsPressed()) {
            if (event->angleDelta().y() > 0) {
                increaseIcon();
            } else {
                decreaseIcon();
            }
            emit viewStateChanged();
            event->accept();
        } else {
            verticalScrollBar()->setSliderPosition(verticalScrollBar()->sliderPosition() - event->angleDelta().y());
        }
    } else if (event->modifiers() == Qt::AltModifier) {
        horizontalScrollBar()->setSliderPosition(horizontalScrollBar()->sliderPosition() - event->angleDelta().x());
    } else {
        verticalScrollBar()->setSliderPosition(verticalScrollBar()->sliderPosition() - event->angleDelta().y());
    }
}

void FileView::keyPressEvent(QKeyEvent *event)
{
    if (!d->shortcutHelper->processKeyPressEvent(event)) {
        switch (event->modifiers()) {
        case Qt::AltModifier:
        case Qt::AltModifier | Qt::KeypadModifier:
            switch (event->key()) {
            case Qt::Key_Left:
            case Qt::Key_Right:
                return QWidget::keyPressEvent(event);
            }
        }
        return DListView::keyPressEvent(event);
    }
}

void FileView::onScalingValueChanged(const int value)
{
    qobject_cast<IconItemDelegate *>(itemDelegate())->setIconSizeByIconSizeLevel(value);
    setFileViewStateValue(rootUrl(), "iconSizeLevel", value);
}

void FileView::delayUpdateStatusBar()
{
    if (d->updateStatusBarTimer)
        d->updateStatusBarTimer->start();
}

void FileView::viewModeChanged(quint64 windowId, int viewMode)
{
    auto thisWindId = WorkspaceHelper::instance()->windowId(this);
    Global::ViewMode mode = static_cast<Global::ViewMode>(viewMode);
    if (thisWindId == windowId) {
        if (mode == Global::ViewMode::kIconMode) {
            setViewModeToIcon();
        } else if (mode == Global::ViewMode::kListMode) {
            setViewModeToList();
        }

        setFocus();
        saveViewModeState();
    }
}

void FileView::updateModelActiveIndex()
{
    const RandeIndexList randeList = visibleIndexes(QRect(QPoint(0, verticalScrollBar()->value()), QSize(size())));

    if (randeList.isEmpty()) {
        return;
    }

    const RandeIndex &rande = randeList.first();
    AbstractFileWatcherPointer fileWatcher = sourceModel()->fileWatcher();

    for (int i = d->visibleIndexRande.first; i < rande.first; ++i) {
        const AbstractFileInfoPointer &fileInfo = model()->itemFileInfo(model()->index(i, 0));

        if (fileInfo && fileWatcher)
            fileWatcher->setEnabledSubfileWatcher(fileInfo->url(), false);
    }

    for (int i = rande.second; i < d->visibleIndexRande.second; ++i) {
        const AbstractFileInfoPointer &fileInfo = model()->itemFileInfo(model()->index(i, 0));

        if (fileInfo && fileWatcher) {
            fileWatcher->setEnabledSubfileWatcher(fileInfo->url(), false);
        }
    }

    d->visibleIndexRande = rande;
    for (int i = rande.first; i <= rande.second; ++i) {
        const AbstractFileInfoPointer &fileInfo = model()->itemFileInfo(model()->index(i, 0));

        if (fileInfo) {
            if (!fileInfo->exists()) {
                model()->removeRow(i, rootIndex());
            } else if (fileWatcher) {
                fileWatcher->setEnabledSubfileWatcher(fileInfo->url());
            }
        }
    }
}

FileView::RandeIndexList FileView::visibleIndexes(QRect rect) const
{
    RandeIndexList list;

    QSize itemSize = itemSizeHint();
    QSize aIconSize = iconSize();

    int count = this->count();
    int spacing = this->spacing();
    int itemWidth = itemSize.width() + spacing * 2;
    int itemHeight = itemSize.height() + spacing * 2;

    if (isListViewMode()) {
        list << RandeIndex(qMax((rect.top() + spacing) / itemHeight, 0),
                           qMin((rect.bottom() - spacing) / itemHeight, count - 1));
    } else if (isIconViewMode()) {
        rect -= QMargins(spacing, spacing, spacing, spacing);

        int columnCount = d->iconModeColumnCount(itemWidth);

        if (columnCount <= 0)
            return list;

        int beginRowIndex = rect.top() / itemHeight;
        int endRowIndex = rect.bottom() / itemHeight;
        int beginColumnIndex = rect.left() / itemWidth;
        int endColumnIndex = rect.right() / itemWidth;

        if (rect.top() % itemHeight > aIconSize.height())
            ++beginRowIndex;

        int iconMargin = (itemWidth - aIconSize.width()) / 2;

        if (rect.left() % itemWidth > itemWidth - iconMargin)
            ++beginColumnIndex;

        if (rect.right() % itemWidth < iconMargin)
            --endColumnIndex;

        beginRowIndex = qMax(beginRowIndex, 0);
        beginColumnIndex = qMax(beginColumnIndex, 0);
        endRowIndex = qMin(endRowIndex, count / columnCount);
        endColumnIndex = qMin(endColumnIndex, columnCount - 1);

        if (beginRowIndex > endRowIndex || beginColumnIndex > endColumnIndex)
            return list;

        int beginIndex = beginRowIndex * columnCount;

        if (endColumnIndex - beginColumnIndex + 1 == columnCount) {
            list << RandeIndex(qMax(beginIndex, 0), qMin((endRowIndex + 1) * columnCount - 1, count - 1));

            return list;
        }

        for (int i = beginRowIndex; i <= endRowIndex; ++i) {
            if (beginIndex + beginColumnIndex >= count)
                break;

            list << RandeIndex(qMax(beginIndex + beginColumnIndex, 0),
                               qMin(beginIndex + endColumnIndex, count - 1));

            beginIndex += columnCount;
        }
    }

    return list;
}

BaseItemDelegate *FileView::itemDelegate() const
{
    return qobject_cast<BaseItemDelegate *>(DListView::itemDelegate());
}

int FileView::rowCount() const
{
    int count = this->count();
    int itemCountForRow = this->itemCountForRow();

    return count / itemCountForRow + int(count % itemCountForRow > 0);
}

bool FileView::isSelected(const QModelIndex &index) const
{
    return static_cast<FileSelectionModel *>(selectionModel())->isSelected(index);
}

int FileView::selectedIndexCount() const
{
    return static_cast<FileSelectionModel *>(selectionModel())->selectedCount();
}

void FileView::selectFiles(const QList<QUrl> &files) const
{
    d->selectHelper->select(files);
}

void FileView::setSelectionMode(const QAbstractItemView::SelectionMode mode)
{
    if (d->enabledSelectionModes.contains(mode))
        QAbstractItemView::setSelectionMode(mode);
}

void FileView::setEnabledSelectionModes(const QList<QAbstractItemView::SelectionMode> &modes)
{
    d->enabledSelectionModes = modes;
    if (!modes.contains(selectionMode()))
        resetSelectionModes();
}

void FileView::setSort(const ItemRoles role, const Qt::SortOrder order)
{
    if (role == model()->sortRole() && order == model()->sortOrder())
        return;

    int column = model()->getColumnByRole(role);

    if (d->headerView)
        d->headerView->setSortIndicator(column, order);
}

QModelIndex FileView::currentPressIndex() const
{
    return d->selectHelper->getCurrentPressedIndex();
}

bool FileView::isDragTarget(const QModelIndex &index) const
{
    return d->currentDragHoverIndex == index;
}

QRectF FileView::itemRect(const QUrl &url, const ItemRoles role) const
{
    QModelIndex index = model()->getIndexByUrl(url);

    switch (role) {
    case kItemIconRole: {
        QRectF rect = visualRect(index);
        return itemDelegate()->itemIconRect(rect);
    }
    case kItemBackgroundRole: {
        QRectF rect = visualRect(index);
        return rect;
    }
    default:
        return QRectF();
    }
}

void FileView::setNameFilters(const QStringList &filters)
{
    model()->setNameFilters(filters);
}

void FileView::setFilters(const QDir::Filters filters)
{
    model()->setFilters(filters);
}

QDir::Filters FileView::getFilters()
{
    return model()->getFilters();
}

void FileView::setReadOnly(const bool readOnly)
{
    model()->setReadOnly(readOnly);
}

int FileView::itemCountForRow() const
{

    if (!isIconViewMode())
        return 1;

    return d->iconModeColumnCount();
}

QSize FileView::itemSizeHint() const
{
    if (itemDelegate())
        return itemDelegate()->sizeHint(viewOptions(), rootIndex());

    return QSize();
}

void FileView::increaseIcon()
{
    int level = itemDelegate()->increaseIcon();
    if (level >= 0)
        setIconSizeBySizeIndex(level);
}

void FileView::decreaseIcon()
{
    int level = itemDelegate()->decreaseIcon();
    if (level >= 0)
        setIconSizeBySizeIndex(level);
}

void FileView::setIconSizeBySizeIndex(const int sizeIndex)
{
    QSignalBlocker blocker(d->statusBar->scalingSlider());
    Q_UNUSED(blocker)

    d->statusBar->scalingSlider()->setValue(sizeIndex);
    itemDelegate()->setIconSizeByIconSizeLevel(sizeIndex);
}

void FileView::onShowHiddenFileChanged(bool isShow)
{
    auto filters = model()->getFilters();
    if (isShow) {
        filters |= QDir::Hidden;
    } else {
        filters &= ~QDir::Hidden;
    }

    model()->setFilters(filters);
}

void FileView::onShowFileSuffixChanged(bool isShow)
{
    Q_UNUSED(isShow);
    update();
}

void FileView::updateHorizontalOffset()
{
    if (isIconViewMode()) {
        int contentWidth = maximumViewportSize().width();
        int itemWidth = itemSizeHint().width() + spacing() * 2;
        int itemColumn = d->iconModeColumnCount(itemWidth);

        d->horizontalOffset = -(contentWidth - itemWidth * itemColumn) / 2;
    } else {
        d->horizontalOffset = 0;
    }
}

void FileView::updateView()
{
    viewport()->update();
}

void FileView::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    delayUpdateStatusBar();

    // TODO(lixiang):  need remove from workspace
    setDetailFileUrl(selected, deselected);

    quint64 winId = WorkspaceHelper::instance()->windowId(this);
    WorkspaceEventCaller::sendViewSelectionChanged(winId, selected, deselected);
}

bool FileView::isIconViewMode() const
{
    return d->currentViewMode == Global::ViewMode::kIconMode;
}

bool FileView::isListViewMode() const
{
    return d->currentViewMode == Global::ViewMode::kListMode;
}

void FileView::resetSelectionModes()
{
    const QList<SelectionMode> &supportSelectionModes = fetchSupportSelectionModes();

    for (SelectionMode mode : supportSelectionModes) {
        if (d->enabledSelectionModes.contains(mode)) {
            setSelectionMode(mode);
            break;
        }
    }
}

QList<QAbstractItemView::SelectionMode> FileView::fetchSupportSelectionModes()
{
    QList<SelectionMode> modes {};
    WorkspaceEventSequence::instance()->doFetchSelectionModes(rootUrl(), &modes);

    if (modes.isEmpty())
        modes << ExtendedSelection << SingleSelection << MultiSelection
              << ContiguousSelection << NoSelection;

    return modes;
}

bool FileView::cdUp()
{
    // Todo(yanghao):
    const QUrl &oldCurrentUrl = rootUrl();
    QUrl parentUrl = UrlRoute::urlParent(oldCurrentUrl);

    if (parentUrl.isValid()) {
        FileOperatorHelperIns->openFilesByMode(this, { parentUrl }, DirOpenMode::kOpenInCurrentWindow);
        return true;
    } else {
        auto windowId = WorkspaceHelper::instance()->windowId(this);
        QUrl computerRoot;
        computerRoot.setScheme(Global::kComputer);
        computerRoot.setPath("/");
        WorkspaceEventCaller::sendChangeCurrentUrl(windowId, computerRoot);
    }
    return false;
}

DirOpenMode FileView::currentDirOpenMode() const
{
    DirOpenMode mode;

    if (d->isAlwaysOpenInCurrentWindow) {
        mode = DirOpenMode::kOpenInCurrentWindow;
    } else {
        if (Application::instance()->appAttribute(Application::kAllwayOpenOnNewWindow).toBool()) {
            mode = DirOpenMode::kOpenNewWindow;
        } else {
            mode = DirOpenMode::kOpenInCurrentWindow;
        }
    }
    return mode;
}

void FileView::onRowCountChanged()
{
    delayUpdateStatusBar();
    updateModelActiveIndex();
}

void FileView::onChildrenChanged()
{
    QList<QUrl> addedFiles = sourceModel()->takeAddedFiles();
    if (!addedFiles.isEmpty())
        selectFiles(addedFiles);

    updateContentLabel();
    delayUpdateStatusBar();
}

void FileView::setFilterData(const quint64 windowID, const QUrl &url, const QVariant &data)
{
    auto thisWindId = WorkspaceHelper::instance()->windowId(this);
    if (thisWindId == windowID && url == rootUrl() && isVisible()) {
        clearSelection();
        model()->setFilterData(data);
        update();
    }
}

void FileView::setFilterCallback(const quint64 windowID, const QUrl &url, const FileViewFilterCallback callback)
{
    auto thisWindId = WorkspaceHelper::instance()->windowId(this);
    if (thisWindId == windowID && url == rootUrl() && isVisible()) {
        clearSelection();
        model()->setFilterCallBack(callback);
        update();
    }
}

bool FileView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    return DListView::edit(index, trigger, event);
}

void FileView::setDetailFileUrl(const QItemSelection &selected, const QItemSelection &deselected)
{
    static QUrl current;
    if (selected.indexes().isEmpty() && (!deselected.indexes().isEmpty())) {
        QList<QUrl> urls = selectedUrlList();
        if (!urls.isEmpty())
            WorkspaceEventCaller::sendSetSelectDetailFileUrl(this->topLevelWidget()->winId(), urls.back());
        else {
            QUrl url = rootUrl();
            if (current != url) {
                current = url;
                WorkspaceEventCaller::sendSetSelectDetailFileUrl(this->topLevelWidget()->winId(), url);
            }
        }
    } else if (!selected.indexes().isEmpty()) {
        QModelIndex index = selected.first().topLeft();
        QUrl url = model()->getUrlByIndex(index);
        if (current != url) {
            current = url;
            WorkspaceEventCaller::sendSetSelectDetailFileUrl(this->topLevelWidget()->winId(), url);
        }
    }
}

void FileView::resizeEvent(QResizeEvent *event)
{
    DListView::resizeEvent(event);

    updateHorizontalOffset();

    // TODO(liuyangming) crash when launch via command with params.
    if (itemDelegate() && itemDelegate()->editingIndex().isValid())
        doItemsLayout();

    updateModelActiveIndex();
}

void FileView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
    d->selectHelper->selection(rect, flags);
}

void FileView::mousePressEvent(QMouseEvent *event)
{
    switch (event->button()) {
    case Qt::LeftButton: {
        if (dragDropMode() != NoDragDrop) {
            setDragDropMode(DragDrop);
        }

        bool isEmptyArea = d->fileViewHelper->isEmptyArea(event->pos());

        QModelIndex index = indexAt(event->pos());
        d->selectHelper->click(isEmptyArea ? QModelIndex() : index);
        d->selectHelper->setSelection(selectionModel()->selection());

        DListView::mousePressEvent(event);
        break;
    }
    case Qt::RightButton: {
        DListView::mousePressEvent(event);
        break;
    }
    default:
        break;
    }
}

void FileView::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);
}

void FileView::mouseReleaseEvent(QMouseEvent *event)
{
    d->currentDragHoverIndex = QModelIndex();
    d->selectHelper->release();
    if (!QScroller::hasScroller(this)) return DListView::mouseReleaseEvent(event);
}

void FileView::dragEnterEvent(QDragEnterEvent *event)
{
    if (d->dragDropHelper->dragEnter(event))
        return;

    DListView::dragEnterEvent(event);
}

void FileView::dragMoveEvent(QDragMoveEvent *event)
{
    if (isIconViewMode()) {
        d->currentDragHoverIndex = d->fileViewHelper->isEmptyArea(event->pos()) ? rootIndex() : indexAt(event->pos());
    } else {
        d->currentDragHoverIndex = indexAt(event->pos());
        if (!d->currentDragHoverIndex.isValid()) {
            d->currentDragHoverIndex = rootIndex();
        }
    }

    if (d->dragDropHelper->dragMove(event)) {
        if (!event->isAccepted())
            d->currentDragHoverIndex = QModelIndex();

        viewport()->update();
        return;
    }

    DListView::dragMoveEvent(event);
}

void FileView::dragLeaveEvent(QDragLeaveEvent *event)
{
    if (d->dragDropHelper->dragLeave(event))
        return;

    DListView::dragLeaveEvent(event);
}

void FileView::dropEvent(QDropEvent *event)
{
    d->currentDragHoverIndex = QModelIndex();
    if (d->dragDropHelper->drop(event))
        return;

    DListView::dropEvent(event);
}

QModelIndex FileView::indexAt(const QPoint &pos) const
{
    QPoint actualPos = QPoint(pos.x() + horizontalOffset(), pos.y() + verticalOffset());
    QSize itemSize = itemSizeHint();

    int index = -1;
    if (isListViewMode()) {
        index = FileViewHelper::caculateListItemIndex(itemSize, actualPos);
    } else if (isIconViewMode()) {
        index = FileViewHelper::caculateIconItemIndex(this, itemSize, actualPos);
    }

    return model()->index(index, 0);
}

QRect FileView::visualRect(const QModelIndex &index) const
{
    QRect rect;
    if (index.column() != 0)
        return rect;

    QSize itemSize = itemSizeHint();

    if (isListViewMode()) {
        rect.setLeft(kListViewSpacing - horizontalScrollBar()->value());
        rect.setRight(viewport()->width() - kListViewSpacing - 1);
        rect.setTop(index.row() * (itemSize.height() + kListViewSpacing * 2) + kListViewSpacing);
        rect.setHeight(itemSize.height());

        if (d->allowedAdjustColumnSize) {
            rect.setWidth(d->headerView->length());
        }
    } else {
        int itemWidth = itemSize.width() + kIconViewSpacing * 2;
        int columnCount = d->iconModeColumnCount(itemWidth);

        if (columnCount == 0)
            return rect;

        int columnIndex = index.row() % columnCount;
        int rowIndex = index.row() / columnCount;

        rect.setTop(rowIndex * (itemSize.height() + kIconViewSpacing * 2) + kIconViewSpacing);
        rect.setLeft(columnIndex * itemWidth + kIconViewSpacing);
        rect.setSize(itemSize);
    }

    rect.moveLeft(rect.left() - horizontalOffset());
    rect.moveTop(rect.top() - verticalOffset());

    return rect;
}

void FileView::setIconSize(const QSize &size)
{
    DListView::setIconSize(size);

    updateHorizontalOffset();
    updateGeometries();
}

int FileView::horizontalOffset() const
{
    return d->horizontalOffset;
}

FileViewModel *FileView::sourceModel() const
{
    if (model())
        return qobject_cast<FileViewModel *>(model()->sourceModel());

    return nullptr;
}

QList<ItemRoles> FileView::getColumnRoles() const
{
    return d->columnRoles;
}

void FileView::updateGeometries()
{
    if (!d->headerView || !d->allowedAdjustColumnSize) {
        return DListView::updateGeometries();
    }

    resizeContents(d->headerView->length(), contentsSize().height());

    DListView::updateGeometries();
}

void FileView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = d->selectedDraggableIndexes();
    if (!indexes.isEmpty()) {
        if (indexes.count() == 1) {
            DListView::startDrag(supportedActions);
            return;
        }

        QMimeData *data = model()->mimeData(indexes);
        if (!data)
            return;

        QPixmap pixmap = d->viewDrawHelper->renderDragPixmap(indexes);
        QDrag *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->setHotSpot(QPoint(static_cast<int>(pixmap.size().width() / (2 * pixmap.devicePixelRatio())),
                                static_cast<int>(pixmap.size().height() / (2 * pixmap.devicePixelRatio()))));

        Qt::DropAction dropAction = Qt::IgnoreAction;
        Qt::DropAction defaultDropAction = QAbstractItemView::defaultDropAction();
        if (defaultDropAction != Qt::IgnoreAction && (supportedActions & defaultDropAction))
            dropAction = defaultDropAction;
        else if (supportedActions & Qt::CopyAction && dragDropMode() != QAbstractItemView::InternalMove)
            dropAction = Qt::CopyAction;

        drag->exec(supportedActions, dropAction);
    }
}

QModelIndexList FileView::selectedIndexes() const
{
    FileSelectionModel *fileSelectionModel = dynamic_cast<FileSelectionModel *>(selectionModel());
    if (fileSelectionModel) {
        QModelIndexList indexes = fileSelectionModel->selectedIndexes();

        auto isInvalid = [=](const QModelIndex &index) {
            return !(index.isValid() && model()->itemFromIndex(index));
        };

        indexes.erase(std::remove_if(indexes.begin(), indexes.end(), isInvalid),
                      indexes.end());

        return indexes;
    }

    return QModelIndexList();
}

void FileView::showEvent(QShowEvent *event)
{
    DListView::showEvent(event);
    setFocus();
}

void FileView::keyboardSearch(const QString &search)
{
    d->fileViewHelper->keyboardSearch(search);
}

void FileView::contextMenuEvent(QContextMenuEvent *event)
{
    const QModelIndex &index = indexAt(event->pos());

    if (d->fileViewHelper->isEmptyArea(event->pos())) {
        itemDelegate()->hideNotEditingIndexWidget();
        clearSelection();

        d->viewMenuHelper->showEmptyAreaMenu();
    } else {
        if (!isSelected(index)) {
            itemDelegate()->hideNotEditingIndexWidget();
            clearSelection();

            if (!index.isValid()) {
                d->viewMenuHelper->showEmptyAreaMenu();
                return;
            }

            d->selectHelper->click(index);
        }

        d->viewMenuHelper->showNormalMenu(index, model()->flags(index));
    }
}

QModelIndex FileView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex current = currentIndex();

    if (!current.isValid()) {
        d->lastCursorIndex = DListView::moveCursor(cursorAction, modifiers);

        return d->lastCursorIndex;
    }

    if (rectForIndex(current).isEmpty()) {
        d->lastCursorIndex = model()->index(0, 0, rootIndex());

        return d->lastCursorIndex;
    }

    QModelIndex index;

    switch (cursorAction) {
    case MoveLeft:
        if (WindowUtils::keyShiftIsPressed()) {
            index = DListView::moveCursor(cursorAction, modifiers);

            if (index == d->lastCursorIndex) {
                index = index.sibling(index.row() - 1, index.column());
            }
        } else {
            index = current.sibling(current.row() - 1, current.column());
        }

        // rekols: Loop to find the next file item that can be selected.
        while (index.model() && !(index.flags() & Qt::ItemIsSelectable) && index.isValid()) {
            index = index.sibling(index.row() - 1, index.column());
        }

        break;

    case MoveRight:
        if (WindowUtils::keyShiftIsPressed()) {
            index = DListView::moveCursor(cursorAction, modifiers);

            if (index == d->lastCursorIndex) {
                index = index.sibling(index.row() + 1, index.column());
            }
        } else {
            index = current.sibling(current.row() + 1, current.column());
        }

        while (index.model() && !(index.flags() & Qt::ItemIsSelectable) && index.isValid()) {
            index = index.sibling(index.row() + 1, index.column());
        }

        break;

    default:
        index = DListView::moveCursor(cursorAction, modifiers);
        break;
    }

    if (index.isValid()) {
        if (viewMode() == IconMode) {
            bool lastRow = indexOfRow(index) == rowCount() - 1;

            if (!lastRow
                && current == index
                && (cursorAction == MoveDown
                    || cursorAction == MovePageDown
                    || cursorAction == MoveNext)) {
                // 当下一个位置没有元素时，QListView不会自动换一列选择，应该直接选中最后一个
                index = model()->index(count() - 1, 0);
                lastRow = true;
            }

            if (lastRow) {
                // call later
                //QTimer::singleShot(0, this, [this, index, d] {//this index unused,改成如下
                QTimer::singleShot(0, this, [this] {
                    // scroll to end
                    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
                });
            }
        }

        d->lastCursorIndex = index;

        return index;
    }

    d->lastCursorIndex = current;

    return current;
}

bool FileView::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::KeyPress: {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        if (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab) {
            if (keyEvent->modifiers() == Qt::ControlModifier || keyEvent->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
                return DListView::event(e);
            e->accept();

            if (keyEvent->modifiers() == Qt::ShiftModifier) {
                QKeyEvent nkeyEvent(keyEvent->type(), Qt::Key_Left, Qt::NoModifier);
                keyPressEvent(&nkeyEvent);
            } else {
                QKeyEvent nkeyEvent(keyEvent->type(), Qt::Key_Right, Qt::NoModifier);
                keyPressEvent(&nkeyEvent);
            }

            return true;
        }
    } break;
    case QEvent::Resize:
        d->pureResizeEvent(static_cast<QResizeEvent *>(e));
        break;
    case QEvent::ParentChange:
        window()->installEventFilter(this);
        break;
    case QEvent::FontChange:
        // blumia: to trigger DIconItemDelegate::updateItemSizeHint() to update its `d->itemSizeHint` ...
        emit iconSizeChanged(iconSize());
        break;
    default:
        break;
    }

    return DListView::event(e);
}

bool FileView::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Move:
        if (obj != horizontalScrollBar()->parentWidget())
            return DListView::eventFilter(obj, event);
        d->updateHorizontalScrollBarPosition();
        break;
    case QEvent::WindowStateChange:
        if (d->headerView) {
            d->adjustFileNameColumn = true;
            d->headerView->doFileNameColumnResize(width());
        }
        break;
    // blumia: 这里通过给横向滚动条加事件过滤器并监听其显示隐藏时间来判断是否应当进入吸附状态。
    //         不过其实可以通过 Resize 事件的 size 和 oldSize 判断是否由于窗口调整大小而进入了吸附状态。
    //         鉴于已经实现完了，如果当前的实现方式实际发现了较多问题，则应当调整为使用 Resize 事件来标记吸附状态的策略。
    case QEvent::ShowToParent:
    case QEvent::HideToParent:
        if (d->headerView && d->cachedViewWidth != this->width()) {
            d->cachedViewWidth = this->width();
            d->adjustFileNameColumn = true;
        }
        break;
    default:
        break;
    }

    return DListView::eventFilter(obj, event);
}

void FileView::initializeModel()
{
    FileViewModel *model = new FileViewModel(this);
    FileSortFilterProxyModel *proxyModel = new FileSortFilterProxyModel(this);

    proxyModel->setSourceModel(model);
    setModel(proxyModel);

    FileSelectionModel *selectionModel = new FileSelectionModel(proxyModel, this);
    setSelectionModel(selectionModel);

    d->sortTimer = new QTimer(this);
    d->sortTimer->setInterval(20);
    d->sortTimer->setSingleShot(true);
}

void FileView::initializeDelegate()
{
    d->fileViewHelper = new FileViewHelper(this);
    setDelegate(Global::ViewMode::kIconMode, new IconItemDelegate(d->fileViewHelper));
    setDelegate(Global::ViewMode::kListMode, new ListItemDelegate(d->fileViewHelper));
}

void FileView::initializeStatusBar()
{
    d->statusBar = new FileViewStatusBar(this);
    d->statusBar->resetScalingSlider(iconSizeList().length() - 1);

    d->updateStatusBarTimer = new QTimer(this);
    d->updateStatusBarTimer->setInterval(100);
    d->updateStatusBarTimer->setSingleShot(true);

    addFooterWidget(d->statusBar);
}

void FileView::initializeConnect()
{
    connect(d->sortTimer, &QTimer::timeout, this, &FileView::delaySort);
    connect(d->updateStatusBarTimer, &QTimer::timeout, this, &FileView::updateStatusBar);

    connect(d->statusBar->scalingSlider(), &QSlider::valueChanged, this, &FileView::onScalingValueChanged);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &FileView::updateModelActiveIndex);

    connect(sourceModel(), &FileViewModel::updateFiles, this, &FileView::updateView);
    connect(sourceModel(), &FileViewModel::stateChanged, this, &FileView::onModelStateChanged);
    connect(sourceModel(), &FileViewModel::modelChildrenUpdated, this, &FileView::onChildrenChanged);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileView::onSelectionChanged);

    connect(this, &DListView::rowCountChanged, this, &FileView::onRowCountChanged, Qt::QueuedConnection);
    connect(this, &DListView::clicked, this, &FileView::onClicked);
    connect(this, &DListView::doubleClicked, this, &FileView::onDoubleClicked);
    connect(this, &DListView::iconSizeChanged, this, &FileView::updateHorizontalOffset, Qt::QueuedConnection);

    connect(this, &FileView::clicked, this, &FileView::onClicked, Qt::UniqueConnection);
    connect(this, &FileView::viewStateChanged, this, &FileView::saveViewModeState);

    connect(WorkspaceHelper::instance(), &WorkspaceHelper::viewModeChanged, this, &FileView::viewModeChanged);
    connect(WorkspaceHelper::instance(), &WorkspaceHelper::requestSetViewFilterData, this, &FileView::setFilterData);
    connect(WorkspaceHelper::instance(), &WorkspaceHelper::requestSetViewFilterCallback, this, &FileView::setFilterCallback);

    connect(Application::instance(), &Application::iconSizeLevelChanged, this, &FileView::setIconSizeBySizeIndex);
    connect(Application::instance(), &Application::showedHiddenFilesChanged, this, &FileView::onShowHiddenFileChanged);
    connect(Application::instance(), &Application::showedFileSuffixChanged, this, &FileView::onShowFileSuffixChanged);
}

void FileView::updateStatusBar()
{
    if (sourceModel()->state() != FileViewModel::Idle)
        return;

    int count = selectedIndexCount();
    if (count == 0) {
        d->statusBar->itemCounted(model()->rowCount());
        return;
    }

    QList<AbstractFileInfo *> list;
    for (const QModelIndex &index : selectedIndexes())
        list << model()->itemFromIndex(index)->fileInfo().data();

    d->statusBar->itemSelected(list);
}

void FileView::updateLoadingIndicator()
{
    auto state = sourceModel()->state();
    if (state == FileViewModel::Busy) {
        QString tip;

        AbstractFileInfoPointer fileInfo = sourceModel()->rootItem()->fileInfo();
        if (fileInfo)
            tip = fileInfo->loadingTip();

        d->statusBar->showLoadingIncator(tip);
    }

    if (state == FileViewModel::Idle) {
        d->statusBar->hideLoadingIncator();
        updateStatusBar();
    }
}

void FileView::updateContentLabel()
{
    d->initContentLabel();
    if (sourceModel()->state() == FileViewModel::Busy
        /*|| model()->canFetchMore(model()->rootIndex())*/) {
        d->contentLabel->setText(QString());
        return;
    }

    if (count() <= 0) {
        // set custom empty tips
        AbstractFileInfoPointer fileInfo = sourceModel()->rootItem()->fileInfo();
        if (fileInfo) {
            d->contentLabel->setText(fileInfo->emptyDirectoryTip());
            d->contentLabel->adjustSize();
            return;
        }
    }

    d->contentLabel->setText(QString());
}

void FileView::updateListHeaderView()
{
    if (!d->headerView)
        return;

    d->headerView->setModel(Q_NULLPTR);
    d->headerView->setModel(model());

    d->headerView->setDefaultSectionSize(kDefualtHeaderSectionWidth);
    if (d->allowedAdjustColumnSize) {
        d->headerView->setSectionResizeMode(QHeaderView::Interactive);
        d->headerView->setMinimumSectionSize(kMinimumHeaderSectionWidth);
    } else {
        d->headerView->setSectionResizeMode(QHeaderView::Fixed);
        d->headerView->setMinimumSectionSize(kDefualtHeaderSectionWidth);
    }

    d->headerView->setSortIndicator(model()->getColumnByRole(d->currentSortRole), d->currentSortOrder);
    d->columnRoles.clear();

    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "ViewColumnState").toMap();

    for (int i = 0; i < d->headerView->count(); ++i) {
        int logicalIndex = d->headerView->logicalIndex(i);
        d->columnRoles << model()->getRoleByColumn(i);

        if (d->allowedAdjustColumnSize) {
            int colWidth = state.value(QString::number(d->columnRoles.last()), -1).toInt();
            if (colWidth > 0) {
                d->headerView->resizeSection(model()->getColumnByRole(d->columnRoles.last()), colWidth);
            }
        } else {
            int columnWidth = model()->getColumnWidth(i);
            if (columnWidth >= 0) {
                d->headerView->resizeSection(logicalIndex, columnWidth + kColumnPadding * 2);
            } else {
                d->headerView->setSectionResizeMode(logicalIndex, QHeaderView::Stretch);
            }
        }

        const QString &columnName = model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();

        if (!d->columnForRoleHiddenMap.contains(columnName)) {
            d->headerView->setSectionHidden(logicalIndex, false);
        } else {
            d->headerView->setSectionHidden(logicalIndex, d->columnForRoleHiddenMap.value(columnName));
        }
    }

    if (d->adjustFileNameColumn) {
        d->headerView->doFileNameColumnResize(width());
    }
}

void FileView::setDefaultViewMode()
{
    setViewMode(d->currentViewMode);
}

void FileView::loadViewState(const QUrl &url)
{
    int defaultViewMode = static_cast<int>(WorkspaceHelper::instance()->findViewMode(url.scheme()));
    d->currentViewMode = static_cast<Global::ViewMode>(fileViewStateValue(url, "viewMode", defaultViewMode).toInt());

    QVariant defaultIconSize = Application::instance()->appAttribute(Application::kIconSizeLevel).toInt();
    d->currentIconSizeLevel = fileViewStateValue(url, "iconSizeLevel", defaultIconSize).toInt();

    d->currentSortRole = static_cast<ItemRoles>(fileViewStateValue(url, "sortRole", kItemNameRole).toInt());
    d->currentSortOrder = static_cast<Qt::SortOrder>(fileViewStateValue(url, "sortOrder", Qt::SortOrder::AscendingOrder).toInt());
}

void FileView::delaySort()
{
    model()->sort(model()->getColumnByRole(d->currentSortRole), d->currentSortOrder);
}

void FileView::onModelStateChanged()
{
    updateContentLabel();
    updateLoadingIndicator();

    if (d->headerView) {
        d->headerView->setAttribute(Qt::WA_TransparentForMouseEvents, sourceModel()->state() == FileViewModel::Busy);
    }
}

void FileView::openIndexByClicked(const ClickedAction action, const QModelIndex &index)
{
    ClickedAction configAction = static_cast<ClickedAction>(Application::instance()->appAttribute(Application::kOpenFileMode).toInt());
    if (action == configAction) {
        Qt::ItemFlags flags = model()->flags(index);
        if (!flags.testFlag(Qt::ItemIsEnabled))
            return;

        if (!WindowUtils::keyCtrlIsPressed() && !WindowUtils::keyShiftIsPressed())
            openIndex(index);
    }
}

void FileView::openIndex(const QModelIndex &index)
{
    const FileViewItem *item = model()->itemFromIndex(index);

    if (!item)
        return;

    FileOperatorHelperIns->openFiles(this, { item->url() });
}

QVariant FileView::fileViewStateValue(const QUrl &url, const QString &key, const QVariant &defalutValue)
{
    return Application::appObtuselySetting()->value("FileViewState", url).toMap().value(key, defalutValue);
}

void FileView::setFileViewStateValue(const QUrl &url, const QString &key, const QVariant &value)
{
    QVariantMap map = Application::appObtuselySetting()->value("FileViewState", url).toMap();

    map[key] = value;

    Application::appObtuselySetting()->setValue("FileViewState", url, map);
}

void FileView::saveViewModeState()
{
    const QUrl &url = rootUrl();

    setFileViewStateValue(url, "iconSizeLevel", d->statusBar->scalingSlider()->value());
    setFileViewStateValue(url, "viewMode", static_cast<int>(d->currentViewMode));
}
