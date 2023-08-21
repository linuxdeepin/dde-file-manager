// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "headerview.h"
#include "fileview.h"
#include "private/fileview_p.h"
#include "models/fileselectionmodel.h"
#include "models/fileviewmodel.h"
#include "models/rootinfo.h"
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
#include "utils/filedatamanager.h"
#include "events/workspaceeventsequence.h"

#include <dfm-base/mimedata/dfmmimedata.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#    include <DGuiApplicationHelper>
#endif

#include <QResizeEvent>
#include <QScrollBar>
#include <QScroller>
#include <QTimer>
#include <QDrag>
#include <QApplication>
#include <QUrlQuery>
#include <QMimeData>

using namespace dfmplugin_workspace;
DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

FileView::FileView(const QUrl &url, QWidget *parent)
    : DListView(parent), d(new FileViewPrivate(this))
{
    Q_UNUSED(url);
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
    setDragEnabled(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    initializeModel();
    initializeDelegate();
    initializeStatusBar();
    initializeConnect();
    initializeScrollBarWatcher();

    viewport()->installEventFilter(this);
}

FileView::~FileView()
{
    disconnect(model(), &FileViewModel::stateChanged, this, &FileView::onModelStateChanged);
    disconnect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileView::onSelectionChanged);

    dpfSignalDispatcher->unsubscribe("dfmplugin_workspace", "signal_View_HeaderViewSectionChanged", this, &FileView::onHeaderViewSectionChanged);
    dpfSignalDispatcher->unsubscribe("dfmplugin_filepreview", "signal_ThumbnailDisplay_Changed", this, &FileView::onWidgetUpdate);
}

QWidget *FileView::widget() const
{
    return const_cast<FileView *>(this);
}

void FileView::setViewMode(Global::ViewMode mode)
{
    if (itemDelegate())
        itemDelegate()->hideAllIIndexWidget();

    int modeValue = static_cast<int>(mode);
    if (d->delegates.keys().contains(modeValue)) {
        d->currentViewMode = mode;
    } else {
        qWarning() << QString("The view mode %1 is not support in this dir! This view will set default mode.").arg(modeValue);
        d->currentViewMode = Global::ViewMode::kIconMode;
    }

    setItemDelegate(d->delegates[static_cast<int>(d->currentViewMode)]);

    switch (d->currentViewMode) {
    case Global::ViewMode::kIconMode:
        setUniformItemSizes(false);
        setResizeMode(Adjust);
        setOrientation(QListView::LeftToRight, true);
#ifdef DTKWIDGET_CLASS_DSizeMode
        setSpacing(DSizeModeHelper::element(kCompactIconViewSpacing, kIconViewSpacing));
#else
        setSpacing(kIconViewSpacing);
#endif
        d->initIconModeView();
        setMinimumWidth(0);
        break;
    case Global::ViewMode::kListMode:
        setUniformItemSizes(true);
        setResizeMode(Fixed);
        setOrientation(QListView::TopToBottom, false);
        setSpacing(kListViewSpacing);

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
    selectionModel()->clear();
    d->statusBar->itemCounted(0);

    // Todo(yanghao&lzj):!url.isSearchFile()
    setFocus();

    const QUrl &fileUrl = parseSelectedUrl(url);
    const QModelIndex &index = model()->setRootUrl(fileUrl);

    setRootIndex(index);

    loadViewState(fileUrl);
    delayUpdateStatusBar();
    updateContentLabel();
    setDefaultViewMode();

    resetSelectionModes();
    updateListHeaderView();

    // dir already traversal
    if (model()->currentState() == ModelState::kIdle)
        updateSelectedUrl();

    return true;
}

QUrl FileView::rootUrl() const
{
    return model()->rootUrl();
}

AbstractBaseView::ViewState FileView::viewState() const
{
    switch (model()->currentState()) {
    case ModelState::kIdle:
        return AbstractBaseView::ViewState::kViewIdle;
    case ModelState::kBusy:
        return AbstractBaseView::ViewState::kViewBusy;
    }

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
        list << model()->data(index, ItemRoles::kItemUrlRole).toUrl();
    }

    return list;
}

void FileView::refresh()
{
    model()->refresh();
}

void FileView::doItemsLayout()
{
    if (rootIndex().isValid())
        DListView::doItemsLayout();
}

FileViewModel *FileView::model() const
{
    return qobject_cast<FileViewModel *>(QAbstractItemView::model());
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
    model()->stopTraversWork();
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
    if (model()->currentState() != ModelState::kIdle)
        return;

    int rowCount = model()->rowCount(rootIndex());

    if (rowCount < 1)
        return;

    QStyleOptionViewItem option = viewOptions();

    option.rect.setWidth(QWIDGETSIZE_MAX);
    option.rect.setHeight(itemSizeHint().height());

    int columnMaxWidth = 0;

    for (int i = 0; i < rowCount; ++i) {
        const QModelIndex &index = model()->index(i, 0, rootIndex());
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

    // each views should refresh
    dpfSignalDispatcher->publish("dfmplugin_workspace", "signal_View_HeaderViewSectionChanged", rootUrl);
}

void FileView::onHeaderHiddenChanged(const QString &roleName, const bool isHidden)
{
    d->columnForRoleHiddenMap[roleName] = isHidden;

    if (d->allowedAdjustColumnSize) {
        updateListHeaderView();
    } else {
        d->headerView->updateColumnWidth();
    }
}

void FileView::onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    model()->sort(logicalIndex, order);

    const QUrl &url = rootUrl();
    const ItemRoles &role = model()->getRoleByColumn(logicalIndex);
    setFileViewStateValue(url, "sortRole", role);
    setFileViewStateValue(url, "sortOrder", static_cast<int>(order));
}

void FileView::onClicked(const QModelIndex &index)
{
    openIndexByClicked(ClickedAction::kClicked, index);

    QUrl url { "" };
    const FileInfoPointer &info = model()->fileInfo(index);
    if (info)
        url = info->urlOf(UrlInfoType::kUrl);
    QVariantMap data;
    data.insert("displayName", model()->data(index));
    data.insert("url", url);
    WorkspaceEventCaller::sendViewItemClicked(data);
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
    } else if (event->modifiers() == Qt::AltModifier || event->orientation() == Qt::Horizontal) {
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
    Global::ViewMode mode = static_cast<Global::ViewMode>(viewMode);
    if (mode == Global::ViewMode::kIconMode) {
        setViewModeToIcon();
    } else if (mode == Global::ViewMode::kListMode) {
        setViewModeToList();
    }

    setFocus();
    saveViewModeState();
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
        int firstIndex = (rect.top() + spacing) / itemHeight;
        int lastIndex = (rect.bottom() - spacing) / itemHeight;

        if (firstIndex >= count)
            return list;

        list << RandeIndex(qMax(firstIndex, 0), qMin(lastIndex, count - 1));
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

    onSortIndicatorChanged(column, order);
    if (d->headerView) {
        QSignalBlocker blocker(d->headerView);
        Q_UNUSED(blocker)
        d->headerView->setSortIndicator(column, order);
    }
}

void FileView::setViewSelectState(bool isSelect)
{
    d->isShowViewSelectBox = isSelect;
    viewport()->update();
}

QModelIndex FileView::currentPressIndex() const
{
    return d->selectHelper->getCurrentPressedIndex();
}

bool FileView::isDragTarget(const QModelIndex &index) const
{
    return d->dragDropHelper->isDragTarget(index);
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

bool FileView::isVerticalScrollBarSliderDragging() const
{
    if (d->scrollBarValueChangedTimer)
        return d->scrollBarValueChangedTimer->isActive();

    return false;
}

void FileView::onSelectAndEdit(const QUrl &url)
{
    if (!url.isValid())
        return;

    quint64 winId = WorkspaceHelper::instance()->windowId(this);
    if (!WorkspaceHelper::kSelectionAndRenameFile.contains(winId))
        return;

    QPair<QUrl, QUrl> urlPair = WorkspaceHelper::kSelectionAndRenameFile[winId];
    if (!UniversalUtils::urlEquals(urlPair.first, rootUrl()) || !UniversalUtils::urlEquals(urlPair.second, url))
        return;

    WorkspaceHelper::kSelectionAndRenameFile.take(winId);

    const QModelIndex &index = model()->getIndexByUrl(url);
    if (!index.isValid())
        return;

    selectionModel()->clear();
    selectFiles({ url });
    edit(index, QAbstractItemView::AllEditTriggers, nullptr);
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

void FileView::updateOneView(const QModelIndex &index)
{
    update(index);
}

void FileView::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    delayUpdateStatusBar();

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
    const QUrl &oldCurrentUrl = rootUrl();
    QUrl parentUrl = UrlRoute::urlParent(oldCurrentUrl);

    if (parentUrl.isValid()) {
        FileOperatorHelperIns->openFilesByMode(this, { parentUrl }, DirOpenMode::kOpenInCurrentWindow);
        return true;
    } else {
        auto windowId = WorkspaceHelper::instance()->windowId(this);
        QUrl computerRoot;
        computerRoot.setScheme(Global::Scheme::kComputer);
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

void FileView::onWidgetUpdate()
{
    this->update();
}

void FileView::onRowCountChanged()
{
    // clean selected indexes
    // the selectList will be reseted while call the selectedIndexes() at next time
    static_cast<FileSelectionModel *>(selectionModel())->clearSelectList();

    delayUpdateStatusBar();
    updateContentLabel();
}

void FileView::setFilterData(const QUrl &url, const QVariant &data)
{
    if (url == rootUrl() && isVisible()) {
        clearSelection();
        model()->setFilterData(data);
    }
}

void FileView::setFilterCallback(const QUrl &url, const FileViewFilterCallback callback)
{
    if (url == rootUrl() && isVisible()) {
        clearSelection();
        model()->setFilterCallback(callback);
    }
}

void FileView::trashStateChanged()
{
    if (Q_LIKELY(model()))
        model()->updateFile(FileUtils::trashRootUrl());
}

void FileView::onHeaderViewSectionChanged(const QUrl &url)
{
    if (UniversalUtils::urlEquals(url, rootUrl()) && viewMode() == ViewMode::ListMode) {
        // refresh
        updateListHeaderView();
        update();
    }
}

bool FileView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    return DListView::edit(index, trigger, event);
}

void FileView::resizeEvent(QResizeEvent *event)
{
    DListView::resizeEvent(event);

    updateHorizontalOffset();

    // TODO(liuyangming) crash when launch via command with params.
    if (itemDelegate() && itemDelegate()->editingIndex().isValid())
        doItemsLayout();
}

void FileView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
    d->selectHelper->selection(rect, flags);
}

void FileView::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons().testFlag(Qt::LeftButton)) {
        d->mouseLeftPressed = true;
        d->mouseLastPos = event->globalPos();
    } else {
        d->mouseLeftPressed = false;
    }

    switch (event->button()) {
    case Qt::LeftButton: {
        if (dragDropMode() != NoDragDrop) {
            setDragDropMode(DragDrop);
        }

        bool isEmptyArea = d->fileViewHelper->isEmptyArea(event->pos());
        if (isEmptyArea && (qApp->keyboardModifiers() == Qt::NoModifier))
            setCurrentIndex(QModelIndex());

        QModelIndex index = indexAt(event->pos());
        d->selectHelper->click(isEmptyArea ? QModelIndex() : index);

        if (itemDelegate())
            itemDelegate()->commitDataAndCloseActiveEditor();

        if (isEmptyArea) {
            if (selectionMode() != QAbstractItemView::SingleSelection)
                d->selectHelper->setSelection(selectionModel()->selection());
            if (!WindowUtils::keyCtrlIsPressed()) {
                if (itemDelegate())
                    itemDelegate()->hideNotEditingIndexWidget();
                if (qApp->keyboardModifiers() == Qt::NoModifier)
                    setCurrentIndex(QModelIndex());
                if (dragDropMode() != NoDragDrop) {
                    setDragDropMode(DropOnly);
                }
            }
        } else if (WindowUtils::keyCtrlIsPressed() && selectionModel()->isSelected(index)) {
            d->selectHelper->setSelection(selectionModel()->selection());
            d->lastMousePressedIndex = index;

            DListView::mousePressEvent(event);

            selectionModel()->select(index, QItemSelectionModel::Select);

            return;
        } else if (WindowUtils::keyShiftIsPressed()) {
            if (!selectionModel()->isSelected(index)) {   // 如果该项没有被选择
                DListView::mousePressEvent(event);   // 选择该项
                return;
            }
        } else {
            if (selectionMode() != QAbstractItemView::SingleSelection)
                d->selectHelper->setSelection(selectionModel()->selection());
        }

        d->lastMousePressedIndex = QModelIndex();

        DListView::mousePressEvent(event);
        break;
    }
    case Qt::RightButton: {
        bool isEmptyArea = d->fileViewHelper->isEmptyArea(event->pos());

        if (!isEmptyArea) {
            const QModelIndex &index = indexAt(event->pos());
            if (selectedIndexes().isEmpty() || !selectedIndexes().contains(index)) {
                setCurrentIndex(index);
            }
        }

        break;
    }
    default:
        break;
    }
}

void FileView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
        d->mouseMoveRect = QRect(event->globalPos(), d->mouseLastPos);

    DListView::mouseMoveEvent(event);
}

void FileView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        d->mouseMoveRect = QRect(-1, -1, 1, 1);
        d->mouseLastPos = QPoint(0, 0);
    }

    d->selectHelper->release();

    if (WindowUtils::keyCtrlIsPressed()
        && d->lastMousePressedIndex.isValid()
        && d->lastMousePressedIndex == indexAt(event->pos())) {
        selectionModel()->select(d->lastMousePressedIndex, QItemSelectionModel::Deselect);
    }

    if (!QScroller::hasScroller(this))
        return DListView::mouseReleaseEvent(event);
}

void FileView::dragEnterEvent(QDragEnterEvent *event)
{
    if (d->dragDropHelper->dragEnter(event))
        return;

    DListView::dragEnterEvent(event);
}

void FileView::dragMoveEvent(QDragMoveEvent *event)
{
    if (d->dragDropHelper->dragMove(event)) {
        viewport()->update();
        return;
    }

    DListView::dragMoveEvent(event);
}

void FileView::dragLeaveEvent(QDragLeaveEvent *event)
{
    setViewSelectState(false);
    if (d->dragDropHelper->dragLeave(event))
        return;

    DListView::dragLeaveEvent(event);
}

void FileView::dropEvent(QDropEvent *event)
{
    setViewSelectState(false);
    if (d->dragDropHelper->drop(event))
        return;
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

    if (index == -1 || index >= model()->rowCount(rootIndex()))
        return QModelIndex();

    return model()->index(index, 0, rootIndex());
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

        if (d->allowedAdjustColumnSize && d->headerView) {
            rect.setWidth(d->headerView->length());
        }
    } else {
        int iconViewSpacing = kIconViewSpacing;
#ifdef DTKWIDGET_CLASS_DSizeMode
        iconViewSpacing = DSizeModeHelper::element(kCompactIconViewSpacing, kIconViewSpacing);
#endif
        int itemWidth = itemSize.width() + iconViewSpacing * 2;
        int columnCount = d->iconModeColumnCount(itemWidth);

        if (columnCount == 0)
            return rect;

        int columnIndex = index.row() % columnCount;
        int rowIndex = index.row() / columnCount;

        rect.setTop(rowIndex * (itemSize.height() + iconViewSpacing * 2) + iconViewSpacing);
        rect.setLeft(columnIndex * itemWidth + iconViewSpacing);
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

int FileView::verticalOffset() const
{
    return DListView::verticalOffset();
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
    if (NetworkUtils::instance()->checkFtpOrSmbBusy(rootUrl())) {
        DialogManager::instance()->showUnableToVistDir(rootUrl().path());
        return;
    }
    QModelIndexList indexes = d->selectedDraggableIndexes();
    if (!indexes.isEmpty()) {
        QMimeData *data = model()->mimeData(indexes);
        if (!data)
            return;
        Qt::DropAction defaultDropAction = QAbstractItemView::defaultDropAction();
        if (WorkspaceEventSequence::instance()->doCheckDragTarget(data->urls(), QUrl(), &defaultDropAction)) {
            qDebug() << "Change supported actions: " << defaultDropAction;
            supportedActions = defaultDropAction;
        }

        QList<QUrl> transformedUrls;
        UniversalUtils::urlsTransformToLocal(data->urls(), &transformedUrls);
        qDebug() << "Drag source urls: " << data->urls();
        qDebug() << "Drag transformed urls: " << transformedUrls;
        DFMMimeData dfmmimeData;
        dfmmimeData.setUrls(data->urls());
        data->setData(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey, dfmmimeData.toByteArray());
        data->setUrls(transformedUrls);

        QPixmap pixmap = d->viewDrawHelper->renderDragPixmap(currentViewMode(), indexes);
        QDrag *drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->setHotSpot(QPoint(static_cast<int>(pixmap.size().width() / (2 * pixmap.devicePixelRatio())),
                                static_cast<int>(pixmap.size().height() / (2 * pixmap.devicePixelRatio()))));

        Qt::DropAction dropAction = Qt::IgnoreAction;
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
            return !(index.isValid());
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
    // if the left btn is pressed and the rect large enough, means the view state is draggingSelectState.
    // and don‘t show menu in the draggingSelectState.
    if (d->mouseLeftPressed && (abs(d->mouseMoveRect.width()) > kMinMoveLenght || abs(d->mouseMoveRect.height()) > kMinMoveLenght))
        return;

    if (NetworkUtils::instance()->checkFtpOrSmbBusy(rootUrl())) {
        DialogManager::instance()->showUnableToVistDir(rootUrl().path());
        return;
    }

    if (FileViewMenuHelper::disableMenu())
        return;

    d->viewMenuHelper->setWaitCursor();
    const QModelIndex &index = indexAt(event->pos());
    if (itemDelegate()->editingIndex().isValid() && itemDelegate()->editingIndex() == index)
        setFocus(Qt::FocusReason::OtherFocusReason);
    if (d->fileViewHelper->isEmptyArea(event->pos())) {
        BaseItemDelegate *de = itemDelegate();
        if (de)
            de->hideNotEditingIndexWidget();
        clearSelection();

        d->viewMenuHelper->showEmptyAreaMenu();
    } else {
        if (!isSelected(index)) {
            itemDelegate()->hideNotEditingIndexWidget();
            clearSelection();

            if (!index.isValid()) {
                d->viewMenuHelper->showEmptyAreaMenu();
                d->viewMenuHelper->reloadCursor();
                return;
            }

            selectionModel()->select(index, QItemSelectionModel::Select);
        }

        d->viewMenuHelper->showNormalMenu(index, model()->flags(index));
    }
}

QModelIndex FileView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    if (!rootIndex().isValid())
        return QModelIndex();

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
                index = model()->index(count() - 1, 0, rootIndex());
                lastRow = true;
            }

            if (lastRow) {
                // call later
                // QTimer::singleShot(0, this, [this, index, d] {//this index unused,改成如下
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
        if (keyEvent && (keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab)) {
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
    case QEvent::MouseButtonRelease: {
        QWidget *viewport = qobject_cast<QWidget *>(obj);
        quint64 winId = dfmbase::FileManagerWindowsManager::instance().findWindowId(viewport);
        auto e = static_cast<QMouseEvent *>(event);
        if (!e)
            break;
        if (e->button() == Qt::BackButton) {
            dpfSlotChannel->push("dfmplugin_titlebar", "slot_Navigator_Backward", winId);
            return true;
        } else if (e->button() == Qt::ForwardButton) {
            dpfSlotChannel->push("dfmplugin_titlebar", "slot_Navigator_Forward", winId);
            return true;
        }
    } break;
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

void FileView::paintEvent(QPaintEvent *event)
{
    DListView::paintEvent(event);

    if (d->isShowViewSelectBox) {
        QPainter painter(viewport());
        QColor color = palette().color(QPalette::Active, QPalette::Highlight);
        color.setAlphaF(0.4);   // 40% transparency
        QPen pen(color, kSelectBoxLineWidth);
        painter.setPen(pen);
        painter.drawRect(QRectF(kSelectBoxLineWidth / 2, kSelectBoxLineWidth / 2, viewport()->size().width() - kSelectBoxLineWidth, viewport()->size().height() - kSelectBoxLineWidth));
    }
}

void FileView::focusInEvent(QFocusEvent *event)
{
    DListView::focusInEvent(event);
    if (itemDelegate())
        itemDelegate()->commitDataAndCloseActiveEditor();

    setAttribute(Qt::WA_InputMethodEnabled, false);
}

void FileView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    DListView::currentChanged(current, previous);
    setAttribute(Qt::WA_InputMethodEnabled, false);
}

void FileView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QModelIndex currentIdx = currentIndex();
    for (const QModelIndex &index : selectedIndexes()) {
        if (index.parent() != parent)
            continue;

        if (index.row() >= start && index.row() <= end) {
            selectionModel()->select(index, QItemSelectionModel::Clear);
            if (index == currentIdx) {
                clearSelection();
                setCurrentIndex(QModelIndex());
            }
        }
    }

    DListView::rowsAboutToBeRemoved(parent, start, end);
}

void FileView::initializeModel()
{
    FileViewModel *viewModel = new FileViewModel(this);
    setModel(viewModel);

    FileSelectionModel *selectionModel = new FileSelectionModel(viewModel, this);
    setSelectionModel(selectionModel);
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
    connect(d->updateStatusBarTimer, &QTimer::timeout, this, &FileView::updateStatusBar);

    connect(d->statusBar->scalingSlider(), &DSlider::valueChanged, this, &FileView::onScalingValueChanged);

    connect(model(), &FileViewModel::stateChanged, this, &FileView::onModelStateChanged);
    connect(model(), &FileViewModel::selectAndEditFile, this, &FileView::onSelectAndEdit);
    connect(model(), &FileViewModel::dataChanged, this, &FileView::updateOneView);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileView::onSelectionChanged);

    connect(this, &DListView::rowCountChanged, this, &FileView::onRowCountChanged, Qt::QueuedConnection);
    connect(this, &DListView::clicked, this, &FileView::onClicked, Qt::UniqueConnection);
    connect(this, &DListView::doubleClicked, this, &FileView::onDoubleClicked);
    connect(this, &DListView::iconSizeChanged, this, &FileView::updateHorizontalOffset, Qt::QueuedConnection);
    connect(this, &FileView::viewStateChanged, this, &FileView::saveViewModeState);

    connect(Application::instance(), &Application::iconSizeLevelChanged, this, &FileView::setIconSizeBySizeIndex);
    connect(Application::instance(), &Application::showedFileSuffixChanged, this, &FileView::onShowFileSuffixChanged);
    connect(Application::instance(), &Application::previewAttributeChanged, this, &FileView::onWidgetUpdate);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        if (d->currentViewMode == Global::ViewMode::kIconMode)
            this->setSpacing(DSizeModeHelper::element(kCompactIconViewSpacing, kIconViewSpacing));
    });
#endif

    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_View_HeaderViewSectionChanged", this, &FileView::onHeaderViewSectionChanged);
    dpfSignalDispatcher->subscribe("dfmplugin_filepreview", "signal_ThumbnailDisplay_Changed", this, &FileView::onWidgetUpdate);
}

void FileView::initializeScrollBarWatcher()
{
    d->scrollBarValueChangedTimer = new QTimer(this);
    d->scrollBarValueChangedTimer->setInterval(50);
    d->scrollBarValueChangedTimer->setSingleShot(true);

    connect(d->scrollBarValueChangedTimer, &QTimer::timeout, this, [this] { this->update(); });

    connect(verticalScrollBar(), &QScrollBar::sliderPressed, this, [this] { d->scrollBarSliderPressed = true; });
    connect(verticalScrollBar(), &QScrollBar::sliderReleased, this, [this] { d->scrollBarSliderPressed = false; });
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this] {
       if (d->scrollBarSliderPressed)
           d->scrollBarValueChangedTimer->start();
    });
}

void FileView::updateStatusBar()
{
    if (model()->currentState() != ModelState::kIdle)
        return;

    int count = selectedIndexCount();

    if (count == 0) {
        d->statusBar->itemCounted(model()->rowCount(rootIndex()));
        return;
    }

    QList<QUrl> list;
    int selectFiles = 0;
    int selectFolders = 0;
    qint64 filesizes = 0;
    for (const auto &index : selectedIndexes()) {
        if (index.data(Global::ItemRoles::kItemFileIsDirRole).toBool()) {
            selectFolders++;
            list << index.data(Global::ItemRoles::kItemUrlRole).value<QUrl>();
        } else {
            selectFiles++;
            filesizes += index.data(Global::ItemRoles::kItemFileSizeIntRole).toLongLong();
        }
    }

    d->statusBar->itemSelected(selectFiles, selectFolders, filesizes, list);
}

void FileView::updateLoadingIndicator()
{
    auto state = model()->currentState();
    if (state == ModelState::kBusy) {
        QString tip;

        const FileInfoPointer &fileInfo = model()->fileInfo(rootIndex());
        if (fileInfo)
            tip = fileInfo->viewOfTip(ViewInfoType::kLoading);

        d->statusBar->showLoadingIncator(tip);
    }

    if (state == ModelState::kIdle) {
        d->statusBar->hideLoadingIncator();
        updateStatusBar();
    }
}

void FileView::updateContentLabel()
{
    d->initContentLabel();
    if (model()->currentState() == ModelState::kBusy
        || model()->canFetchMore(rootIndex())) {
        d->contentLabel->setText(QString());
        return;
    }

    if (count() <= 0) {
        // set custom empty tips
        const FileInfoPointer &fileInfo = model()->fileInfo(rootIndex());
        if (fileInfo) {
            d->contentLabel->setText(fileInfo->viewOfTip(ViewInfoType::kEmptyDir));
            d->contentLabel->adjustSize();
            return;
        }
    }

    d->contentLabel->setText(QString());
}

void FileView::updateSelectedUrl()
{
    if (d->preSelectionUrls.isEmpty() || model()->currentState() != ModelState::kIdle)
        return;

    selectFiles(d->preSelectionUrls);
    d->preSelectionUrls.clear();
}

void FileView::updateListHeaderView()
{
    if (!d->headerView)
        return;

    d->headerView->setModel(Q_NULLPTR);
    d->headerView->setModel(model());
    d->headerView->setRootIndex(rootIndex());

    d->headerView->setDefaultSectionSize(kDefualtHeaderSectionWidth);
    if (d->allowedAdjustColumnSize) {
        d->headerView->setSectionResizeMode(QHeaderView::Interactive);
        d->headerView->setMinimumSectionSize(kMinimumHeaderSectionWidth);
    } else {
        d->headerView->setSectionResizeMode(QHeaderView::Fixed);
        d->headerView->setMinimumSectionSize(kDefualtHeaderSectionWidth);
    }

    d->headerView->setSortIndicator(model()->getColumnByRole(model()->sortRole()), model()->sortOrder());
    d->columnRoles.clear();

    const QVariantMap &state = Application::appObtuselySetting()->value("WindowManager", "ViewColumnState").toMap();

    for (int i = 0; i < d->headerView->count(); ++i) {
        int logicalIndex = d->headerView->logicalIndex(i);
        d->columnRoles << model()->getRoleByColumn(i);

        if (d->allowedAdjustColumnSize) {
            ItemRoles curRole = d->columnRoles.last();
            int colWidth = state.value(QString::number(curRole), -1).toInt();
            if (colWidth > 0) {
                d->headerView->resizeSection(model()->getColumnByRole(curRole), colWidth);
            } else {
                if (curRole == kItemFileLastModifiedRole) {
                    d->headerView->resizeSection(model()->getColumnByRole(curRole), kDefaultItemFileLastModifiedWidth);
                }
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

QUrl FileView::parseSelectedUrl(const QUrl &url)
{
    auto fileUrl = url;
    QUrlQuery urlQuery;
    QByteArray encode = QUrl::toPercentEncoding(fileUrl.query(QUrl::FullyEncoded), "=");
    urlQuery.setQuery(encode);
    const auto &selectFile = urlQuery.queryItemValue("selectUrl", QUrl::FullyDecoded);

    const QUrl &defaultSelectUrl = QUrl::fromUserInput(selectFile);
    if (defaultSelectUrl.isValid()) {
        d->preSelectionUrls << defaultSelectUrl;
        urlQuery.removeQueryItem("selectUrl");
        fileUrl.setQuery(urlQuery);
    } else {
        // todo: liuzhangjian
        // checkGvfsMountfileBusy
        QList<QUrl> ancestors;
        if (const FileInfoPointer &currentFileInfo = InfoFactory::create<FileInfo>(rootUrl())) {
            if (UrlRoute::isAncestorsUrl(rootUrl(), fileUrl, &ancestors))
                d->preSelectionUrls << (ancestors.count() > 1 ? ancestors.at(ancestors.count() - 2) : rootUrl());
        }
    }

    return fileUrl;
}

void FileView::loadViewState(const QUrl &url)
{
    d->loadViewMode(url);

    QVariant defaultIconSize = Application::instance()->appAttribute(Application::kIconSizeLevel).toInt();
    d->currentIconSizeLevel = d->fileViewStateValue(url, "iconSizeLevel", defaultIconSize).toInt();
}

void FileView::onModelStateChanged()
{
    updateContentLabel();
    updateLoadingIndicator();
    updateSelectedUrl();

    if (d->headerView)
        d->headerView->setAttribute(Qt::WA_TransparentForMouseEvents, model()->currentState() == ModelState::kBusy);
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
    const FileInfoPointer &info = model()->fileInfo(index);

    if (!info)
        return;
    if (NetworkUtils::instance()->checkFtpOrSmbBusy(info->urlOf(UrlInfoType::kUrl))) {
        DialogManager::instance()->showUnableToVistDir(info->urlOf(UrlInfoType::kUrl).path());
        return;
    }

    FileOperatorHelperIns->openFiles(this, { info->urlOf(UrlInfoType::kUrl) });
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
