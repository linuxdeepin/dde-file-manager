// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "headerview.h"
#include "fileview.h"
#include "private/fileview_p.h"
#include "models/fileselectionmodel.h"
#include "models/fileviewmodel.h"
#include "baseitemdelegate.h"
#include "iconitemdelegate.h"
#include "listitemdelegate.h"
#include "listitempaintproxy.h"
#include "treeitempaintproxy.h"
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
#include "utils/viewanimationhelper.h"
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
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/utils/viewdefines.h>

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
#include <QLayout>

using namespace dfmplugin_workspace;
DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

FileView::FileView(const QUrl &url, QWidget *parent)
    : DListView(parent), d(new FileViewPrivate(this))
{
    d->url = url;
    setMinimumHeight(10);
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
//  TODO (search): perf
//  setLayoutMode(QListView::Batched);
#ifdef QT_SCROLL_WHEEL_ANI
    QScrollBar *bar = verticalScrollBar();
    bar->setSingleStep(1);
    setVerticalScrollBarPolicy(Qt::ScrollBarSlideAnimationOn);
#endif
    setVerticalScrollMode(ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    initializeModel();
    initializeDelegate();
    initializeStatusBar();
    initializeConnect();
    initializeScrollBarWatcher();
    initializePreSelectTimer();

    viewport()->installEventFilter(this);
}

FileView::~FileView()
{
    fmInfo() << "Destroying FileView for URL:" << rootUrl();

    disconnect(model(), &FileViewModel::stateChanged, this, &FileView::onModelStateChanged);
    disconnect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileView::onSelectionChanged);

    dpfSignalDispatcher->unsubscribe("dfmplugin_workspace", "signal_View_HeaderViewSectionChanged", this, &FileView::onHeaderViewSectionChanged);
    dpfSignalDispatcher->unsubscribe("dfmplugin_filepreview", "signal_ThumbnailDisplay_Changed", this, &FileView::onWidgetUpdate);

    fmDebug() << "FileView destruction completed";
}

QWidget *FileView::widget() const
{
    return const_cast<FileView *>(this);
}

QWidget *FileView::contentWidget() const
{
    return findChild<QWidget *>("qt_scrollarea_viewport");
}

void FileView::setViewMode(Global::ViewMode mode)
{
    // itemDelegate 未设置时为未初始化状态，此时调用setViewMode需要执行设置流程
    // itemDelegate 已设置时，若view不可见，则暂不执行viewMode设置逻辑
    if (!isVisible() && itemDelegate()
        && d->delegates[static_cast<int>(mode)] == itemDelegate()) {
        fmDebug() << "View mode change skipped: view not visible and delegate already set for mode:" << static_cast<int>(mode);
        return;
    }

    if (itemDelegate())
        itemDelegate()->hideAllIIndexWidget();

    int delegateModeIndex = mode == Global::ViewMode::kTreeMode ? static_cast<int>(Global::ViewMode::kListMode) : static_cast<int>(mode);
    if (d->delegates.keys().contains(delegateModeIndex)) {
        d->currentViewMode = mode;
        fmDebug() << "View mode set successfully to:" << static_cast<int>(mode);
    } else {
        fmWarning() << QString("The view mode %1 is not support in this dir! This view will set default mode.").arg(delegateModeIndex);
        d->currentViewMode = Global::ViewMode::kIconMode;
        delegateModeIndex = static_cast<int>(Global::ViewMode::kIconMode);
    }

    // 切换视图模式前记录当前选中文件，以便切换视图模式后文件数量变化了也能保持存在文件的选中状态。
    recordSelectedUrls();

    setItemDelegate(d->delegates[delegateModeIndex]);
    switch (d->currentViewMode) {
    case Global::ViewMode::kIconMode:
        d->initHorizontalOffset = false;
        setUniformItemSizes(false);
        setResizeMode(QListView::Adjust);
        setMovement(QListView::Static);
        setOrientation(QListView::LeftToRight, true);
#ifdef DTKWIDGET_CLASS_DSizeMode
        setSpacing(DSizeModeHelper::element(kCompactIconViewSpacing, kIconViewSpacing));
#else
        setSpacing(kIconViewSpacing);
#endif
        d->initIconModeView();
        setMinimumWidth(0);
        model()->setTreeView(false);
        verticalScrollBar()->setFixedHeight(rect().height() - d->statusBar->height());
        break;
    case Global::ViewMode::kListMode:
        d->delegates[static_cast<int>(Global::ViewMode::kListMode)]->setPaintProxy(new ListItemPaintProxy(this));
        setIconSize(QSize(kListViewIconSize, kListViewIconSize));
        viewport()->setContentsMargins(0, 0, 0, 0);
        d->delegates[static_cast<int>(Global::ViewMode::kListMode)]->setPaintProxy(new ListItemPaintProxy(this));
        model()->setTreeView(false);
        setListViewMode();
        break;
    case Global::ViewMode::kExtendMode:
        break;
    case Global::ViewMode::kTreeMode:
        viewport()->setContentsMargins(0, 0, 0, 0);
        if (d->itemsExpandable) {
            auto proxy = new TreeItemPaintProxy(this);
            proxy->setStyleProxy(style());
            d->delegates[static_cast<int>(Global::ViewMode::kListMode)]->setPaintProxy(proxy);
            model()->setTreeView(true);
        } else {
            d->delegates[static_cast<int>(Global::ViewMode::kListMode)]->setPaintProxy(new ListItemPaintProxy(this));
            model()->setTreeView(false);
        }
        setIconSize(QSize(kListViewIconSize, kListViewIconSize));
        setListViewMode();
        break;
    case Global::ViewMode::kAllViewMode:
        break;
    default:
        break;
    }
    fmDebug() << "View mode change completed for URL:" << rootUrl().toString();
}

Global::ViewMode FileView::currentViewMode() const
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
    d->url = url;
    setFocus();

    clearSelection();
    selectionModel()->clear();
    d->statusBar->itemCounted(0);

    const QUrl &fileUrl = parseSelectedUrl(url);

    const QModelIndex &index = model()->setRootUrl(fileUrl);
    d->itemsExpandable = DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool()
            && WorkspaceHelper::instance()->isViewModeSupported(rootUrl().scheme(), DFMGLOBAL_NAMESPACE::ViewMode::kTreeMode);

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

    fmDebug() << "FileView: root URL set successfully to" << url.toString();
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
    fmDebug() << "FileView: refresh requested for URL" << rootUrl().toString();
    if (NetworkUtils::instance()->checkFtpOrSmbBusy(rootUrl())) {
        fmWarning() << "FileView: cannot refresh - FTP or SMB is busy for URL" << rootUrl().toString();
        DialogManager::instance()->showUnableToVistDir(rootUrl().path());
        return;
    }
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

void FileView::stopWork(const QUrl &newUrl)
{
    model()->stopTraversWork(newUrl);
}

void FileView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    DListView::dataChanged(topLeft, bottomRight, roles);

    d->selectHelper->resortSelectFiles();
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

    if (d->headerView->length() != d->oldHeaderViewLenght) {
        QList<ItemRoles> roleList = d->columnRoles;
        QVariantMap state;
        for (const ItemRoles role : roleList) {
            int colWidth = getColumnWidth(model()->getColumnByRole(role));

            if (colWidth > 0)
                state[QString::number(role)] = colWidth;
        }
        Application::appObtuselySetting()->setValue("WindowManager", "ViewColumnState", state);
    }
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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QStyleOptionViewItem option = viewOptions();
#else
    QStyleOptionViewItem option;
    initViewItemOption(&option);
#endif

    option.rect.setWidth(QWIDGETSIZE_MAX);
    option.rect.setHeight(itemSizeHint().height());

    int columnMaxWidth = 0;

    for (int i = 0; i < rowCount; ++i) {
        const QModelIndex &index = model()->index(i, 0, rootIndex());
        const QList<QRect> &list = itemDelegate()->paintGeomertys(option, index, true);

        // 第0列为文件名列，此列比较特殊，因为前面还有文件图标占用了一部分空间
        int width = 0;

        if (logicalIndex == 0) {
            // 树形视图多绘制一个扩展标识，名称是第三列
            width = list.at(currentViewMode() == Global::ViewMode::kTreeMode ? 2 : 1).right() + kColumnPadding / 2;
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
    fmDebug() << "Header section move completed and published for URL:" << rootUrl.toString();
}

void FileView::onHeaderHiddenChanged(const QString &roleName, const bool isHidden)
{
    fmDebug() << "Header hidden changed - role:" << roleName << "hidden:" << isHidden << "for URL:" << rootUrl().toString();

    d->columnForRoleHiddenMap[roleName] = isHidden;

    if (d->allowedAdjustColumnSize) {
        updateListHeaderView();
    } else {
        d->headerView->updateColumnWidth();
    }
}

void FileView::onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    if (model()->currentState() == ModelState::kBusy) {
        fmDebug() << "Sort indicator change ignored: model is busy for URL:" << rootUrl().toString();
        return;
    }

    fmDebug() << "Sort indicator changed - column:" << logicalIndex << "order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending") << "for URL:" << rootUrl().toString();

    recordSelectedUrls();

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

    fmDebug() << "Item clicked:" << url.toString() << "for URL:" << rootUrl().toString();
}

void FileView::onDoubleClicked(const QModelIndex &index)
{
    fmDebug() << "Item double clicked for URL:" << rootUrl().toString();
    openIndexByClicked(ClickedAction::kDoubleClicked, index);
}

void FileView::wheelEvent(QWheelEvent *event)
{
    // Handle size control for all view modes with Ctrl + wheel
    if (WindowUtils::keyCtrlIsPressed()) {
        if (event->angleDelta().y() > 0) {
            increaseIcon();
        } else {
            decreaseIcon();
        }
        emit viewStateChanged();
        event->accept();
        return;
    }

    // Handle horizontal scrolling with Alt modifier or horizontal wheel
    if (event->modifiers() == Qt::AltModifier || event->angleDelta().x() != 0) {
        horizontalScrollBar()->setSliderPosition(horizontalScrollBar()->sliderPosition() - event->angleDelta().x());
        return;
    }

    // Default vertical scrolling
#ifdef QT_SCROLL_WHEEL_ANI
    DListView::wheelEvent(event);
#else
    verticalScrollBar()->setSliderPosition(verticalScrollBar()->sliderPosition() - event->angleDelta().y());
#endif
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
    const QUrl &url = rootUrl();
    setFileViewStateValue(url, "iconSizeLevel", value);
}

void FileView::delayUpdateStatusBar()
{
    if (d->updateStatusBarTimer)
        d->updateStatusBarTimer->start();
}

void FileView::viewModeChanged(quint64 windowId, int viewMode)
{
    Global::ViewMode mode = static_cast<Global::ViewMode>(viewMode);
    if (currentViewMode() == mode) {
        qWarning() << "Current view mode equal to the new view mode that switched by global event. Don't need to do anything.";
        return;
    }

    if (mode == Global::ViewMode::kIconMode || mode == Global::ViewMode::kListMode || mode == Global::ViewMode::kTreeMode) {
        setViewMode(mode);
    }

    saveViewModeState();
}

FileView::RandeIndexList FileView::visibleIndexes(const QRect &rect) const
{
    RandeIndexList list;

    QSize itemSize = itemSizeHint();

    int count = this->count();
    int spacing = this->spacing();
    // int itemWidth = itemSize.width() + spacing * 2;
    int itemHeight = itemSize.height() + spacing * 2;

    if (isListViewMode() || isTreeViewMode()) {
        int firstIndex = (rect.top() + spacing) / itemHeight;
        int lastIndex = (rect.bottom() - spacing) / itemHeight;

        if (firstIndex >= count)
            return list;

        list << RandeIndex(qMax(firstIndex, 0), qMin(lastIndex, count - 1));
    } else if (isIconViewMode()) {
        int columnCount = d->calcColumnCount(rect.width(), itemSize.width());

        list << calcRectContiansIndexes(columnCount, rect);
    }

    return list;
}

FileView::RandeIndexList FileView::rectContainsIndexes(const QRect &rect) const
{
    RandeIndexList list;

    QSize itemSize = itemSizeHint();

    int count = this->count();
    int spacing = this->spacing();
    int itemWidth = itemSize.width() + spacing * 2;
    int itemHeight = itemSize.height() + spacing * 2;

    if (isListViewMode() || isTreeViewMode()) {
        int firstIndex = (rect.top() + spacing) / itemHeight;
        int lastIndex = (rect.bottom() - spacing) / itemHeight;

        if (firstIndex >= count)
            return list;

        list << RandeIndex(qMax(firstIndex, 0), qMin(lastIndex, count - 1));
    } else if (isIconViewMode()) {
        int columnCount = d->iconModeColumnCount(itemWidth);
        list << calcRectContiansIndexes(columnCount, rect);
    }

    return list;
}

FileView::RandeIndexList FileView::calcRectContiansIndexes(int columnCount, const QRect &rect) const
{
    RandeIndexList list {};

    QSize itemSize = itemSizeHint();
    QSize aIconSize = iconSize();

    int count = this->count();
    int spacing = this->spacing();
    int itemWidth = itemSize.width() + spacing * 2;
    int itemHeight = itemSize.height() + spacing * 2;
    QRect validRect = rect.marginsRemoved(QMargins(spacing, spacing, spacing, spacing));

    if (columnCount <= 0)
        return list;

    int beginRowIndex = validRect.top() / itemHeight;
    int endRowIndex = validRect.bottom() / itemHeight;
    int beginColumnIndex = validRect.left() / itemWidth;
    int endColumnIndex = validRect.right() / itemWidth;

    if (validRect.top() % itemHeight > aIconSize.height())
        ++beginRowIndex;

    int iconMargin = (itemWidth - aIconSize.width()) / 2;

    if (validRect.left() % itemWidth > itemWidth - iconMargin)
        ++beginColumnIndex;

    if (validRect.right() % itemWidth < iconMargin)
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

bool FileView::selectFiles(const QList<QUrl> &files) const
{
    if (isTreeViewMode())
        return d->selectHelper->select(files);

    if (files.isEmpty())
        return false;

    // do not check parent if root is virtual path.
    if (rootUrl().scheme() == Global::Scheme::kFile) {
        if (UniversalUtils::urlEquals(files.first().adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash), rootUrl()))
            return d->selectHelper->select(files);
        return false;
    }

    return d->selectHelper->select(files);
}

void FileView::setSelectionMode(const QAbstractItemView::SelectionMode mode)
{
    if (d->enabledSelectionModes.contains(mode))
        QAbstractItemView::setSelectionMode(mode);
}

void FileView::reverseSelect() const
{
    d->shortcutHelper->reverseSelect();
}

void FileView::setEnabledSelectionModes(const QList<QAbstractItemView::SelectionMode> &modes)
{
    d->enabledSelectionModes = modes;
    if (!modes.contains(selectionMode()))
        resetSelectionModes();
}

void FileView::setSort(const ItemRoles role, const Qt::SortOrder order)
{
    if (model()->currentState() == ModelState::kBusy)
        return;

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

// Modern grouping interface
void FileView::setGroupingStrategy(const QString &strategyName)
{
    if (model()->currentState() == ModelState::kBusy) {
        fmWarning() << "Cannot set grouping strategy while model is busy for URL:" << rootUrl().toString();
        return;
    }

    fmInfo() << "Setting grouping strategy:" << strategyName << "for URL:" << rootUrl().toString();

    // Delegate to model
    model()->setGroupingStrategy(strategyName);
}

void FileView::setGroupingEnabled(bool enabled)
{
    if (model()->currentState() == ModelState::kBusy) {
        fmWarning() << "Cannot change grouping enabled state while model is busy for URL:" << rootUrl().toString();
        return;
    }

    fmInfo() << "Setting grouping enabled:" << enabled << "for URL:" << rootUrl().toString();

    // Delegate to model
    model()->setGroupingEnabled(enabled);
}

void FileView::setGroupingOrder(Qt::SortOrder order)
{
    if (model()->currentState() == ModelState::kBusy) {
        fmWarning() << "Cannot change grouping order while model is busy for URL:" << rootUrl().toString();
        return;
    }

    fmInfo() << "Setting grouping order:" << (order == Qt::AscendingOrder ? "Ascending" : "Descending")
             << "for URL:" << rootUrl().toString();

    // Delegate to model
    model()->setGroupingOrder(order);
}

QString FileView::getGroupingStrategy() const
{
    return model() ? model()->getGroupingStrategy() : QString("NoGroupStrategy");
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

void FileView::updateViewportContentsMargins(const QSize &itemSize)
{
    if (isListViewMode() || isTreeViewMode() || itemSize.width() <= spacing())
        return;
    int itemWidth = itemSize.width() + 2 * spacing();
    int iconHorizontalMargin = kIconHorizontalMargin;
#ifdef DTKWIDGET_CLASS_DSizeMode
    iconHorizontalMargin = DSizeModeHelper::element(kCompactIconHorizontalMargin, kIconHorizontalMargin);
#endif

    if (itemWidth < 2 * kIconHorizontalMargin)
        return;

    int contentsWidth = contentsSize().width();
    if (contentsWidth < itemWidth) {
        viewport()->setContentsMargins(0, 0, 0, 0);
        return;
    }
    int widthModel = (contentsWidth - 1) % itemWidth;
    if (widthModel >= iconHorizontalMargin && widthModel <= itemWidth - kIconHorizontalMargin) {
        viewport()->setContentsMargins(0, 0, 0, 0);
        return;
    }
    int margin = kIconHorizontalMargin - (widthModel >= iconHorizontalMargin ? itemWidth - widthModel : widthModel);
    if (margin <= 0 || margin > kIconHorizontalMargin) {
        viewport()->setContentsMargins(0, 0, 0, 0);
        return;
    }
    viewport()->setContentsMargins(margin, 0, margin, 0);
}

bool FileView::indexInRect(const QRect &actualRect, const QModelIndex &index)
{
    auto paintRect = visualRect(index);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto opt = viewOptions();
#else
    QStyleOptionViewItem opt;
    initViewItemOption(&opt);
#endif
    opt.rect = paintRect;
    auto rectList = itemDelegate()->itemGeomertys(opt, index);
    for (const auto &rect : rectList) {
        if (!(actualRect.left() > rect.right()
              || actualRect.top() > rect.bottom()
              || rect.left() > actualRect.right()
              || rect.top() > actualRect.bottom()))
            return true;
    }

    return false;
}

QList<QUrl> FileView::selectedTreeViewUrlList() const
{
    if (isIconViewMode() || !d->itemsExpandable)
        return selectedUrlList();

    QModelIndex rootIndex = this->rootIndex();
    QList<QUrl> list;

    QModelIndex expandIndex;
    auto selectIndex = selectedIndexes();
    if (selectIndex.count() < 1)
        return list;
    if (selectIndex.count() >= 2)
        std::sort(selectIndex.begin(), selectIndex.end(),
                  [](const QModelIndex &left, const QModelIndex &right) {
                      return left.row() < right.row();
                  });
    for (const QModelIndex &index : selectIndex) {
        bool expandIsParent = false;
        if (expandIndex.isValid()) {
            auto parentUrl = expandIndex.data(Global::ItemRoles::kItemUrlRole).toUrl();
            auto child = index.data(Global::ItemRoles::kItemUrlRole).toUrl();
            expandIsParent = (index.data(Global::ItemRoles::kItemTreeViewDepthRole).toInt()
                              > expandIndex.data(Global::ItemRoles::kItemTreeViewDepthRole).toInt())
                    && UniversalUtils::isParentUrl(child, parentUrl);
        }
        if (index.parent() != rootIndex || (expandIndex.isValid() && expandIsParent))
            continue;
        if (!expandIndex.isValid() || !expandIsParent) {
            list << model()->data(index, ItemRoles::kItemUrlRole).toUrl();
            if (index.data(Global::ItemRoles::kItemTreeViewExpandedRole).toBool()) {
                expandIndex = index;
            } else if (expandIndex.isValid()) {
                expandIndex = QModelIndex();
            }
        }
    }

    return list;
}

void FileView::selectedTreeViewUrlList(QList<QUrl> &selectedUrls, QList<QUrl> &treeSelectedUrls) const
{
    selectedUrls.clear();
    treeSelectedUrls.clear();
    if (isIconViewMode() || !d->itemsExpandable)
        return selectedUrls.append(selectedUrlList());

    QModelIndex rootIndex = this->rootIndex();

    QModelIndex expandIndex;
    auto selectIndex = selectedIndexes();
    if (selectIndex.count() < 1)
        return;
    if (selectIndex.count() >= 2)
        std::sort(selectIndex.begin(), selectIndex.end(),
                  [](const QModelIndex &left, const QModelIndex &right) {
                      return left.row() < right.row();
                  });
    for (const QModelIndex &index : selectIndex) {
        selectedUrls.append(index.data(Global::ItemRoles::kItemUrlRole).toUrl());
        bool expandIsParent = false;
        if (expandIndex.isValid()) {
            auto parentUrl = expandIndex.data(Global::ItemRoles::kItemUrlRole).toUrl();
            auto child = index.data(Global::ItemRoles::kItemUrlRole).toUrl();
            expandIsParent = (index.data(Global::ItemRoles::kItemTreeViewDepthRole).toInt()
                              > expandIndex.data(Global::ItemRoles::kItemTreeViewDepthRole).toInt())
                    && UniversalUtils::isParentUrl(child, parentUrl);
        }
        if (index.parent() != rootIndex || (expandIndex.isValid() && expandIsParent))
            continue;
        if (!expandIndex.isValid() || !expandIsParent) {
            treeSelectedUrls << model()->data(index, ItemRoles::kItemUrlRole).toUrl();
            if (index.data(Global::ItemRoles::kItemTreeViewExpandedRole).toBool()) {
                expandIndex = index;
            } else if (expandIndex.isValid()) {
                expandIndex = QModelIndex();
            }
        }
    }

    return;
}

QRect FileView::calcVisualRect(int widgetWidth, int index) const
{
    int iconViewSpacing = spacing();
    int iconHorizontalMargin = spacing();
#ifdef DTKWIDGET_CLASS_DSizeMode
    iconHorizontalMargin = DSizeModeHelper::element(kCompactIconHorizontalMargin, spacing());
#endif
    QSize itemSize = itemSizeHint();

    // 计算列数
    int columnCount = d->calcColumnCount(widgetWidth, itemSize.width());
    if (columnCount == 0)
        return QRect();

    int columnIndex = index % columnCount;
    int rowIndex = index / columnCount;

    int iconVerticalTopMargin = 0;
#ifdef DTKWIDGET_CLASS_DSizeMode
    iconVerticalTopMargin = DSizeModeHelper::element(kCompactIconVerticalTopMargin, spacing());
#endif

    QRect rect;
    // 计算顶部位置：上边距(VerticalTopMargin) + 行索引 * (项目高度 + 间距)
    rect.setTop(iconVerticalTopMargin + rowIndex * (itemSize.height() + 2 * iconViewSpacing));

    // 计算左侧位置：左边距(kIconHorizontalMargin) + 列索引 * (项目宽度 + 间距)
    rect.setLeft(iconHorizontalMargin + columnIndex * (itemSize.width() + 2 * iconViewSpacing));

    rect.setSize(itemSize);

    // 计算水平居中偏移，仅当行数大于1时才应用
    int totalItems = model()->rowCount();
    int rowCount = (totalItems + columnCount - 1) / columnCount;   // 向上取整
    if (rowCount > 1) {   // 计算可用宽度（减去左右边距）
        int availableWidth = widgetWidth - 2 * iconHorizontalMargin;
        int totalItemsWidth = columnCount * itemSize.width() + (columnCount - 1) * 2 * iconViewSpacing;
        int horizontalOffset = (availableWidth - totalItemsWidth) / 2;
        rect.moveLeft(rect.left() + horizontalOffset);
    }

    rect.moveTop(rect.top() - verticalOffset());

    return rect;
}

void FileView::aboutToChangeWidth(int deltaWidth)
{
    if (!isIconViewMode())
        return;

    d->animationHelper->initAnimationHelper();
    d->animationHelper->aboutToPlay();
    d->animationHelper->playAnimationWithWidthChange(deltaWidth);
}

void FileView::initDefaultHeaderView()
{
    if (!model())
        return;

    auto roleName = model()->roleDisplayString(kItemFileCreatedRole);
    d->columnForRoleHiddenMap[roleName] = true;
}

void FileView::onHeaderViewMousePressed()
{
    d->oldHeaderViewLenght = d->headerView->length();
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (itemDelegate())
        return itemDelegate()->sizeHint(viewOptions(), rootIndex());
#else
    if (itemDelegate()) {
        QStyleOptionViewItem option;
        initViewItemOption(&option);
        return itemDelegate()->sizeHint(option, rootIndex());
    }
#endif

    return QSize();
}

void FileView::increaseIcon()
{
    if (!itemDelegate())
        return;

    int newLevel = itemDelegate()->increaseIcon();
    if (newLevel < 0)
        return;

    const QUrl &url = rootUrl();
    if (isIconViewMode()) {
        setFileViewStateValue(url, "iconSizeLevel", newLevel);
        fmDebug() << "Icon size level increased to:" << newLevel;
    } else if (isListViewMode() || isTreeViewMode()) {
        setFileViewStateValue(url, "listHeightLevel", newLevel);
        fmDebug() << "List height level increased to:" << newLevel;
    }
}

void FileView::decreaseIcon()
{
    if (!itemDelegate())
        return;

    int newLevel = itemDelegate()->decreaseIcon();
    if (newLevel < 0)
        return;

    const QUrl &url = rootUrl();
    if (isIconViewMode()) {
        setFileViewStateValue(url, "iconSizeLevel", newLevel);
        fmDebug() << "Icon size level decreased to:" << newLevel;
    } else if (isListViewMode() || isTreeViewMode()) {
        setFileViewStateValue(url, "listHeightLevel", newLevel);
        fmDebug() << "List height level decreased to:" << newLevel;
    }
}

void FileView::setIconSizeBySizeIndex(const int sizeIndex)
{
    QSignalBlocker blocker(d->statusBar->scalingSlider());
    Q_UNUSED(blocker)

    d->currentIconSizeLevel = sizeIndex;
    d->statusBar->scalingSlider()->setValue(sizeIndex);
}

void FileView::onShowFileSuffixChanged(bool isShow)
{
    Q_UNUSED(isShow);
    update();
}

void FileView::updateHorizontalOffset()
{
    d->updateHorizontalOffset();

    // Update editor positions after horizontal offset changes
    updateEditorGeometries();
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

    emit selectUrlChanged(selectedUrlList());

    quint64 winId = WorkspaceHelper::instance()->windowId(this);
    WorkspaceEventCaller::sendViewSelectionChanged(winId, selected, deselected);
}

void FileView::onDefaultViewModeChanged(int mode)
{
    Global::ViewMode newMode = static_cast<Global::ViewMode>(mode);

    if (!WorkspaceHelper::instance()->isViewModeSupported(rootUrl().scheme(), newMode)) {
        fmWarning() << "View mode not supported for scheme:" << rootUrl().scheme() << "mode:" << mode;
        return;
    }

    if (newMode == d->currentViewMode) {
        fmDebug() << "View mode unchanged, skipping update";
        return;
    }

    Global::ViewMode oldMode = d->currentViewMode;
    d->loadViewMode(rootUrl());

    if (oldMode == d->currentViewMode) {
        fmDebug() << "View mode unchanged after loading, skipping update";
        return;
    }

    fmDebug() << "Switching view mode from" << static_cast<int>(oldMode) << "to" << static_cast<int>(d->currentViewMode) << "for URL:" << rootUrl().toString();
    setViewMode(d->currentViewMode);
}

void FileView::onIconSizeChanged(int sizeIndex)
{
    if (!itemDelegate())
        return;

    if (itemDelegate()->iconSizeLevel() == sizeIndex && d->currentIconSizeLevel == sizeIndex)
        return;

    itemDelegate()->setIconSizeByIconSizeLevel(sizeIndex);
    setIconSizeBySizeIndex(sizeIndex);
}

void FileView::onItemWidthLevelChanged(int level)
{
    if (!itemDelegate())
        return;

    // Check if this is an icon delegate that supports width level control
    auto iconDelegate = dynamic_cast<IconItemDelegate *>(itemDelegate());
    if (!iconDelegate)
        return;

    if (iconDelegate->minimumWidthLevel() == level && d->currentGridDensityLevel == level)
        return;

    d->currentGridDensityLevel = level;
    iconDelegate->setItemMinimumWidthByWidthLevel(level);
    doItemsLayout();
    updateHorizontalOffset();
}

void FileView::onItemHeightLevelChanged(int level)
{
    if (!itemDelegate())
        return;

    if (!d->fileViewHelper->canChangeListItemHeight())
        return;

    if (itemDelegate()->iconSizeLevel() == level && d->currentListHeightLevel == level)
        return;

    d->currentListHeightLevel = level;
    itemDelegate()->setIconSizeByIconSizeLevel(level);
    doItemsLayout();
}

bool FileView::isIconViewMode() const
{
    return d->currentViewMode == Global::ViewMode::kIconMode;
}

bool FileView::isListViewMode() const
{
    return d->currentViewMode == Global::ViewMode::kListMode;
}

bool FileView::isTreeViewMode() const
{
    return d->currentViewMode == Global::ViewMode::kTreeMode;
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
    fmInfo() << "Navigating up from URL:" << oldCurrentUrl;

    QUrl parentUrl = UrlRoute::urlParent(oldCurrentUrl);

    if (parentUrl.isValid()) {
        fmDebug() << "Parent URL found:" << parentUrl;
        FileOperatorHelperIns->openFilesByMode(this, { parentUrl }, DirOpenMode::kOpenInCurrentWindow);
        return true;
    } else {
        fmDebug() << "No valid parent URL, navigating to computer root";
        auto windowId = WorkspaceHelper::instance()->windowId(this);
        QUrl computerRoot;
        computerRoot.setScheme(Global::Scheme::kComputer);
        computerRoot.setPath("/");
        WorkspaceEventCaller::sendChangeCurrentUrl(windowId, computerRoot);
    }
    return false;
}

QModelIndex FileView::iconIndexAt(const QPoint &pos, const QSize &itemSize) const
{
    if (isListViewMode() || isTreeViewMode())
        return QModelIndex();

    int iconVerticalTopMargin = 0;
#ifdef DTKWIDGET_CLASS_DSizeMode
    iconVerticalTopMargin = DSizeModeHelper::element(kCompactIconVerticalTopMargin, spacing());
#endif

    if (itemDelegate() && itemDelegate()->itemExpanded() && itemDelegate()->expandItemRect().contains(pos)) {
        return itemDelegate()->expandedIndex();
    }

    QPoint actualPos = QPoint(pos.x() + horizontalOffset(), pos.y() + verticalOffset() - iconVerticalTopMargin);
    auto index = FileViewHelper::caculateIconItemIndex(this, itemSize, actualPos);

    if (index == -1 || index >= model()->rowCount(rootIndex()))
        return QModelIndex();

    auto currentIndex = model()->index(index, 0, rootIndex());
    auto paintRect = visualRect(currentIndex);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto opt = viewOptions();
#else
    QStyleOptionViewItem opt;
    initViewItemOption(&opt);
#endif
    opt.rect = paintRect;
    auto rectList = itemDelegate()->itemGeomertys(opt, currentIndex);
    for (const auto &rect : rectList) {
        if (rect.contains(pos))
            return currentIndex;
    }
    return QModelIndex();
}

bool FileView::expandOrCollapseItem(const QModelIndex &index, const QPoint &pos)
{
    QRect arrowRect = itemDelegate()->getRectOfItem(RectOfItemType::kItemTreeArrowRect, index);

    if (!arrowRect.contains(pos))
        return false;

    // get expand state
    bool expanded = model()->data(index, kItemTreeViewExpandedRole).toBool();
    if (expanded) {
        // do collapse
        fmInfo() << "do collapse item, index = " << index << index.row() << model()->data(index, kItemUrlRole).toUrl();
        model()->doCollapse(index);
    } else {
        // do expanded
        fmInfo() << "do expanded item, index = " << index << index.row() << model()->data(index, kItemUrlRole).toUrl();
        model()->doExpand(index);
    }

    return true;
}

void FileView::recordSelectedUrls()
{
    auto selectedUrls = selectedUrlList();
    if (!selectedUrls.isEmpty()) {
        const QUrl curFile = model()->data(currentIndex(), ItemRoles::kItemUrlRole).toUrl();
        d->selectHelper->saveSelectedFilesList(curFile, selectedUrls);
    }
}

DirOpenMode FileView::currentDirOpenMode() const
{
    DirOpenMode mode;

    if (d->isAlwaysOpenInCurrentWindow) {
        mode = DirOpenMode::kAwaysInCurrentWindow;
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

void FileView::onRenameProcessStarted()
{
    if (d->shortcutHelper) {
        d->shortcutHelper->renameProcessing();
    }
}

void FileView::onAboutToSwitchListView(const QList<QUrl> &allShowList)
{
    d->selectHelper->filterSelectedFiles(allShowList);
}

void FileView::onRowCountChanged()
{
    // clean selected indexes
    // the selectList will be reseted while call the selectedIndexes() at next time
    static_cast<FileSelectionModel *>(selectionModel())->clearSelectList();

    d->selectHelper->resortSelectFiles();

    delayUpdateStatusBar();
    updateContentLabel();

    if (isIconViewMode()) {
        updateHorizontalOffset();
    }
}

void FileView::setFilterData(const QUrl &url, const QVariant &data)
{
    fmDebug() << "Setting filter data for URL:" << url << "current URL:" << rootUrl();

    if (url == rootUrl() && isVisible()) {
        fmInfo() << "Applying filter data to current view";
        clearSelection();
        model()->setFilterData(data);
    } else {
        fmDebug() << "Filter data not applied - URL mismatch or view not visible";
    }
}

void FileView::setFilterCallback(const QUrl &url, const FileViewFilterCallback callback)
{
    fmDebug() << "Setting filter callback for URL:" << url << "current URL:" << rootUrl();

    if (url == rootUrl() && isVisible()) {
        fmInfo() << "Applying filter callback to current view";
        clearSelection();
        model()->setFilterCallback(callback);
    } else {
        fmDebug() << "Filter callback not applied - URL mismatch or view not visible";
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

void FileView::onAppAttributeChanged(const QString &group, const QString &key, const QVariant &value)
{
    if (group == "FileViewState") {
        if (isListViewMode() || isTreeViewMode()) {
            int configListHeightLevel = d->fileViewStateValue(rootUrl(), "listHeightLevel", d->currentListHeightLevel).toInt();
            onItemHeightLevelChanged(configListHeightLevel);
        }

        if (isIconViewMode()) {
            int configGridDensityLevel = d->fileViewStateValue(rootUrl(), "gridDensityLevel", d->currentGridDensityLevel).toInt();
            onItemWidthLevelChanged(configGridDensityLevel);

            int configIconSizeLevel = d->fileViewStateValue(rootUrl(), "iconSizeLevel", d->currentIconSizeLevel).toInt();
            onIconSizeChanged(configIconSizeLevel);
        }
    }
}

bool FileView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    if (selectedIndexCount() > 1)
        return false;

    return DListView::edit(index, trigger, event);
}

void FileView::resizeEvent(QResizeEvent *event)
{
    d->initHorizontalOffset = false;

    d->isResizeEvent = true;
    DListView::resizeEvent(event);
    d->isResizeEvent = false;

    updateHorizontalOffset();

    // TODO(liuyangming) crash when launch via command with params.
    if (itemDelegate() && itemDelegate()->editingIndex().isValid())
        doItemsLayout();

    if (isIconViewMode()) {
        updateViewportContentsMargins(itemSizeHint());
        if (model()->currentState() == ModelState::kIdle && event->size().width() != event->oldSize().width()) {
            d->animationHelper->playViewAnimation();
        }
    }

    verticalScrollBar()->setFixedHeight(rect().height() - d->statusBar->height() - (d->headerView ? d->headerView->height() : 0));
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

        if (itemDelegate())
            itemDelegate()->commitDataAndCloseActiveEditor();

        if (d->currentViewMode == Global::ViewMode::kTreeMode && d->itemsExpandable) {
            if (index.data(kItemTreeViewCanExpandRole).toBool() && expandOrCollapseItem(index, event->pos())) {
                d->lastMousePressedIndex = QModelIndex();
                d->pressedStartWithExpand = true;
                return;
            }
        }

        d->selectHelper->click(isEmptyArea ? QModelIndex() : index);

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
    if (d->pressedStartWithExpand)
        return;

    if (event->buttons() & Qt::LeftButton)
        d->mouseMoveRect = QRect(event->globalPos(), d->mouseLastPos);

    DListView::mouseMoveEvent(event);
}

void FileView::mouseReleaseEvent(QMouseEvent *event)
{
    d->pressedStartWithExpand = false;

    if (event->buttons() & Qt::LeftButton) {
        d->mouseMoveRect = QRect(-1, -1, 1, 1);
        d->mouseLastPos = QPoint(0, 0);
    }

    d->selectHelper->release();

    if (!QScroller::hasScroller(this))
        return DListView::mouseReleaseEvent(event);

    if (WindowUtils::keyCtrlIsPressed()
        && d->lastMousePressedIndex.isValid()
        && d->lastMousePressedIndex == indexAt(event->pos())) {
        selectionModel()->select(d->lastMousePressedIndex, QItemSelectionModel::Deselect);
    }
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
    d->dragDropHelper->drop(event);
    setState(QAbstractItemView::NoState);
}

QModelIndex FileView::indexAt(const QPoint &pos) const
{
    QSize itemSize = itemSizeHint();
    if (isIconViewMode())
        return iconIndexAt(pos, itemSize);

    QPoint actualPos = QPoint(pos.x() + horizontalOffset(), pos.y() + verticalOffset());
    int index = FileViewHelper::caculateListItemIndex(itemSize, actualPos);

    if (index == -1 || index >= model()->rowCount(rootIndex()))
        return QModelIndex();

    return model()->index(index, 0, rootIndex());
}

QRect FileView::visualRect(const QModelIndex &index) const
{
    QRect rect;
    if (index.column() != 0)
        return rect;

    if (isListViewMode() || isTreeViewMode()) {
        rect = DListView::visualRect(index);
        rect.moveLeft(rect.left() - horizontalScrollBar()->value());
    } else {
        rect = DListView::visualRect(index);
        if (!d->initHorizontalOffset) {
            d->initHorizontalOffset = true;
            d->updateHorizontalOffset();
            rect = DListView::visualRect(index);
        }
    }

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
    int totalHeight = 0;
    if (isIconViewMode()) {
        int iconVerticalTopMargin = 0;
#ifdef DTKWIDGET_CLASS_DSizeMode
        iconVerticalTopMargin = DSizeModeHelper::element(kCompactIconVerticalTopMargin, spacing());
#endif
        if (!d->isResizeEvent
            || (d->isResizeEvent && d->lastContentHeight > 0 && d->lastContentHeight != contentsSize().height()))
            resizeContents(contentsSize().width(), contentsSize().height() + iconVerticalTopMargin);

        totalHeight = contentsSize().height();
        d->lastContentHeight = totalHeight;
    } else {
        int rowCount = model()->rowCount(rootIndex());
        int listHeight = rowCount * itemSizeHint().height() + kListModeBottomMargin;
        int contentHeight = contentsSize().height();

        totalHeight = listHeight > contentHeight ? listHeight : contentHeight;
    }

    if (!d->headerView || !d->allowedAdjustColumnSize) {
        return DListView::updateGeometries();
    }

    resizeContents(d->headerView->length(), totalHeight);
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
            fmDebug() << "Change supported actions: " << defaultDropAction;
            supportedActions = defaultDropAction;
        }

        QList<QUrl> transformedUrls;
        UniversalUtils::urlsTransformToLocal(data->urls(), &transformedUrls);
        fmDebug() << "Drag source urls: " << data->urls();
        fmDebug() << "Drag transformed urls: " << transformedUrls;
        DFMMimeData dfmmimeData;
        dfmmimeData.setUrls(data->urls());
        data->setData(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey, dfmmimeData.toByteArray());
        data->setUrls(transformedUrls);
        // treeview set treeview select url
        if (isTreeViewMode() && d->itemsExpandable) {
            auto treeSelectedUrl = selectedTreeViewUrlList();
            transformedUrls.clear();
            UniversalUtils::urlsTransformToLocal(treeSelectedUrl, &transformedUrls);
            QByteArray ba;
            for (const auto &url : transformedUrls) {
                ba.append(QString(url.toString() + "\n").toUtf8());
            }
            data->setData(DFMGLOBAL_NAMESPACE::Mime::kDFMTreeUrlsKey, ba);
        }

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

void FileView::keyboardSearch(const QString &search)
{
    d->fileViewHelper->keyboardSearch(search);
}

void FileView::contextMenuEvent(QContextMenuEvent *event)
{
    // if the left btn is pressed and the rect large enough, means the view state is draggingSelectState.
    // and don't show menu in the draggingSelectState.
    if (d->mouseLeftPressed && (abs(d->mouseMoveRect.width()) > kMinMoveLenght || abs(d->mouseMoveRect.height()) > kMinMoveLenght)) {
        fmDebug() << "Context menu blocked due to drag selection state";
        return;
    }

    if (NetworkUtils::instance()->checkFtpOrSmbBusy(rootUrl())) {
        fmWarning() << "Cannot show context menu: FTP or SMB is busy for URL:" << rootUrl().toString();
        DialogManager::instance()->showUnableToVistDir(rootUrl().path());
        return;
    }

    if (FileViewMenuHelper::disableMenu()) {
        fmDebug() << "Context menu disabled by helper";
        return;
    }

    d->viewMenuHelper->setWaitCursor();
    const QModelIndex &index = indexAt(event->pos());
    if (itemDelegate()->editingIndex().isValid() && itemDelegate()->editingIndex() == index) {
        fmDebug() << "Setting focus due to editing index";
        setFocus(Qt::FocusReason::OtherFocusReason);
    }

    if (d->fileViewHelper->isEmptyArea(event->pos())) {
        fmDebug() << "Showing context menu for empty area";
        BaseItemDelegate *de = itemDelegate();
        if (de)
            de->hideNotEditingIndexWidget();
        clearSelection();

        d->viewMenuHelper->showEmptyAreaMenu();
    } else {
        if (!isSelected(index)) {
            fmDebug() << "Item not selected, clearing selection and selecting clicked item";
            itemDelegate()->hideNotEditingIndexWidget();
            clearSelection();

            if (!index.isValid()) {
                fmDebug() << "Invalid index, showing empty area menu";
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
        if (d->currentViewMode == DFMGLOBAL_NAMESPACE::ViewMode::kIconMode) {
            bool lastRow = indexOfRow(index) == rowCount() - 1;

            if (!lastRow
                && current == index
                && (cursorAction == MoveDown
                    || cursorAction == MovePageDown
                    || cursorAction == MoveNext)) {
                // When there is no element at the next position, QListView will not automatically switch columns to select, you should directly select the last one
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
    case QEvent::MouseButtonPress: {
        if (obj != d->headerWidget)
            break;
        auto e = dynamic_cast<QMouseEvent *>(event);
        if (!e)
            break;

        if (e->button() == Qt::RightButton) {
            d->mouseLeftPressed = false;
            QContextMenuEvent menuEvent(QContextMenuEvent::Mouse, { -1, -1 });
            contextMenuEvent(&menuEvent);
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

    if (obj == d->headerWidget && event->type() == QEvent::Resize) {
        d->headerView->adjustSize();
    }

    return DListView::eventFilter(obj, event);
}

void FileView::paintEvent(QPaintEvent *event)
{
    if (d->animationHelper->isWaitingToPlaying() || d->animationHelper->isAnimationPlaying()) {
        d->animationHelper->paintItems();
        itemDelegate()->hideAllIIndexWidget();
        return;
    }

    if (d->horizontalOffset == 0)
        d->updateHorizontalOffset();
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

    if (itemDelegate())
        itemDelegate()->hideNotEditingIndexWidget();

    DListView::rowsAboutToBeRemoved(parent, start, end);
}

void FileView::showEvent(QShowEvent *event)
{
    DListView::showEvent(event);
    focusOnView();
}

void FileView::initializeModel()
{
    fmDebug() << "Initializing FileView model and selection model";

    FileViewModel *viewModel = new FileViewModel(this);
    setModel(viewModel);

    FileSelectionModel *selectionModel = new FileSelectionModel(viewModel, this);
    setSelectionModel(selectionModel);

    fmDebug() << "Model and selection model initialized successfully";
}

void FileView::initializeDelegate()
{
    fmDebug() << "Initializing FileView delegates";

    d->fileViewHelper = new FileViewHelper(this);

    // Create delegates
    auto iconDelegate = new IconItemDelegate(d->fileViewHelper);
    auto listDelegate = new ListItemDelegate(d->fileViewHelper);

    setDelegate(Global::ViewMode::kIconMode, iconDelegate);
    setDelegate(Global::ViewMode::kListMode, listDelegate);

    // Connect grouping signals for all delegates
    connect(iconDelegate, &BaseItemDelegate::groupExpansionToggled, this, &FileView::onGroupExpansionToggled);
    connect(iconDelegate, &BaseItemDelegate::groupHeaderClicked, this, &FileView::onGroupHeaderClicked);

    connect(listDelegate, &BaseItemDelegate::groupExpansionToggled, this, &FileView::onGroupExpansionToggled);
    connect(listDelegate, &BaseItemDelegate::groupHeaderClicked, this, &FileView::onGroupHeaderClicked);

    d->itemsExpandable = DConfigManager::instance()->value(kViewDConfName, kTreeViewEnable, true).toBool()
            && WorkspaceHelper::instance()->isViewModeSupported(rootUrl().scheme(), DFMGLOBAL_NAMESPACE::ViewMode::kTreeMode);

    fmDebug() << "Delegates initialized with grouping signal connections, items expandable:" << d->itemsExpandable;
}

void FileView::initializeStatusBar()
{
    fmDebug() << "Initializing FileView status bar";

    ViewDefines viewDefines;
    d->statusBar = new FileViewStatusBar(this);
    d->statusBar->resetScalingSlider(viewDefines.iconSizeCount() - 1);

    d->updateStatusBarTimer = new QTimer(this);
    d->updateStatusBarTimer->setInterval(100);
    d->updateStatusBarTimer->setSingleShot(true);

    addFooterWidget(d->statusBar);

    fmDebug() << "Status bar initialized with" << viewDefines.iconSizeCount() << "icon size levels";
}

void FileView::initializeConnect()
{
    connect(d->updateStatusBarTimer, &QTimer::timeout, this, &FileView::updateStatusBar);

    connect(d->statusBar->scalingSlider(), &DSlider::valueChanged, this, &FileView::onScalingValueChanged);

    connect(model(), &FileViewModel::stateChanged, this, &FileView::onModelStateChanged);
    connect(model(), &FileViewModel::selectAndEditFile, this, &FileView::onSelectAndEdit);
    connect(model(), &FileViewModel::dataChanged, this, &FileView::updateOneView);
    connect(model(), &FileViewModel::renameFileProcessStarted, this, &FileView::onRenameProcessStarted);
    connect(model(), &FileViewModel::aboutToSwitchToListView, this, &FileView::onAboutToSwitchListView);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileView::onSelectionChanged);

    connect(this, &DListView::rowCountChanged, this, &FileView::onRowCountChanged, Qt::QueuedConnection);
    connect(this, &DListView::clicked, this, &FileView::onClicked, Qt::UniqueConnection);
    connect(this, &DListView::doubleClicked, this, &FileView::onDoubleClicked);
    connect(this, &DListView::iconSizeChanged, this, &FileView::updateHorizontalOffset, Qt::QueuedConnection);
    connect(this, &FileView::viewStateChanged, this, &FileView::saveViewModeState);

    connect(Application::instance(), &Application::iconSizeLevelChanged, this, &FileView::onIconSizeChanged);
    connect(Application::instance(), &Application::gridDensityLevelChanged, this, &FileView::onItemWidthLevelChanged);
    connect(Application::instance(), &Application::listHeightLevelChanged, this, &FileView::onItemHeightLevelChanged);
    connect(Application::instance(), &Application::showedFileSuffixChanged, this, &FileView::onShowFileSuffixChanged);
    connect(Application::instance(), &Application::previewAttributeChanged, this, &FileView::onWidgetUpdate);
    connect(Application::instance(), &Application::viewModeChanged, this, &FileView::onDefaultViewModeChanged);
    connect(Application::appObtuselySetting(), &Settings::valueChanged, this, &FileView::onAppAttributeChanged);

#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        if (d->currentViewMode == Global::ViewMode::kIconMode)
            this->setSpacing(DSizeModeHelper::element(kCompactIconViewSpacing, kIconViewSpacing));
    });
#endif

    dpfSignalDispatcher->subscribe("dfmplugin_workspace", "signal_View_HeaderViewSectionChanged", this, &FileView::onHeaderViewSectionChanged);

    auto pluginName { DPF_NAMESPACE::LifeCycle::pluginMetaObj("dfmplugin_filepreview") };
    if (pluginName && pluginName->pluginState() == DPF_NAMESPACE::PluginMetaObject::kStarted) {
        dpfSignalDispatcher->subscribe("dfmplugin_filepreview", "signal_ThumbnailDisplay_Changed", this, &FileView::onWidgetUpdate);
    } else {
        connect(
                DPF_NAMESPACE::Listener::instance(), &DPF_NAMESPACE::Listener::pluginStarted, this, [=](const QString &iid, const QString &name) {
                    Q_UNUSED(iid)
                    if (name == "dfmplugin_filepreview")
                        dpfSignalDispatcher->subscribe("dfmplugin_filepreview", "signal_ThumbnailDisplay_Changed", this, &FileView::onWidgetUpdate);
                },
                Qt::DirectConnection);
    }
    connect(&FileInfoHelper::instance(), &FileInfoHelper::smbSeverMayModifyPassword, this, [this](const QUrl &url) {
        if (ProtocolUtils::isSMBFile(rootUrl()) && url.path().startsWith(rootUrl().path())) {
            fmInfo() << rootUrl() << url << "smb server may modify password";
            if (d->isShowSmbMountError)
                return;
            d->isShowSmbMountError = true;
            DialogManager::instance()->showErrorDialog(tr("Mount error"),
                                                       tr("Server login credentials are invalid. Please uninstall and remount"));
            d->isShowSmbMountError = false;
        }
    });
}

void FileView::initializeScrollBarWatcher()
{
    d->scrollBarValueChangedTimer = new QTimer(this);
    d->scrollBarValueChangedTimer->setInterval(50);
    d->scrollBarValueChangedTimer->setSingleShot(true);

    connect(d->scrollBarValueChangedTimer, &QTimer::timeout, this, [this] { this->update(); });

    connect(verticalScrollBar(), &QScrollBar::sliderPressed, this, [this] { d->scrollBarSliderPressed = true; });
    connect(verticalScrollBar(), &QScrollBar::sliderReleased, this, [this] { d->scrollBarSliderPressed = false; });
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int value) {
        if (d->scrollBarSliderPressed)
            d->scrollBarValueChangedTimer->start();

        if (d->headerWidget && d->headerWidget->isVisible()) {
            auto headerLayout = d->headerWidget->layout();
            auto margins = headerLayout->contentsMargins();
            if (value > 0 && margins.bottom() != 0)
                headerLayout->setContentsMargins(0, 0, 0, 0);
            else if (value == 0 && margins.bottom() == 0)
                headerLayout->setContentsMargins(0, 0, 0, 10);
        }
    });
}

void FileView::initializePreSelectTimer()
{
    d->preSelectTimer = new QTimer(this);

    d->preSelectTimer->setInterval(100);
    d->preSelectTimer->setSingleShot(true);
    connect(d->preSelectTimer, &QTimer::timeout, this, [=] {
        if (selectFiles(d->preSelectionUrls))
            d->preSelectionUrls.clear();
    });
}

void FileView::updateStatusBar()
{
    if (model()->currentState() != ModelState::kIdle) {
        fmDebug() << "Status bar update skipped: model is busy for URL:" << rootUrl().toString();
        return;
    }

    int count = selectedIndexCount();

    if (count == 0) {
        d->statusBar->itemCounted(model()->rowCount(rootIndex()));
        fmDebug() << "Status bar updated: no selection, total items:" << model()->rowCount(rootIndex()) << "for URL:" << rootUrl().toString();
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
    fmDebug() << "Status bar updated: selected files:" << selectFiles << "folders:" << selectFolders << "size:" << filesizes << "for URL:" << rootUrl().toString();
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
        fmDebug() << "Loading indicator shown for URL:" << rootUrl().toString();
    }

    if (state == ModelState::kIdle) {
        d->statusBar->hideLoadingIncator();
        updateStatusBar();
        fmDebug() << "Loading indicator hidden for URL:" << rootUrl().toString();
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

    d->preSelectTimer->start();
}

void FileView::updateListHeaderView()
{
    if (!d->headerView) {
        return;
    }

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
                if (curRole == kItemFileLastModifiedRole || curRole == kItemFileCreatedRole) {
                    d->headerView->resizeSection(model()->getColumnByRole(curRole), kDefaultItemFileTimeWidth);
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

void FileView::setListViewMode()
{
    setMovement(QListView::Static);
    setUniformItemSizes(false);
    setResizeMode(QListView::Fixed);
    setOrientation(QListView::TopToBottom, false);
    setSpacing(kListViewSpacing);

    d->initListModeView();
    if (d->allowedAdjustColumnSize) {
        horizontalScrollBar()->parentWidget()->installEventFilter(this);

        d->cachedViewWidth = this->width();
        d->adjustFileNameColumn = true;
        updateListHeaderView();
    }

    verticalScrollBar()->setFixedHeight(rect().height() - d->statusBar->height() - d->headerView->height());
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
        d->preSelectionUrls.clear();
        d->preSelectionUrls << defaultSelectUrl;
        urlQuery.removeQueryItem("selectUrl");
        fileUrl.setQuery(urlQuery);
    } else {
        // todo: liuzhangjian
        // checkGvfsMountfileBusy
        QList<QUrl> ancestors;
        if (const FileInfoPointer &currentFileInfo = InfoFactory::create<FileInfo>(rootUrl())) {
            if (UrlRoute::isAncestorsUrl(rootUrl(), fileUrl, &ancestors)) {
                d->preSelectionUrls.clear();
                d->preSelectionUrls << (ancestors.count() > 1 ? ancestors.at(ancestors.count() - 2) : rootUrl());
            }
        }
    }

    return fileUrl;
}

void FileView::loadViewState(const QUrl &url)
{
    d->loadViewMode(url);

    QVariant defaultIconSize = Application::instance()->appAttribute(Application::kIconSizeLevel).toInt();
    QVariant defaultGridDensity = Application::instance()->appAttribute(Application::kGridDensityLevel).toInt();

    d->currentIconSizeLevel = d->fileViewStateValue(url, "iconSizeLevel", defaultIconSize).toInt();
    d->currentGridDensityLevel = d->fileViewStateValue(url, "gridDensityLevel", defaultGridDensity).toInt();

    int customListHeightLevel = d->fileViewHelper->customDefaultListItemHeightLevel();
    QVariant defaultListHeight = customListHeightLevel >= 0 ? customListHeightLevel : Application::instance()->appAttribute(Application::kListHeightLevel).toInt();
    if (d->fileViewHelper->canChangeListItemHeight()) {
        d->currentListHeightLevel = d->fileViewStateValue(url, "listHeightLevel", defaultListHeight).toInt();
    } else {
        d->currentListHeightLevel = customListHeightLevel;
    }
}

void FileView::onModelStateChanged()
{
    fmDebug() << "Model state changed to:" << static_cast<int>(model()->currentState()) << "for URL:" << rootUrl().toString();

    updateContentLabel();
    updateLoadingIndicator();
    updateSelectedUrl();

    if (model()->currentState() == ModelState::kBusy) {
        fmDebug() << "Model is busy - disabling header interactions for URL:" << rootUrl().toString();
        if (d->headerView) {
            d->headerView->setSortIndicatorShown(false);
            d->headerView->setSectionsClickable(false);
        }
        d->animationHelper->reset();
    } else {
        fmDebug() << "Model is idle - enabling header interactions for URL:" << rootUrl().toString();
        if (d->headerView) {
            d->headerView->setSortIndicatorShown(true);
            d->headerView->setSectionsClickable(true);
        }
        d->animationHelper->initAnimationHelper();
    }

    if (d->headerView)
        d->headerView->setAttribute(Qt::WA_TransparentForMouseEvents, model()->currentState() == ModelState::kBusy);

    notifyStateChanged();
}

void FileView::openIndexByClicked(const ClickedAction action, const QModelIndex &index)
{
    ClickedAction configAction = static_cast<ClickedAction>(Application::instance()->appAttribute(Application::kOpenFileMode).toInt());
    if (action == configAction) {
        Qt::ItemFlags flags = model()->flags(index);
        if (!flags.testFlag(Qt::ItemIsEnabled)) {
            fmDebug() << "Item not enabled, skipping open for URL:" << rootUrl().toString();
            return;
        }

        if (!WindowUtils::keyCtrlIsPressed() && !WindowUtils::keyShiftIsPressed())
            openIndex(index);
    }
}

void FileView::openIndex(const QModelIndex &index)
{
    const FileInfoPointer &info = model()->fileInfo(index);

    if (!info) {
        fmWarning() << "Cannot open index: file info is null";
        return;
    }

    QUrl fileUrl = info->urlOf(UrlInfoType::kUrl);
    fmInfo() << "Opening file:" << fileUrl;
    if (NetworkUtils::instance()->checkFtpOrSmbBusy(fileUrl)) {
        fmWarning() << "Cannot open file: FTP or SMB is busy for URL:" << fileUrl;
        DialogManager::instance()->showUnableToVistDir(fileUrl.path());
        return;
    }

    FileOperatorHelperIns->openFiles(this, { info->urlOf(UrlInfoType::kUrl) });
}

void FileView::setFileViewStateValue(const QUrl &url, const QString &key, const QVariant &value)
{
    WorkspaceHelper::instance()->setFileViewStateValue(url, key, value);
}

void FileView::saveViewModeState()
{
    const QUrl &url = rootUrl();

    setFileViewStateValue(url, "iconSizeLevel", d->statusBar->scalingSlider()->value());
    setFileViewStateValue(url, "viewMode", static_cast<int>(d->currentViewMode));
}

void FileView::focusOnView()
{
    if (WorkspaceHelper::instance()->isFocusFileViewDisabled(d->url.scheme()))
        return;

    if (isVisible())
        setFocus();
}

// Grouping-related slot implementations
void FileView::onGroupExpansionToggled(const QString &groupKey)
{
    fmDebug() << "Group expansion toggled for key:" << groupKey << "for URL:" << rootUrl().toString();

    if (groupKey.isEmpty()) {
        fmWarning() << "Cannot toggle expansion: empty group key";
        return;
    }

    // Forward to model for handling
    if (model()) {
        model()->toggleGroupExpansion(groupKey);
    }
}

void FileView::onGroupHeaderClicked(const QString &groupKey)
{
    fmDebug() << "Group header clicked for key:" << groupKey << "for URL:" << rootUrl().toString();

    if (groupKey.isEmpty()) {
        fmWarning() << "Cannot handle header click: empty group key";
        return;
    }

    // Use SelectHelper to handle group selection
    if (d->selectHelper) {
        // Get keyboard modifiers from current application state
        Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();

        // Create a dummy index for the group header to pass to SelectHelper
        // We need to find the actual group header index
        QAbstractItemModel *itemModel = model();
        if (itemModel) {
            int rowCount = itemModel->rowCount(rootIndex());
            for (int row = 0; row < rowCount; ++row) {
                QModelIndex index = itemModel->index(row, 0, rootIndex());
                if (index.isValid()) {
                    QUrl url = index.data(Global::kItemUrlRole).toUrl();
                    if (url.scheme() == "group-header") {
                        QString currentGroupKey = url.path();
                        if (currentGroupKey.startsWith("/")) {
                            currentGroupKey = currentGroupKey.mid(1);
                        }
                        if (currentGroupKey == groupKey) {
                            d->selectHelper->handleGroupHeaderClick(index, modifiers);
                            break;
                        }
                    }
                }
            }
        }
    }

    fmDebug() << "Group header click processed for group:" << groupKey;
}
