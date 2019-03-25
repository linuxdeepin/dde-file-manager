/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "dfileview.h"
#include "fileitem.h"
#include "dfilemenumanager.h"
#include "dfilemenu.h"
#include "windowmanager.h"
#include "dstatusbar.h"
#include "fileviewhelper.h"
#include "dfilemanagerwindow.h"
#include "dtoolbar.h"
#include "dabstractfilewatcher.h"
#include "dfmheaderview.h"
#include "dfmeventdispatcher.h"
#include "themeconfig.h"
#include "dfmsettings.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "interfaces/dfmglobal.h"
#include "interfaces/diconitemdelegate.h"
#include "interfaces/dlistitemdelegate.h"
#include "dfmapplication.h"
#include "interfaces/dfmcrumbbar.h"

#include "controllers/appcontroller.h"
#include "dfileservices.h"
#include "controllers/pathmanager.h"

#include "models/dfileselectionmodel.h"
#include "dfilesystemmodel.h"

#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "fileoperations/filejob.h"
#include "deviceinfo/udisklistener.h"

#include "singleton.h"
#include "interfaces/dfilemenumanager.h"

#include <dthememanager.h>
#include <danchors.h>

#include <QUrlQuery>
#include <QActionGroup>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMimeData>
#include <QScrollBar>
#include <QScroller>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>

DWIDGET_USE_NAMESPACE

#define ICON_VIEW_SPACING 5
#define LIST_VIEW_SPACING 1
#define LIST_VIEW_MINIMUM_WIDTH 80

#define DEFAULT_HEADER_SECTION_WIDTH 140

class DFileViewPrivate
{
public:
    DFileViewPrivate(DFileView *qq)
        : q_ptr(qq) {}

    int iconModeColumnCount(int itemWidth = 0) const;
    QVariant fileViewStateValue(const DUrl &url, const QString &key, const QVariant &defalutValue);
    void setFileViewStateValue(const DUrl &url, const QString &key, const QVariant &value);
    void updateHorizontalScrollBarPosition();
    void pureResizeEvent(QResizeEvent *event);
    void doFileNameColResize();
    void toggleHeaderViewSnap(bool on);
    void _q_onSectionHandleDoubleClicked(int logicalIndex);

    DFileView *q_ptr;

    DFileMenuManager* fileMenuManager;
    DFMHeaderView *headerView = nullptr;
    QWidget *headerViewHolder = nullptr;
    DStatusBar* statusBar = nullptr;

    QActionGroup* displayAsActionGroup;
    QActionGroup* sortByActionGroup;
    QActionGroup* openWithActionGroup;

    QList<int> columnRoles;

    DFileView::ViewMode defaultViewMode = DFileView::IconMode;
    DFileView::ViewMode currentViewMode = DFileView::IconMode;

    int horizontalOffset = 0;

    /// move cursor later selecte index when pressed key shift
    QModelIndex lastCursorIndex;

    /// list mode column visible
    QMap<QString, bool> columnForRoleHiddenMap;

    int firstVisibleColumn = -1;
    int lastVisibleColumn = -1;

    DUrlList preSelectionUrls;

    DAnchors<QLabel> contentLabel = Q_NULLPTR;

    QModelIndex mouseLastPressedIndex;

    /// drag drop
    QModelIndex dragMoveHoverIndex;

    /// Saved before sorting
    DUrlList oldSelectedUrls;
    DUrl oldCurrentUrl;

    DFileView::RandeIndex visibleIndexRande;

    /// menu actions filter
    QSet<MenuAction> menuWhitelist;
    QSet<MenuAction> menuBlacklist;

    QSet<DFileView::SelectionMode> enabledSelectionModes;

    FileViewHelper *fileViewHelper;

    QTimer* updateStatusBarTimer;

    QScrollBar* verticalScrollBar = NULL;

    QActionGroup *toolbarActionGroup;

    bool allowedAdjustColumnSize = true;
    bool adjustFileNameCol = false; // mac finder style half-auto col size adjustment flag.
    int cachedViewWidth = -1;

    // 用于实现触屏滚动视图和框选文件不冲突，手指在屏幕上按下短时间内就开始移动
    // 会被认为触发滚动视图，否则为触发文件选择（时间默认为300毫秒）
    QPointer<QTimer> updateEnableSelectionByMouseTimer;
    // 记录触摸按下事件，在mouse move事件中使用，用于判断手指移动的距离，当大于
    // QPlatformTheme::TouchDoubleTapDistance 的值时认为触发触屏滚动
    QPoint lastTouchBeginPos;
    int touchTapDistance = -1;

    Q_DECLARE_PUBLIC(DFileView)
};

DFileView::DFileView(QWidget *parent)
    : DListView(parent)
    , d_ptr(new DFileViewPrivate(this))
{
    if (DFMGlobal::isRootUser()) {
        D_THEME_INIT_WIDGET(DFileViewRoot)
    } else {
        D_THEME_INIT_WIDGET(DFileView);
    }

    D_D(DFileView);

    d_ptr->enabledSelectionModes << NoSelection << SingleSelection
                                 << MultiSelection << ExtendedSelection
                                 << ContiguousSelection;

    d_ptr->defaultViewMode = static_cast<ViewMode>(DFMApplication::instance()->appAttribute(DFMApplication::AA_ViewMode).toInt());

#if QT_VERSION < QT_VERSION_CHECK(5,9,0)
    d_ptr->touchTapDistance = 15;
#else
    d_ptr->touchTapDistance = QGuiApplicationPrivate::platformTheme()->themeHint(QPlatformTheme::TouchDoubleTapDistance).toInt();
#endif

    initUI();
    initModel();
    initDelegate();
    initConnects();

    setIconSizeBySizeIndex(DFMApplication::instance()->appAttribute(DFMApplication::AA_IconSizeLevel).toInt());
    d->updateStatusBarTimer = new QTimer;
    d->updateStatusBarTimer->setInterval(100);
    d->updateStatusBarTimer->setSingleShot(true);
    connect(d->updateStatusBarTimer, &QTimer::timeout, this, &DFileView::updateStatusBar);
}

DFileView::~DFileView()
{
    disconnect(this, &DFileView::rowCountChanged, this, &DFileView::onRowCountChanged);
    disconnect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &DFileView::delayUpdateStatusBar);
}

DFileSystemModel *DFileView::model() const
{
    return qobject_cast<DFileSystemModel*>(DListView::model());
}

DStyledItemDelegate *DFileView::itemDelegate() const
{
    return qobject_cast<DStyledItemDelegate*>(DListView::itemDelegate());
}

void DFileView::setItemDelegate(DStyledItemDelegate *delegate)
{
    D_D(DFileView);

    QAbstractItemDelegate *dg = DListView::itemDelegate();

    if (dg)
        dg->deleteLater();

    DListView::setItemDelegate(delegate);

    connect(d->statusBar->scalingSlider(), &DSlider::valueChanged, delegate, &DStyledItemDelegate::setIconSizeByIconSizeLevel);

    if (isIconViewMode()) {
        d->statusBar->scalingSlider()->setMinimum(delegate->minimumIconSizeLevel());
        d->statusBar->scalingSlider()->setMaximum(delegate->maximumIconSizeLevel());
    }
}

DStatusBar *DFileView::statusBar() const
{
    D_DC(DFileView);

    return d->statusBar;
}

FileViewHelper *DFileView::fileViewHelper() const
{
    D_DC(DFileView);

    return d->fileViewHelper;
}

DUrl DFileView::rootUrl() const
{
    return model()->rootUrl();
}

DFMBaseView::ViewState DFileView::viewState() const
{
    return model()->state() == DFileSystemModel::Busy ? ViewBusy : ViewIdle;
}

QList<DUrl> DFileView::selectedUrls() const
{
    QModelIndex rootIndex = this->rootIndex();
    DUrlList list;

    for(const QModelIndex &index : selectedIndexes()) {
        if (index.parent() != rootIndex)
            continue;

        list << model()->getUrlByIndex(index);
    }

    return list;
}

bool DFileView::isIconViewMode() const
{
    D_DC(DFileView);

    return d->currentViewMode == IconMode;
}

int DFileView::columnWidth(int column) const
{
    D_DC(DFileView);

    return d->headerView ? d->headerView->sectionSize(column) : 100;
}

void DFileView::setColumnWidth(int column, int width)
{
    D_D(DFileView);

    if(!d->headerView)
        return;

    d->headerView->resizeSection(column, width);
}

int DFileView::columnCount() const
{
    D_DC(DFileView);

    return d->headerView ? d->headerView->count() : 1;
}

int DFileView::rowCount() const
{
    int count = this->count();
    int itemCountForRow = this->itemCountForRow();

    return count / itemCountForRow + int(count % itemCountForRow > 0);
}

int DFileView::itemCountForRow() const
{
    Q_D(const DFileView);

    if (!isIconViewMode())
        return 1;

    return d->iconModeColumnCount();
}

QList<int> DFileView::columnRoleList() const
{
    D_DC(DFileView);

    return d->columnRoles;
}

quint64 DFileView::windowId() const
{
    return WindowManager::getWindowId(this);
}

void DFileView::setIconSize(const QSize &size)
{
    DListView::setIconSize(size);

    updateHorizontalOffset();
    updateGeometries();
}

DFileView::ViewMode DFileView::getDefaultViewMode() const
{
    D_DC(DFileView);

    return d->defaultViewMode;
}

DFileView::ViewMode DFileView::viewMode() const
{
    D_DC(DFileView);

    return d->currentViewMode;
}

bool DFileView::testViewMode(ViewModes modes, DFileView::ViewMode mode) const
{
    return (modes | mode) == modes;
}

int DFileView::horizontalOffset() const
{
    D_DC(DFileView);

    return d->horizontalOffset;
}

bool DFileView::isSelected(const QModelIndex &index) const
{
#ifndef CLASSICAL_SECTION
    return static_cast<DFileSelectionModel*>(selectionModel())->isSelected(index);
#else
    return selectionModel()->isSelected(index);
#endif
}

int DFileView::selectedIndexCount() const
{
#ifndef CLASSICAL_SECTION
    return static_cast<const DFileSelectionModel*>(selectionModel())->selectedCount();
#else
    return selectionModel()->selectedIndexes().count();
#endif
}

QModelIndexList DFileView::selectedIndexes() const
{
#ifndef CLASSICAL_SECTION
    return static_cast<DFileSelectionModel*>(selectionModel())->selectedIndexes();
#else
    return selectionModel()->selectedIndexes();
#endif
}

QModelIndex DFileView::indexAt(const QPoint &point) const
{
    Q_D(const DFileView);

    if (isIconViewMode()) {
        for (QModelIndex &index : itemDelegate()->hasWidgetIndexs()) {
            if (index == itemDelegate()->editingIndex())
                continue;

            QWidget *widget = indexWidget(index);

            if (widget && widget->isVisible() && widget->geometry().contains(point)) {
                return index;
            }
        }
    }

    QPoint pos = QPoint(point.x() + horizontalOffset(), point.y() + verticalOffset());
    QSize item_size = itemSizeHint();

    if (pos.y() % (item_size.height() + spacing() * 2) < spacing())
        return QModelIndex();

    int index = -1;

    if (item_size.width() == -1) {
        int item_height = item_size.height() + LIST_VIEW_SPACING * 2;

        index = pos.y() / item_height;
    } else {
        int item_width = item_size.width() + ICON_VIEW_SPACING * 2;

        if (pos.x() % item_width <= ICON_VIEW_SPACING)
            return QModelIndex();

        int row_index = pos.y() / (item_size.height() + ICON_VIEW_SPACING * 2);
        int column_count = d->iconModeColumnCount(item_width);
        int column_index = pos.x() / item_width;

        if (column_index >= column_count)
            return QModelIndex();

        index = row_index * column_count + column_index;

        const QModelIndex &tmp_index = rootIndex().child(index, 0);
        QStyleOptionViewItem option = viewOptions();

        option.rect = QRect(QPoint(column_index * item_width + ICON_VIEW_SPACING,
                                    row_index * (item_size.height()  + ICON_VIEW_SPACING * 2) + ICON_VIEW_SPACING),
                            item_size);

        const QList<QRect> &list = itemDelegate()->paintGeomertys(option, tmp_index);

        for (const QRect &rect : list)
            if (rect.contains(pos))
                return tmp_index;

        return QModelIndex();
    }

    return rootIndex().child(index, 0);
}

QRect DFileView::visualRect(const QModelIndex &index) const
{
    Q_D(const DFileView);

    QRect rect;

    if (index.column() != 0)
        return rect;

    QSize item_size = itemSizeHint();

    if (item_size.width() == -1) {
        rect.setLeft(LIST_VIEW_SPACING - horizontalScrollBar()->value());
        rect.setRight(viewport()->width() - LIST_VIEW_SPACING - 1);
        rect.setTop(index.row() * (item_size.height() + LIST_VIEW_SPACING * 2) + LIST_VIEW_SPACING);
        rect.setHeight(item_size.height());

        if (d->allowedAdjustColumnSize) {
            rect.setWidth(d->headerView->length());
        }
    } else {
        int item_width = item_size.width() + ICON_VIEW_SPACING * 2;
        int column_count = d->iconModeColumnCount(item_width);

        if (column_count == 0)
            return rect;

        int column_index = index.row() % column_count;
        int row_index = index.row() / column_count;

        rect.setTop(row_index * (item_size.height()  + ICON_VIEW_SPACING * 2) + ICON_VIEW_SPACING);
        rect.setLeft(column_index * item_width + ICON_VIEW_SPACING);
        rect.setSize(item_size);
    }

    rect.moveLeft(rect.left() - horizontalOffset());
    rect.moveTop(rect.top() - verticalOffset());

    return rect;
}

DFileView::RandeIndexList DFileView::visibleIndexes(QRect rect) const
{
    Q_D(const DFileView);

    RandeIndexList list;

    QSize item_size = itemSizeHint();
    QSize icon_size = iconSize();

    int count = this->count();
    int spacing  = this->spacing();
    int item_width = item_size.width() + spacing *  2;
    int item_height = item_size.height() + spacing * 2;

    if (item_size.width() == -1) {
        list << RandeIndex(qMax((rect.top() + spacing) / item_height, 0),
                           qMin((rect.bottom() - spacing) / item_height, count - 1));
    } else {
        rect -= QMargins(spacing, spacing, spacing, spacing);

        int column_count = d->iconModeColumnCount(item_width);

        if (column_count <= 0)
            return list;

        int begin_row_index = rect.top() / item_height;
        int end_row_index = rect.bottom() / item_height;
        int begin_column_index = rect.left() / item_width;
        int end_column_index = rect.right() / item_width;

        if (rect.top() % item_height > icon_size.height())
            ++begin_row_index;

        int icon_margin = (item_width - icon_size.width()) / 2;

        if (rect.left() % item_width > item_width - icon_margin)
            ++begin_column_index;

        if (rect.right() % item_width < icon_margin)
            --end_column_index;

        begin_row_index = qMax(begin_row_index, 0);
        begin_column_index = qMax(begin_column_index, 0);
        end_row_index = qMin(end_row_index, count / column_count);
        end_column_index = qMin(end_column_index, column_count - 1);

        if (begin_row_index > end_row_index || begin_column_index > end_column_index)
            return list;

        int begin_index = begin_row_index * column_count;

        if (end_column_index - begin_column_index + 1 == column_count) {
            list << RandeIndex(qMax(begin_index, 0), qMin((end_row_index + 1) * column_count - 1, count - 1));

            return list;
        }

        for (int i = begin_row_index; i <= end_row_index; ++i) {
            if (begin_index + begin_column_index >= count)
                break;

            list << RandeIndex(qMax(begin_index + begin_column_index, 0),
                               qMin(begin_index + end_column_index, count - 1));

            begin_index += column_count;
        }
    }

    return list;
}

QSize DFileView::itemSizeHint() const
{
    D_DC(DFileView);

    return itemDelegate()->sizeHint(viewOptions(), rootIndex());
}

bool DFileView::isDropTarget(const QModelIndex &index) const
{
    D_DC(DFileView);

    return d->dragMoveHoverIndex == index;
}

bool DFileView::cd(const DUrl &url)
{
    DFileManagerWindow* w = qobject_cast<DFileManagerWindow*>(WindowManager::getWindowById(windowId()));

    return w && w->cd(url);
}

bool DFileView::cdUp()
{
    const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(rootIndex());

    const DUrl &oldCurrentUrl = rootUrl();
    const DUrl& parentUrl = fileInfo ? fileInfo->parentUrl() : DUrl::parentUrl(oldCurrentUrl);

    return parentUrl.isValid() && cd(parentUrl);
}

bool DFileView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    DUrl fileUrl = model()->getUrlByIndex(index);

    if (fileUrl.isEmpty() || selectedIndexCount() > 1 || (trigger == SelectedClicked && DFMGlobal::keyShiftIsPressed()))
        return false;
#ifdef SW_LABEL
    bool isCheckRenameAction = false;
#endif
    if (trigger == SelectedClicked) {
        QStyleOptionViewItem option = viewOptions();

        option.rect = visualRect(index);

        const QRect &file_name_rect = itemDelegate()->fileNameRect(option, index);

        if (!file_name_rect.contains(static_cast<QMouseEvent*>(event)->pos())){
            return false;
        }else{
#ifdef SW_LABEL
            isCheckRenameAction = true;
#endif
        }
    }
#ifdef SW_LABEL
    if (trigger == EditKeyPressed){
        isCheckRenameAction = true;
    }

    if (isCheckRenameAction){
        bool isCanRename = checkRenamePrivilege_sw(fileUrl);
        if (!isCanRename)
            return false;
    }
#endif
    return DListView::edit(index, trigger, event);
}

void DFileView::select(const QList<DUrl> &list)
{
    QModelIndex firstIndex;
    QModelIndex lastIndex;
    const QModelIndex &root = rootIndex();
    clearSelection();

    for (const DUrl &url : list) {
        const QModelIndex &index = model()->index(url);

        if (index == root || !index.isValid()){
            continue;
        }

        selectionModel()->select(index, QItemSelectionModel::Select);

        if (!firstIndex.isValid())
            firstIndex = index;

        lastIndex = index;
    }

    if (lastIndex.isValid())
        selectionModel()->setCurrentIndex(lastIndex, QItemSelectionModel::Select);

    if (firstIndex.isValid())
        scrollTo(firstIndex, PositionAtTop);
}

void DFileView::setDefaultViewMode(DFileView::ViewMode mode)
{
    Q_D(DFileView);
    d->defaultViewMode = mode;

    const DUrl &root_url = rootUrl();

    if (!root_url.isValid())
        return;

    if (d->fileViewStateValue(root_url, "viewMode", QVariant()).isValid())
        return;

    DAbstractFileInfoPointer info = model()->fileInfo(rootIndex());

    if (!info)
        return;

    ViewModes modes = (ViewModes)info->supportViewMode();

    //view mode support handler
    if (modes & mode) {
        switchViewMode(mode);
    }
}

void DFileView::setViewMode(DFileView::ViewMode mode)
{
    D_D(DFileView);

    switchViewMode(mode);
    emit viewStateChanged();
}

void DFileView::sortByRole(int role, Qt::SortOrder order)
{
    D_D(DFileView);

    model()->setSortRole(role, order);

    d->oldSelectedUrls = this->selectedUrls();

    if (!d->oldSelectedUrls.isEmpty())
        d->oldCurrentUrl = model()->getUrlByIndex(currentIndex());

    clearSelection();
    model()->sort();

    if (d->headerView) {
        QSignalBlocker blocker(d->headerView);
        Q_UNUSED(blocker)
        d->headerView->setSortIndicator(model()->sortColumn(), model()->sortOrder());
    }
}

QStringList DFileView::nameFilters() const
{
    return model()->nameFilters();
}

void DFileView::setNameFilters(const QStringList &nameFilters)
{
    model()->setNameFilters(nameFilters);
}

QDir::Filters DFileView::filters() const
{
    return model()->filters();
}

void DFileView::setEnabledSelectionModes(const QSet<QAbstractItemView::SelectionMode> &list)
{
    Q_D(DFileView);

    d->enabledSelectionModes = list;

    if (!list.contains(selectionMode())) {
        const DAbstractFileInfoPointer &info = model()->fileInfo(rootIndex());

        if (!info)
            return;

        const QList<DAbstractFileInfo::SelectionMode> &supportSelectionModes = info->supportSelectionModes();

        for (DAbstractFileInfo::SelectionMode mode : supportSelectionModes) {
            if (list.contains((SelectionMode)mode)) {
                setSelectionMode((SelectionMode)mode);
                break;
            }
        }
    }
}

QSet<QAbstractItemView::SelectionMode> DFileView::enabledSelectionModes() const
{
    Q_D(const DFileView);

    return d->enabledSelectionModes;
}

QWidget *DFileView::widget() const
{
    return const_cast<DFileView*>(this);
}

QList<QAction *> DFileView::toolBarActionList() const
{
    Q_D(const DFileView);

    return d->toolbarActionGroup->actions();
}

void DFileView::setFilters(QDir::Filters filters)
{
    model()->setFilters(filters);
}

void DFileView::setAdvanceSearchFilter(const QMap<int, QVariant> &formData, bool turnOn, bool avoidUpdateView)
{
    Q_UNUSED(avoidUpdateView);
    model()->setAdvanceSearchFilter(formData, turnOn);
}

void DFileView::dislpayAsActionTriggered(QAction *action)
{
    QAction* dAction = static_cast<QAction*>(action);
    dAction->setChecked(true);
    MenuAction type = (MenuAction)dAction->data().toInt();

    switch(type){
        case MenuAction::IconView:
            setViewModeToIcon();
            break;
        case MenuAction::ListView:
            setViewModeToList();
            break;
    case MenuAction::ExtendView:
            break;
        default:
            break;
    }
}

void DFileView::sortByActionTriggered(QAction *action)
{
    Q_D(DFileView);

    QAction* dAction = static_cast<QAction*>(action);
    const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(rootIndex());

    if (!fileInfo)
        return;

    int action_index = d->sortByActionGroup->actions().indexOf(dAction);

    if (action_index < 0)
        return;

    int sort_role = fileInfo->sortSubMenuActionUserColumnRoles().at(action_index);

    Qt::SortOrder order = (model()->sortRole() == sort_role && model()->sortOrder() == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;

    sortByRole(sort_role, order);

    const DUrl &root_url = rootUrl();

    d->setFileViewStateValue(root_url, "sortRole", model()->sortRole());
    d->setFileViewStateValue(root_url, "sortOrder", (int)order);
}

void DFileView::openWithActionTriggered(QAction *action)
{
    QAction* dAction = static_cast<QAction*>(action);
    QString app = dAction->property("app").toString();
    DUrl fileUrl(dAction->property("url").toUrl());
    fileService->openFileByApp(this, app, fileUrl);
}

void DFileView::onRowCountChanged()
{
#ifndef CLASSICAL_SECTION
    static_cast<DFileSelectionModel*>(selectionModel())->m_selectedList.clear();
#endif

    delayUpdateStatusBar();
    updateContentLabel();
    updateModelActiveIndex();
}

void DFileView::wheelEvent(QWheelEvent *event)
{
    if (isIconViewMode()) {
        if (DFMGlobal::keyCtrlIsPressed()) {
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

void DFileView::keyPressEvent(QKeyEvent *event)
{
    D_D(DFileView);

    const DUrlList& urls = selectedUrls();

    switch (event->modifiers()) {
    case Qt::NoModifier:
        switch (event->key()) {
        case Qt::Key_Space:
            emit fileSignalManager->requestShowFilePreviewDialog(selectedUrls(), model()->sortedUrls());
            return;
        default:
            break;
        }
    case Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (!itemDelegate()->editingIndex().isValid()) {
                appController->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));

                return;
            }

            break;
        case Qt::Key_Backspace:{
            // blmark: revert commit vbfdf8e575447249ba284402bfac8a512bae2d10e
            cdUp();
        }
            return;
        case Qt::Key_Delete:{
            QString rootPath = rootUrl().toLocalFile();
            if (FileUtils::isGvfsMountFile(rootPath) || deviceListener->isInRemovableDeviceFolder(rootPath)){
                appController->actionCompleteDeletion(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));
            } else {
                appController->actionDelete(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));
            }
            break;
        }
        case Qt::Key_End:
            if (urls.isEmpty()) {
                setCurrentIndex(model()->index(count() - 1, 0));
                return;
            }
        default: break;
        }

        break;
    case Qt::ControlModifier:
        switch (event->key()) {
        case Qt::Key_N: {
            DUrlList list;

            for (const DUrl &url : urls) {
                const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url);

                if (info && info->canFetch())
                    list << url;
            }

            appController->actionNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(this, list.isEmpty() ? DUrlList() << DUrl() : list));
            return;
        }
        case Qt::Key_H:
            d->preSelectionUrls = urls;

            itemDelegate()->hideAllIIndexWidget();
            clearSelection();
            model()->toggleHiddenFiles(rootUrl());

            return;
        case Qt::Key_I:
            if (rootUrl().isNetWorkFile())
                return;

            appController->actionProperty(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));

            return;
        case Qt::Key_Up:
            cdUp();

            return;
        case Qt::Key_Down:
            appController->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));

            return;
        case Qt::Key_T:{
            //do not handle key press event of autoRepeat type
            if (event->isAutoRepeat())
                return;

            DUrl url;

            if (selectedIndexCount() == 1 && model()->fileInfo(selectedIndexes().first())->canFetch()) {
                url = model()->fileInfo(selectedIndexes().first())->fileUrl();
            } else{
                url = DFMApplication::instance()->appUrlAttribute(DFMApplication::AA_UrlOfNewTab);

                if (!url.isValid())
                    url = rootUrl();
            }
            DFMEventDispatcher::instance()->processEvent<DFMOpenNewTabEvent>(this, url);
            return;
        }
        default: break;
        }

        break;
    case Qt::ShiftModifier:
        if (event->key() == Qt::Key_Delete) {
            if (urls.isEmpty())
                return;

            fileService->deleteFiles(this, urls);

            return;
        } else if (event->key() == Qt::Key_T) {
            appController->actionOpenInTerminal(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));

            return;
        }

        break;
    case Qt::ControlModifier | Qt::ShiftModifier:
        if (event->key() == Qt::Key_N) {
            if (itemDelegate()->editingIndex().isValid())
                return;

            clearSelection();
            appController->actionNewFolder(dMakeEventPointer<DFMUrlBaseEvent>(this, rootUrl()));

            return;
        }
        break;

    case Qt::AltModifier:
    case Qt::AltModifier | Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Up:
            cdUp();

            return;
        case Qt::Key_Down:
            appController->actionOpen(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls));

            return;
        case Qt::Key_Home:
            cd(DUrl::fromLocalFile(QDir::homePath()));

            return;
        }
        break;

    default: break;
    }

    DListView::keyPressEvent(event);
}

void DFileView::showEvent(QShowEvent *event)
{
    DListView::showEvent(event);

    Q_D(DFileView);

    /// set menu actions filter
    DFileMenuManager::setActionWhitelist(d->menuWhitelist);
    DFileMenuManager::setActionBlacklist(d->menuBlacklist);

    setFocus();
}

void DFileView::mousePressEvent(QMouseEvent *event)
{
    D_D(DFileView);

    switch (event->button()) {
    case Qt::BackButton: {
        DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMBackEvent>(this), qobject_cast<DFileManagerWindow*>(window()));
        break;
    }
    case Qt::ForwardButton: {
        DFMEventDispatcher::instance()->processEvent(dMakeEventPointer<DFMForwardEvent>(this), qobject_cast<DFileManagerWindow*>(window()));
        break;
    }
    case Qt::LeftButton: {
        // 当事件source为MouseEventSynthesizedByQt，认为此事件为TouchBegin转换而来
        if (event->source() == Qt::MouseEventSynthesizedByQt) {
            d->lastTouchBeginPos = event->pos();

            // 清空触屏滚动操作，因为在鼠标按下时还不知道即将进行的是触屏滚动还是文件框选
            if (QScroller::hasScroller(this)) {
                // 不可使用 ungrab，会导致应用崩溃，或许是Qt的bug
                QScroller::scroller(this)->deleteLater();
            }

            if (d->updateEnableSelectionByMouseTimer) {
                d->updateEnableSelectionByMouseTimer->stop();
            } else {
                d->updateEnableSelectionByMouseTimer = new QTimer(this);
                d->updateEnableSelectionByMouseTimer->setSingleShot(true);

                static QObject *theme_settings = reinterpret_cast<QObject*>(qvariant_cast<quintptr>(qApp->property("_d_theme_settings_object")));
                QVariant touchFlickBeginMoveDelay;

                if (theme_settings) {
                    touchFlickBeginMoveDelay = theme_settings->property("touchFlickBeginMoveDelay");
                }

                d->updateEnableSelectionByMouseTimer->setInterval(touchFlickBeginMoveDelay.isValid() ? touchFlickBeginMoveDelay.toInt() : 300);

                connect(d->updateEnableSelectionByMouseTimer, &QTimer::timeout, d->updateEnableSelectionByMouseTimer, &QTimer::deleteLater);
            }

            d->updateEnableSelectionByMouseTimer->start();
        }

        bool isEmptyArea = d->fileViewHelper->isEmptyArea(event->pos());

        if (dragDropMode() != NoDragDrop) {
            setDragDropMode(DragDrop);
        }

        if (isEmptyArea) {
            if (!DFMGlobal::keyCtrlIsPressed()) {
                itemDelegate()->hideNotEditingIndexWidget();

                if (dragDropMode() != NoDragDrop) {
                    setDragDropMode(DropOnly);
                }

                // 避免通过触屏拖动视图时当前选中被清除
                if (event->source() != Qt::MouseEventSynthesizedByQt) {
                    clearSelection();
                    update();
                }
            }
        } else if (DFMGlobal::keyCtrlIsPressed()) {
            const QModelIndex &index = indexAt(event->pos());

            if (selectionModel()->isSelected(index)) {
                d->mouseLastPressedIndex = index;

                DListView::mousePressEvent(event);

                selectionModel()->select(index, QItemSelectionModel::Select);

                return;
            }
        }

        d->mouseLastPressedIndex = QModelIndex();

        DListView::mousePressEvent(event);
        break;
    }
    default: break;
    }
}

void DFileView::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(const DFileView);

    // source为此类型时认为是触屏事件
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        if (QScroller::hasScroller(this))
            return;

        // 在定时器期间收到鼠标move事件且距离大于一定值则认为触发视图滚动
        if (d->updateEnableSelectionByMouseTimer
                && d->updateEnableSelectionByMouseTimer->isActive()) {
            const QPoint difference_pos = event->pos() - d->lastTouchBeginPos;

            if (qAbs(difference_pos.x()) > d->touchTapDistance
                    || qAbs(difference_pos.y()) > d->touchTapDistance) {
                QScroller::grabGesture(this);
                QScroller *scroller = QScroller::scroller(this);

                scroller->handleInput(QScroller::InputPress, event->localPos(), event->timestamp());
                scroller->handleInput(QScroller::InputMove, event->localPos(), event->timestamp());
            }

            return;
        }
    }

    return DListView::mouseMoveEvent(event);
}

void DFileView::mouseReleaseEvent(QMouseEvent *event)
{
    D_D(DFileView);

    d->dragMoveHoverIndex = QModelIndex();

    if (d->mouseLastPressedIndex.isValid() && DFMGlobal::keyCtrlIsPressed()) {
        if (d->mouseLastPressedIndex == indexAt(event->pos()))
            selectionModel()->select(d->mouseLastPressedIndex, QItemSelectionModel::Deselect);
    }

    // 避免滚动视图导致文件选中状态被取消
    if (!QScroller::hasScroller(this))
        return DListView::mouseReleaseEvent(event);
}

void DFileView::updateModelActiveIndex()
{
    Q_D(DFileView);

    const RandeIndexList randeList = visibleIndexes(QRect(QPoint(0, verticalScrollBar()->value()), QSize(size())));

    if (randeList.isEmpty())
        return;

    const RandeIndex &rande = randeList.first();
    DAbstractFileWatcher *fileWatcher = model()->fileWatcher();

    for (int i = d->visibleIndexRande.first; i < rande.first; ++i) {
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(model()->index(i, 0));

        if (fileInfo) {
            fileInfo->makeToInactive();

            if (fileWatcher)
                fileWatcher->setEnabledSubfileWatcher(fileInfo->fileUrl(), false);
        }
    }

    for (int i = rande.second; i < d->visibleIndexRande.second; ++i) {
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(model()->index(i, 0));

        if (fileInfo) {
            fileInfo->makeToInactive();
            if (fileWatcher)
                fileWatcher->setEnabledSubfileWatcher(fileInfo->fileUrl(), false);
        }
    }

    d->visibleIndexRande = rande;

    for (int i = rande.first; i <= rande.second; ++i) {
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(model()->index(i, 0));

        if (fileInfo) {
            fileInfo->makeToActive();

            if (!fileInfo->exists()) {
                model()->removeRow(i, rootIndex());
            } else if (fileWatcher) {
                fileWatcher->setEnabledSubfileWatcher(fileInfo->fileUrl());
            }
        }
    }
}

void DFileView::handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    DListView::dataChanged(topLeft, bottomRight, roles);

    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        update(model()->index(i, 0));
    }
}

void DFileView::delayUpdateStatusBar()
{
    Q_D(DFileView);

    if (FileUtils::isGvfsMountFile(rootUrl().toLocalFile())){
        d->updateStatusBarTimer->start();
    }else{
        updateStatusBar();
    }
}

void DFileView::updateStatusBar()
{
    Q_D(DFileView);
    if (model()->state() != DFileSystemModel::Idle)
        return;

    DFMEvent event(this);
    event.setWindowId(windowId());
    event.setData(selectedUrls());
    int count = selectedIndexCount();

    if (count == 0){
        d->statusBar->itemCounted(event, this->count());
    }else{
        d->statusBar->itemSelected(event, count);
    }
}

void DFileView::openIndexByOpenAction(const int& action, const QModelIndex &index)
{
    if (action == DFMApplication::instance()->appAttribute(DFMApplication::AA_OpenFileMode).toInt())
        if (!DFMGlobal::keyCtrlIsPressed() && !DFMGlobal::keyShiftIsPressed())
            openIndex(index);
}

void DFileView::setIconSizeBySizeIndex(const int &sizeIndex)
{
    QSignalBlocker blocker(statusBar()->scalingSlider());
    Q_UNUSED(blocker)
    statusBar()->scalingSlider()->setValue(sizeIndex);
    itemDelegate()->setIconSizeByIconSizeLevel(sizeIndex);
}


#ifdef SW_LABEL
bool DFileView::checkRenamePrivilege_sw(DUrl fileUrl)
{
    QString srcFileName = fileUrl.toLocalFile();
    if (FileJob::isLabelFile(srcFileName)){
        int nRet = FileJob::checkRenamePrivilege(srcFileName);
        if (nRet != 0){
            emit fileSignalManager->jobFailed(nRet, "rename", srcFileName);
            return false;
        }
    }
    return true;
}
#endif

void DFileView::onRootUrlDeleted(const DUrl &rootUrl)
{
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, rootUrl);
    DUrl new_url = fileInfo ? fileInfo->goToUrlWhenDeleted() : DUrl::fromLocalFile(QDir::homePath());

    requestCdTo(new_url);
}

void DFileView::freshView()
{
    model()->refresh(rootUrl());
}

void DFileView::loadViewState(const DUrl& url)
{
    Q_D(DFileView);

    setIconSizeBySizeIndex(d->fileViewStateValue(url, "iconSizeLevel", DFMApplication::instance()->appAttribute(DFMApplication::AA_IconSizeLevel)).toInt());
    switchViewMode((ViewMode)d->fileViewStateValue(url, "viewMode", (int)d->defaultViewMode).toInt());
}

void DFileView::saveViewState()
{
    //filter url that we are not interesting on
    const DUrl& url = rootUrl();

    if (url.isSearchFile() || !url.isValid() || url.isComputerFile()) {
        return;
    }

    Q_D(DFileView);

    d->setFileViewStateValue(url, "iconSizeLevel", statusBar()->scalingSlider()->value());
    d->setFileViewStateValue(url, "viewMode", (int)viewMode());
}

void DFileView::onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order)
{
    Q_D(DFileView);

    model()->setSortColumn(logicalIndex, order);

    d->oldSelectedUrls = this->selectedUrls();

    if (!d->oldSelectedUrls.isEmpty())
        d->oldCurrentUrl = model()->getUrlByIndex(currentIndex());

    clearSelection();
    model()->sort();

    const DUrl &root_url = rootUrl();

    d->setFileViewStateValue(root_url, "sortRole", model()->sortRole());
    d->setFileViewStateValue(root_url, "sortOrder", (int)order);
}

void DFileView::reset()
{
    DListView::reset();
}

void DFileView::setRootIndex(const QModelIndex &index)
{
    DListView::setRootIndex(index);
}

void DFileView::focusInEvent(QFocusEvent *event)
{
    Q_D(const DFileView);

    // 为了避免QAbstractItemView::focusInEvent会自动设置currentIndex
    QAbstractScrollArea::focusInEvent(event);
    itemDelegate()->commitDataAndCloseActiveEditor();

    /// set menu actions filter
    DFileMenuManager::setActionWhitelist(d->menuWhitelist);
    DFileMenuManager::setActionBlacklist(d->menuBlacklist);
}

void DFileView::resizeEvent(QResizeEvent *event)
{
    Q_D(DFileView);

    DListView::resizeEvent(event);

    updateHorizontalOffset();

    if (itemDelegate()->editingIndex().isValid())
        doItemsLayout();

    d->verticalScrollBar->setFixedSize(d->verticalScrollBar->sizeHint().width(), event->size().height());

    if(d->currentViewMode == IconMode){
        d->verticalScrollBar->move(event->size().width() - d->verticalScrollBar->width(), 0);
    } else if (d->currentViewMode == ListMode){
        d->verticalScrollBar->move(event->size().width() - d->verticalScrollBar->width(), d->headerView->height());
    }

    updateModelActiveIndex();
}

void DFileView::contextMenuEvent(QContextMenuEvent *event)
{
    D_DC(DFileView);

    const QModelIndex &index = indexAt(event->pos());
    bool indexIsSelected = this->isSelected(index);
    bool isEmptyArea = d->fileViewHelper->isEmptyArea(event->pos()) && !indexIsSelected;
    Qt::ItemFlags flags;

    if (isEmptyArea) {
        flags = model()->flags(rootIndex());

        if (!flags.testFlag(Qt::ItemIsEnabled))
            return;
    } else {
        flags = model()->flags(index);

        if (!flags.testFlag(Qt::ItemIsEnabled)) {
            isEmptyArea = true;
            flags = rootIndex().flags();
        }
    }

    if (isEmptyArea) {
        itemDelegate()->hideNotEditingIndexWidget();
        clearSelection();
        showEmptyAreaMenu(flags);
    } else {
        if (!isSelected(index)) {
            setCurrentIndex(index);
        }

        showNormalMenu(index, flags);
    }
}

void DFileView::dragEnterEvent(QDragEnterEvent *event)
{
    for (const DUrl &url : event->mimeData()->urls()) {
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, url);

        if (!fileInfo || !fileInfo->isReadable()) {
            event->ignore();

            return;
        }
    }

    Q_D(const DFileView);

    d->fileViewHelper->preproccessDropEvent(event);

    if (event->mimeData()->hasFormat("XdndDirectSave0")) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();

        return;
    }

    DListView::dragEnterEvent(event);
}

void DFileView::dragMoveEvent(QDragMoveEvent *event)
{
    D_D(DFileView);

    d->dragMoveHoverIndex = d->fileViewHelper->isEmptyArea(event->pos()) ? rootIndex() : indexAt(event->pos());

    if (d->dragMoveHoverIndex.isValid()) {
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(d->dragMoveHoverIndex);

        if (fileInfo) {
            if (!fileInfo->canDrop()
                    || !fileInfo->supportedDropActions().testFlag(event->dropAction())
                    || (fileInfo->isDir() && !fileInfo->isWritable())) {
                d->dragMoveHoverIndex = QModelIndex();
                update();

                return event->ignore();
            }

            d->fileViewHelper->preproccessDropEvent(event);

            event->accept();
        }
    }

    update();

    if (dragDropMode() == InternalMove
        && (event->source() != this || !(event->possibleActions() & Qt::MoveAction)))
        QAbstractItemView::dragMoveEvent(event);
}

void DFileView::dragLeaveEvent(QDragLeaveEvent *event)
{
    D_D(DFileView);

    d->dragMoveHoverIndex = QModelIndex();

    DListView::dragLeaveEvent(event);
}

void DFileView::dropEvent(QDropEvent *event)
{
    D_D(DFileView);

    d->dragMoveHoverIndex = QModelIndex();
    // clean old index area
    update();

    d->fileViewHelper->preproccessDropEvent(event);

    if (event->mimeData()->property("IsDirectSaveMode").toBool()) {
        event->setDropAction(Qt::CopyAction);

        const QModelIndex &index = indexAt(event->pos());
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(index.isValid() ? index : rootIndex());

        if (fileInfo && fileInfo->fileUrl().isLocalFile()) {
            if (fileInfo->isDir())
                const_cast<QMimeData*>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->fileUrl());
            else
                const_cast<QMimeData*>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->parentUrl());
        }

        event->accept(); // yeah! we've done with XDS so stop Qt from further event propagation.
    } else {
        QModelIndex index = d->fileViewHelper->isEmptyArea(event->pos()) ? QModelIndex() : indexAt(event->pos());

        if (!index.isValid())
            index = rootIndex();

        if (!index.isValid())
            return;

        // 防止把目录拖动到自己里边
        if (isSelected(index) && event->source() == this)
            return;

        if (model()->supportedDropActions() & event->dropAction() && model()->flags(index) & Qt::ItemIsDropEnabled) {
            const Qt::DropAction action = dragDropMode() == InternalMove ? Qt::MoveAction : event->dropAction();

            if (model()->dropMimeData(event->mimeData(), action, index.row(), index.column(), index)) {
                if (action != event->dropAction()) {
                    event->setDropAction(action);
                    event->accept();
                } else {
                    event->acceptProposedAction();
                }
            }
        }

        stopAutoScroll();
        setState(NoState);
        viewport()->update();
    }
}

void DFileView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags)
{
    if (DFMGlobal::keyShiftIsPressed()) {
        const QModelIndex &index = indexAt(rect.bottomRight());

        if (!index.isValid())
            return;

        const QModelIndex &lastSelectedIndex = indexAt(rect.topLeft());

        if (!lastSelectedIndex.isValid())
            return;

        selectionModel()->select(QItemSelection(lastSelectedIndex, index), QItemSelectionModel::ClearAndSelect);

        return;
    }

    if (flags == (QItemSelectionModel::Current|QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect)) {
        QRect tmp_rect = rect;

        tmp_rect.translate(horizontalOffset(), verticalOffset());
        tmp_rect.setCoords(qMin(tmp_rect.left(), tmp_rect.right()), qMin(tmp_rect.top(), tmp_rect.bottom()),
                           qMax(tmp_rect.left(), tmp_rect.right()), qMax(tmp_rect.top(), tmp_rect.bottom()));

        const RandeIndexList &list = visibleIndexes(tmp_rect);

        if (list.isEmpty()) {
            clearSelection();

            return;
        }

#ifndef CLASSICAL_SECTION
        return selectionModel()->select(QItemSelection(rootIndex().child(list.first().first, 0),
                                                       rootIndex().child(list.last().second, 0)), flags);
#else
        QItemSelection selection;

        for (const RandeIndex &index : list) {
            selection.append(QItemSelectionRange(rootIndex().child(index.first, 0), rootIndex().child(index.second, 0)));
        }

        return selectionModel()->select(selection, flags);
#endif
    }

    DListView::setSelection(rect, flags);
}

QModelIndex DFileView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    D_D(DFileView);

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
        if (DFMGlobal::keyShiftIsPressed()) {
            index = DListView::moveCursor(cursorAction, modifiers);

            if (index == d->lastCursorIndex) {
                index = index.sibling(index.row() - 1, index.column());
            }
        } else {
            index = current.sibling(current.row() - 1, current.column());
        }

        // rekols: Loop to find the next file item that can be selected.
        while (!(index.flags() & Qt::ItemIsSelectable) && index.isValid()) {
            index = index.sibling(index.row() - 1, index.column());
        }

        break;

    case MoveRight:
        if (DFMGlobal::keyShiftIsPressed()) {
            index = DListView::moveCursor(cursorAction, modifiers);

            if (index == d->lastCursorIndex) {
                index = index.sibling(index.row() + 1, index.column());
            }
        } else {
            index = current.sibling(current.row() + 1, current.column());
        }

        while (!(index.flags() & Qt::ItemIsSelectable) && index.isValid()) {
            index = index.sibling(index.row() + 1, index.column());
        }

        break;

    default:
        index = DListView::moveCursor(cursorAction, modifiers);
        break;
    }

    if (index.isValid()) {
        if (viewMode() == IconMode) {
            bool last_row = indexOfRow(index) == rowCount() - 1;

            if (!last_row
                    && current == index
                    && (cursorAction == MoveDown
                        || cursorAction == MovePageDown
                        || cursorAction == MoveNext)) {
                // 当下一个位置没有元素时，QListView不会自动换一列选择，应该直接选中最后一个
                index = model()->index(count() - 1, 0);
                last_row = true;
            }

            if (last_row) {
                // call later
                QTimer::singleShot(0, this, [this, index, d] {
                    // scroll to end
                    d->verticalScrollBar->setValue(d->verticalScrollBar->maximum());
                });
            }
        }

        d->lastCursorIndex = index;

        return index;
    }

    d->lastCursorIndex = current;

    return current;
}

void DFileView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QModelIndex currentIdx = currentIndex();
    for (const QModelIndex &index : selectedIndexes()) {
        if (index.parent() != parent)
            continue;

        if (index.row() >= start && index.row() <= end) {
            selectionModel()->select(index, QItemSelectionModel::Clear);
            if (index == currentIdx) {
                clearSelection();
            }
        }
    }

    Q_D(DFileView);

    if (start < d->visibleIndexRande.second) {
        if (end <= d->visibleIndexRande.first) {
            d->visibleIndexRande.first -= (end - start + 1);
            d->visibleIndexRande.second -= (end - start + 1);
        } else if (end <= d->visibleIndexRande.second) {
            d->visibleIndexRande.first = start;
            d->visibleIndexRande.second -= (end - start + 1);
        } else {
            d->visibleIndexRande.first = d->visibleIndexRande.second = -1;
        }

    }

    DListView::rowsAboutToBeRemoved(parent, start, end);
}

void DFileView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    D_D(DFileView);

    DListView::rowsInserted(parent, start, end);
}

void DFileView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    D_D(DFileView);

    DListView::dataChanged(topLeft, bottomRight, roles);

    if (d->oldCurrentUrl.isValid())
        setCurrentIndex(model()->index(d->oldCurrentUrl));

    for (const DUrl &url : d->oldSelectedUrls) {
        selectionModel()->select(model()->index(url), QItemSelectionModel::Select);
    }

    /// Clean
    d->oldCurrentUrl = DUrl();
    d->oldSelectedUrls.clear();
}

bool DFileView::event(QEvent *e)
{
    Q_D(DFileView);
    switch (e->type()) {
    case QEvent::KeyPress: {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(e);
            if(keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab){
                if(keyEvent->modifiers() == Qt::ControlModifier || keyEvent->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
                    return DListView::event(e);
                e->accept();

                if(keyEvent->modifiers() == Qt::ShiftModifier)
                    keyPressEvent(new QKeyEvent(keyEvent->type(), Qt::Key_Left, Qt::NoModifier));
                else
                    keyPressEvent(new QKeyEvent(keyEvent->type(), Qt::Key_Right, Qt::NoModifier));

                return true;
            }
        }
        break;
    case QEvent::Resize:
        d->pureResizeEvent(static_cast<QResizeEvent*>(e));
        break;
    case QEvent::ParentChange:
        window()->installEventFilter(this);
    default:
        break;
    }

    return DListView::event(e);
}

void DFileView::updateGeometries()
{
    Q_D(DFileView);

    if (!d->headerView || !d->allowedAdjustColumnSize) {
        return DListView::updateGeometries();
    }

    resizeContents(d->headerView->length(), contentsSize().height());

    DListView::updateGeometries();
}

bool DFileView::eventFilter(QObject *obj, QEvent *event)
{
    Q_D(DFileView);

    switch (event->type()) {
    case QEvent::Move:
        if (obj != horizontalScrollBar()->parentWidget())
            return DListView::eventFilter(obj, event);
        d->updateHorizontalScrollBarPosition();
        break;
    case QEvent::WindowStateChange:
        if (d->headerView) {
            d->toggleHeaderViewSnap(true);
            d->doFileNameColResize();
        }
        break;
    // blumia: 这里通过给横向滚动条加事件过滤器并监听其显示隐藏时间来判断是否应当进入吸附状态。
    //         不过其实可以通过 Resize 事件的 size 和 oldSize 判断是否由于窗口调整大小而进入了吸附状态。
    //         鉴于已经实现完了，如果当前的实现方式实际发现了较多问题，则应当调整为使用 Resize 事件来标记吸附状态的策略。
    case QEvent::ShowToParent:
    case QEvent::HideToParent:
        if (d->headerView && d->cachedViewWidth != this->width()) {
            d->cachedViewWidth = this->width();
            d->toggleHeaderViewSnap(true);
        }
        break;
    default:
        break;
    }

    return DListView::eventFilter(obj, event);
}

void DFileView::onShowHiddenFileChanged()
{
    QDir::Filters filters;

    if (DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedHiddenFiles).toBool())
        filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
    else
        filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;

    model()->setFilters(filters);
}

void DFileView::initDelegate()
{
    D_D(DFileView);

    setItemDelegate(new DIconItemDelegate(d->fileViewHelper));
    setIconSizeBySizeIndex(itemDelegate()->iconSizeLevel());
}

void DFileView::initUI()
{
    D_D(DFileView);

    QPalette palette = this->palette();

    palette.setColor(QPalette::Text, Qt::red);
    setPalette(palette);

    setSpacing(ICON_VIEW_SPACING);
    setResizeMode(QListView::Adjust);
    setOrientation(QListView::LeftToRight, true);
    setTextElideMode(Qt::ElideMiddle);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDropIndicatorShown(false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QListView::EditKeyPressed | QListView::SelectedClicked);
//    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDefaultDropAction(Qt::CopyAction);
    // disable auto remove item when drop of MoveAction finished
    setDragDropOverwriteMode(true);

    DListView::setSelectionRectVisible(true);

    d->displayAsActionGroup = new QActionGroup(this);
    d->sortByActionGroup = new QActionGroup(this);
    d->openWithActionGroup = new QActionGroup(this);
    d->fileViewHelper = new FileViewHelper(this);

    d->statusBar = new DStatusBar(this);
    d->statusBar->scalingSlider()->setPageStep(1);
    d->statusBar->scalingSlider()->setTickInterval(1);

    addFooterWidget(d->statusBar);

    d->verticalScrollBar = verticalScrollBar();
    d->verticalScrollBar->setParent(this);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setViewportMargins(0, 0, -verticalScrollBar()->sizeHint().width(), 0);

    if (d->allowedAdjustColumnSize) {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    }

    d->toolbarActionGroup = new QActionGroup(this);

    updateToolBarActions(this);

    // context width adjustable:
    d->allowedAdjustColumnSize = DFMApplication::instance()->appAttribute(DFMApplication::AA_ViewSizeAdjustable).toBool();
}

void DFileView::initModel()
{
    D_D(DFileView);

    setModel(new DFileSystemModel(d->fileViewHelper));
#ifndef CLASSICAL_SECTION
    setSelectionModel(new DFileSelectionModel(model(), this));
#endif
}

void DFileView::initConnects()
{
    D_D(DFileView);

    connect(this, &DFileView::clicked, [=] (const QModelIndex &index){
        openIndexByOpenAction(0, index);
    });

    connect(this, &DFileView::doubleClicked, [=] (const QModelIndex &index){
        openIndexByOpenAction(1, index);
    });

    connect(this, &DFileView::rowCountChanged, this, &DFileView::onRowCountChanged, Qt::QueuedConnection);

    connect(d->displayAsActionGroup, &QActionGroup::triggered, this, &DFileView::dislpayAsActionTriggered);
    connect(d->sortByActionGroup, &QActionGroup::triggered, this, &DFileView::sortByActionTriggered);
    connect(d->openWithActionGroup, &QActionGroup::triggered, this, &DFileView::openWithActionTriggered);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &DFileView::delayUpdateStatusBar);
    connect(model(), &DFileSystemModel::dataChanged, this, &DFileView::handleDataChanged);
    connect(model(), &DFileSystemModel::stateChanged, this, &DFileView::onModelStateChanged);
    connect(model(), &DFileSystemModel::rootUrlDeleted, this, &DFileView::onRootUrlDeleted);

    connect(this, &DFileView::iconSizeChanged, this, &DFileView::updateHorizontalOffset, Qt::QueuedConnection);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &DFileView::updateModelActiveIndex);

    connect(DFMApplication::instance(), &DFMApplication::iconSizeLevelChanged, this, &DFileView::setIconSizeBySizeIndex);
    connect(DFMApplication::instance(), &DFMApplication::showedHiddenFilesChanged, this, &DFileView::onShowHiddenFileChanged);
    connect(fileSignalManager, &FileSignalManager::requestFreshAllFileView, this, &DFileView::freshView);
    connect(DFMApplication::instance(), &DFMApplication::viewModeChanged, this, [this](const int& viewMode){
        Q_D(const DFileView);
        setDefaultViewMode(static_cast<ViewMode>(viewMode));
    });
    connect(DFMApplication::instance(), &DFMApplication::previewAttributeChanged, this, [this] {
        model()->refresh();
    });

    connect(d->statusBar->scalingSlider(), &QSlider::valueChanged, this, &DFileView::viewStateChanged);
    connect(this, &DFileView::rootUrlChanged, this, &DFileView::loadViewState);
    connect(this, &DFileView::viewStateChanged, this, &DFileView::saveViewState);

    connect(d->toolbarActionGroup, &QActionGroup::triggered, this, [this] {
        Q_D(const DFileView);

        if (d->toolbarActionGroup->actions().first()->isChecked())
            setViewModeToIcon();
        else
            setViewModeToList();
    });

    connect(DThemeManager::instance(), &DThemeManager::widgetThemeChanged, this, &DFileView::updateToolBarActions);
}

void DFileView::increaseIcon()
{
    D_D(DFileView);

    int iconSizeLevel = itemDelegate()->increaseIcon();

    if (iconSizeLevel >= 0) {
        setIconSizeBySizeIndex(iconSizeLevel);
    }
}

void DFileView::decreaseIcon()
{
    D_D(DFileView);

    int iconSizeLevel = itemDelegate()->decreaseIcon();

    if (iconSizeLevel >= 0) {
        setIconSizeBySizeIndex(iconSizeLevel);
    }
}

void DFileView::openIndex(const QModelIndex &index)
{
    D_D(DFileView);

    const DUrl &url = model()->getUrlByIndex(index);

    DFMOpenUrlEvent::DirOpenMode mode = DFMApplication::instance()->appAttribute(DFMApplication::AA_AllwayOpenOnNewWindow).toBool()
                                        ? DFMOpenUrlEvent::ForceOpenNewWindow
                                        : DFMOpenUrlEvent::OpenInCurrentWindow;

    if (mode == DFMOpenUrlEvent::OpenInCurrentWindow)
        DFMEventDispatcher::instance()->processEventAsync<DFMOpenUrlEvent>(this, DUrlList() << url, mode);
    else
        DFMEventDispatcher::instance()->processEvent<DFMOpenUrlEvent>(this, DUrlList() << url, mode);
}

void DFileView::keyboardSearch(const QString &search)
{
    D_D(DFileView);

    if (search.isEmpty())
        return;

    d->fileViewHelper->keyboardSearch(search.toLocal8Bit().at(0));
}

bool DFileView::setRootUrl(const DUrl &url)
{
    D_D(DFileView);

    if (url.isEmpty())
        return false;

    itemDelegate()->hideAllIIndexWidget();

    clearSelection();

    if (!url.isSearchFile()){
        setFocus();
    }

    DUrl fileUrl = url;

    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(this, fileUrl);

    while (info && info->canRedirectionFileUrl()) {
        const DUrl old_url = fileUrl;

        fileUrl = info->redirectedFileUrl();

        if (old_url == fileUrl)
            break;

        info = DFileService::instance()->createFileInfo(this, fileUrl);

        qDebug() << "url redirected, from:" << old_url << "to:" << fileUrl;
    }

    if (!info) {
        qDebug() << "This scheme isn't support, url" << fileUrl;
        return false;
    }

    const DUrl &rootUrl = this->rootUrl();

    qDebug() << "cd: current url:" << rootUrl << "to url:" << fileUrl;

    if (rootUrl == fileUrl)
        return true;

    const DUrl &defaultSelectUrl = DUrl(QUrlQuery(fileUrl.query()).queryItemValue("selectUrl"));

    if (defaultSelectUrl.isValid()) {
        d->preSelectionUrls << defaultSelectUrl;

        QUrlQuery qq(fileUrl.query());

        qq.removeQueryItem("selectUrl");
        fileUrl.setQuery(qq);
    } else if (const DAbstractFileInfoPointer &current_file_info = DFileService::instance()->createFileInfo(this, rootUrl)) {
        QList<DUrl> ancestors;

        if (current_file_info->isAncestorsUrl(fileUrl, &ancestors)) {
            d->preSelectionUrls << (ancestors.count() > 1 ? ancestors.at(ancestors.count() - 2) : rootUrl);
        }
    }

    QModelIndex index = model()->setRootUrl(fileUrl);

    setRootIndex(index);

    if (!model()->canFetchMore(index)) {
        updateContentLabel();
    }

    model()->setSortRole(d->fileViewStateValue(fileUrl, "sortRole", DFileSystemModel::FileDisplayNameRole).toInt(),
                         (Qt::SortOrder)d->fileViewStateValue(fileUrl, "sortOrder", Qt::AscendingOrder).toInt());

    if (d->headerView) {
        updateListHeaderViewProperty();
        // update header view sort indicator
        QSignalBlocker blocker(d->headerView);
        Q_UNUSED(blocker)
        d->headerView->setSortIndicator(model()->sortColumn(), model()->sortOrder());
    }

    if (info) {
        ViewModes modes = (ViewModes)info->supportViewMode();

        //view mode support handler
        toolBarActionList().first()->setVisible(testViewMode(modes, IconMode));
        toolBarActionList().at(1)->setVisible(testViewMode(modes, ListMode));
    }

    emit rootUrlChanged(fileUrl);

    if (fileUrl.isSearchFile()) {
        setViewMode(ListMode);
    }

    // NOTE(zccrs): 视图模式切换失败后，被选中的action是一个错误的。此时切换目录，应该在目录改变后再根据当前视图模式重设action的选中状态。
    if (viewMode() == IconMode) {
        toolBarActionList().first()->setChecked(true);
    } else {
        toolBarActionList().at(1)->setChecked(true);
    }

    const QList<DAbstractFileInfo::SelectionMode> &supportSelectionModes = info->supportSelectionModes();

    for (DAbstractFileInfo::SelectionMode mode : supportSelectionModes) {
        if (d->enabledSelectionModes.contains((SelectionMode)mode)) {
            setSelectionMode((SelectionMode)mode);
            break;
        }
    }

    return true;
}

void DFileView::clearHeardView()
{
    D_D(DFileView);

    if (d->headerView) {
        removeHeaderWidget(0);

        d->headerView->disconnect();
        d->headerView = nullptr;
        d->headerViewHolder = nullptr;
    }
}

void DFileView::clearSelection()
{
    QListView::clearSelection();
    setCurrentIndex(QModelIndex());
}

void DFileView::setContentLabel(const QString &text)
{
    D_D(DFileView);

    if (!d->contentLabel) {
        d->contentLabel = new QLabel(this);
        d->contentLabel.setCenterIn(this);
        d->contentLabel->setObjectName("contentLabel");
        d->contentLabel->setStyleSheet(this->styleSheet());
        d->contentLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        d->contentLabel->show();
    }

    d->contentLabel->setText(text);
    d->contentLabel->adjustSize();
}

void DFileView::setMenuActionWhitelist(const QSet<MenuAction> &actionList)
{
    Q_D(DFileView);

    d->menuWhitelist = actionList;

    if (focusWidget() == this) {
        DFileMenuManager::setActionWhitelist(d->menuWhitelist);
    }
}

void DFileView::setMenuActionBlacklist(const QSet<MenuAction> &actionList)
{
    Q_D(DFileView);

    d->menuBlacklist = actionList;

    if (focusWidget() == this) {
        DFileMenuManager::setActionBlacklist(d->menuBlacklist);
    }
}

void DFileView::updateHorizontalOffset()
{
    D_D(DFileView);

    if (isIconViewMode()) {
        int contentWidth = maximumViewportSize().width();
        int itemWidth = itemSizeHint().width() + spacing() * 2;
        int itemColumn = d->iconModeColumnCount(itemWidth);

        d->horizontalOffset = -(contentWidth - itemWidth * itemColumn) / 2;
    } else {
        d->horizontalOffset = 0;
    }
}

void DFileView::switchViewMode(DFileView::ViewMode mode)
{
    D_D(DFileView);

    if (d->currentViewMode == mode) {
        return;
    }

    const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(rootUrl());

    if (fileInfo && (fileInfo->supportViewMode() & mode) == 0) {
        return;
    }

    d->currentViewMode = mode;

    itemDelegate()->hideAllIIndexWidget();

    horizontalScrollBar()->parentWidget()->removeEventFilter(this);

    switch (mode) {
    case IconMode: {
        clearHeardView();
        d->columnRoles.clear();
        setOrientation(QListView::LeftToRight, true);
        setSpacing(ICON_VIEW_SPACING);
        setItemDelegate(new DIconItemDelegate(d->fileViewHelper));
        setUniformItemSizes(false);
        setResizeMode(Adjust);

        d->statusBar->scalingSlider()->show();
        itemDelegate()->setIconSizeByIconSizeLevel(d->statusBar->scalingSlider()->value());
        d->toolbarActionGroup->actions().first()->setChecked(true);

        break;
    }
    case ListMode: {
        setItemDelegate(new DListItemDelegate(d->fileViewHelper));
        setUniformItemSizes(true);
        setResizeMode(Fixed);

        if (!d->headerView) {
            if (d->allowedAdjustColumnSize) {
                d->headerViewHolder = new QWidget(this);
                d->headerView = new DFMHeaderView(Qt::Horizontal, d->headerViewHolder);

                connect(d->headerView, &DFMHeaderView::viewResized, this, [this, d] {
                    d->headerViewHolder->setFixedHeight(d->headerView->height());
                });
                connect(d->headerView, &DFMHeaderView::sectionResized, d->headerView, &DFMHeaderView::adjustSize);
                connect(d->headerView, SIGNAL(sectionHandleDoubleClicked(int)), this, SLOT(_q_onSectionHandleDoubleClicked(int)));
            } else {
                d->headerView = new DFMHeaderView(Qt::Horizontal, this);
                d->headerViewHolder = d->headerView;
            }

            updateListHeaderViewProperty();

            d->headerView->setHighlightSections(false);
            d->headerView->setSectionsClickable(true);
            d->headerView->setSortIndicatorShown(true);
            d->headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            d->headerView->setContextMenuPolicy(Qt::CustomContextMenu);

            if(selectionModel()) {
                d->headerView->setSelectionModel(selectionModel());
            }

            connect(d->headerView, &QHeaderView::sectionResized,
                    this, static_cast<void (DFileView::*)()>(&DFileView::update));
            connect(d->headerView, &QHeaderView::sortIndicatorChanged,
                    this, &DFileView::onSortIndicatorChanged);
            connect(d->headerView, &QHeaderView::customContextMenuRequested,
                    this, &DFileView::popupHeaderViewContextMenu);
            connect(d->headerView, &DFMHeaderView::mouseReleased, this, [ = ] {
                d->toggleHeaderViewSnap(false);
                QList<int> roleList = columnRoleList();
                QVariantMap state;
                for (const int role : roleList) {
                    int colWidth = columnWidth(model()->roleToColumn(role));

                    if (colWidth > 0)
                        state[QString::number(role)] = colWidth;
                }
                DFMApplication::appObtuselySetting()->setValue("WindowManager", "ViewColumnState", state);
            });
            connect(horizontalScrollBar(), &QScrollBar::valueChanged, d->headerView,
            [d] (int value) {
                d->headerView->move(-value, d->headerView->y());
            });

            if (d->allowedAdjustColumnSize) {
                connect(d->headerView, &QHeaderView::sectionResized,
                        this, &DFileView::updateGeometries);
            }

            d->headerView->setAttribute(Qt::WA_TransparentForMouseEvents, model()->state() == DFileSystemModel::Busy);
        }

        addHeaderWidget(d->headerViewHolder);

        setOrientation(QListView::TopToBottom, false);
        setSpacing(LIST_VIEW_SPACING);
        d->statusBar->scalingSlider()->hide();
        d->toolbarActionGroup->actions().at(1)->setChecked(true);

        if (d->allowedAdjustColumnSize) {
            horizontalScrollBar()->parentWidget()->installEventFilter(this);
            // 初始化列宽调整
            d->cachedViewWidth = this->width();
            d->adjustFileNameCol = d->headerView->width() == this->width();
        }
        break;
    }
    case ExtendMode: {
        break;
    }
    default:
        break;
    }

    setFocus();

    emit viewModeChanged(mode);
}

void DFileView::showEmptyAreaMenu(const Qt::ItemFlags &indexFlags)
{
    Q_UNUSED(indexFlags)
    D_D(DFileView);

    const QModelIndex &index = rootIndex();
    const DAbstractFileInfoPointer &info = model()->fileInfo(index);
    const QVector<MenuAction> &actions = info->menuActionList(DAbstractFileInfo::SpaceArea);

    if (actions.isEmpty())
        return;

    const QMap<MenuAction, QVector<MenuAction> > &subActions = info->subMenuActionList();

    QSet<MenuAction> disableList = DFileMenuManager::getDisableActionList(model()->getUrlByIndex(index));

    if (model()->state() != DFileSystemModel::Idle){
        disableList << MenuAction::SortBy;
    }

//    if (!indexFlags.testFlag(Qt::ItemIsEditable))
//        disableList << MenuAction::NewDocument << MenuAction::NewFolder << MenuAction::Paste;

    const bool& tabAddable = WindowManager::tabAddableByWinId(windowId());
    if(!tabAddable)
        disableList << MenuAction::OpenInNewTab;

    if (!count())
        disableList << MenuAction::SelectAll;

    DFileMenu *menu = DFileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);
    QAction *tmp_action = menu->actionAt(fileMenuManger->getActionString(MenuAction::DisplayAs));
    DFileMenu *displayAsSubMenu = static_cast<DFileMenu*>(tmp_action ? tmp_action->menu() : Q_NULLPTR);
    tmp_action = menu->actionAt(fileMenuManger->getActionString(MenuAction::SortBy));
    DFileMenu *sortBySubMenu = static_cast<DFileMenu*>(tmp_action ? tmp_action->menu() : Q_NULLPTR);

    for (QAction *action : d->displayAsActionGroup->actions()) {
        d->displayAsActionGroup->removeAction(action);
    }

    if (displayAsSubMenu) {
        foreach (QAction* action, displayAsSubMenu->actions()) {
            action->setActionGroup(d->displayAsActionGroup);
            action->setCheckable(true);
            action->setChecked(false);
        }

        QAction *currentViewModeAction = nullptr;

        if (d->currentViewMode == IconMode) {
            currentViewModeAction = displayAsSubMenu->actionAt(fileMenuManger->getActionString(MenuAction::IconView));
        } else if (d->currentViewMode == ListMode) {
            currentViewModeAction = displayAsSubMenu->actionAt(fileMenuManger->getActionString(MenuAction::ListView));
        } else if (d->currentViewMode == ExtendMode) {
            currentViewModeAction = displayAsSubMenu->actionAt(fileMenuManger->getActionString(MenuAction::ExtendView));
        }

        if (currentViewModeAction)
            currentViewModeAction->setChecked(true);
    }

    for (QAction *action : d->sortByActionGroup->actions()) {
        d->sortByActionGroup->removeAction(action);
    }

    if (sortBySubMenu){
        foreach (QAction* action, sortBySubMenu->actions()) {
            action->setActionGroup(d->sortByActionGroup);
            action->setCheckable(true);
            action->setChecked(false);
        }

        QAction *action = sortBySubMenu->actionAt(info->sortSubMenuActionUserColumnRoles().indexOf(model()->sortRole()));

        if (action)
            action->setChecked(true);
    }


    DFileMenuManager::loadEmptyAreaPluginMenu(menu, rootUrl(), false);
    DFileMenuManager::loadEmptyAreaExtensionMenu(menu, rootUrl(), false);

    if (!menu) {
        return;
    }

    menu->setEventData(rootUrl(), selectedUrls(), windowId(), this);

    fileViewHelper()->handleMenu(menu);

    menu->exec();
    menu->deleteLater();
}


void DFileView::showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags)
{
    D_D(DFileView);

    if(!index.isValid())
        return;

    DUrlList list = selectedUrls();

    DFileMenu* menu;

#ifdef SW_LABEL
    DAbstractFileInfoPointer info = model()->fileInfo(index);
    info->updateLabelMenuItems();
#else
    const DAbstractFileInfoPointer &info = model()->fileInfo(index);
#endif

    QSet<MenuAction> disableList;
    QSet<MenuAction> unusedList;

    // blumia: when touching this part, do the same change in canvasgridview.cpp
    if(list.size() == 1){
        if (!info->isReadable() && !info->isSymLink()) {
            disableList << MenuAction::Copy;
        }

        if (!info->isWritable() && !info->isFile() && !info->isSymLink()) {
            disableList << MenuAction::Delete;
        }

        if (!indexFlags.testFlag(Qt::ItemIsEditable)) {
            disableList << MenuAction::Rename;
        }
    }

    menu = DFileMenuManager::createNormalMenu(info->fileUrl(), list, disableList, unusedList, windowId());

    if (!menu){
        return;
    }

    menu->setEventData(rootUrl(), selectedUrls(), windowId(), this);

    fileViewHelper()->handleMenu(menu);

    menu->exec();
    menu->deleteLater();
}

void DFileView::updateListHeaderViewProperty()
{
    D_D(DFileView);

    if (!d->headerView)
        return;

    d->headerView->setModel(Q_NULLPTR);
    d->headerView->setModel(model());

    d->headerView->setDefaultSectionSize(DEFAULT_HEADER_SECTION_WIDTH);

    if (d->allowedAdjustColumnSize) {
        d->headerView->setSectionResizeMode(QHeaderView::Interactive);
        d->headerView->setMinimumSectionSize(LIST_VIEW_MINIMUM_WIDTH);
    } else {
        d->headerView->setSectionResizeMode(QHeaderView::Fixed);
        d->headerView->setMinimumSectionSize(DEFAULT_HEADER_SECTION_WIDTH);
    }

    d->headerView->setSortIndicator(model()->sortColumn(), model()->sortOrder());
    d->columnRoles.clear();

    // set value from config file.
    const QVariantMap &state = DFMApplication::appObtuselySetting()->value("WindowManager", "ViewColumnState").toMap();

    for (int i = 0; i < d->headerView->count(); ++i) {
        d->columnRoles << model()->columnToRole(i);

        if (d->allowedAdjustColumnSize) {
            int colWidth = state.value(QString::number(d->columnRoles.last()), -1).toInt();
            if (colWidth > 0) {
                d->headerView->resizeSection(model()->roleToColumn(d->columnRoles.last()), colWidth);
            }
        } else {
            int column_width = model()->columnWidth(i);
            if (column_width >= 0) {
                d->headerView->resizeSection(i, column_width + COLUMU_PADDING * 2);
            } else {
                d->headerView->setSectionResizeMode(i, QHeaderView::Stretch);
            }
        }

        const QString &column_name = model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();

        if (!d->columnForRoleHiddenMap.contains(column_name)) {
            d->headerView->setSectionHidden(i, !model()->columnDefaultVisibleForRole(model()->columnToRole(i)));
        } else {
            d->headerView->setSectionHidden(i, d->columnForRoleHiddenMap.value(column_name));
        }
    }

    if (d->adjustFileNameCol) {
        d->doFileNameColResize();
    }

    updateColumnWidth();
}

void DFileView::updateExtendHeaderViewProperty()
{
    D_D(DFileView);

    if (!d->headerView)
        return;

    d->headerView->setModel(Q_NULLPTR);
    d->headerView->setModel(model());
    d->headerView->setDefaultSectionSize(DEFAULT_HEADER_SECTION_WIDTH);

    if (!d->allowedAdjustColumnSize) {
        d->headerView->setSectionResizeMode(QHeaderView::Fixed);
        d->headerView->setSectionResizeMode(0, QHeaderView::Stretch);
        d->headerView->setMinimumSectionSize(DEFAULT_HEADER_SECTION_WIDTH);
    }

    d->columnRoles.clear();
    d->columnRoles << model()->columnToRole(0);
}

void DFileView::updateColumnWidth()
{
    D_D(DFileView);

    if (!d->allowedAdjustColumnSize) {
        int column_count = d->headerView->count();
        int i = 0;
        int j = column_count - 1;

        for (; i < column_count; ++i) {
            if (d->headerView->isSectionHidden(i))
                continue;

            d->headerView->resizeSection(i, model()->columnWidth(i) + LEFT_PADDING + LIST_MODE_LEFT_MARGIN + 2 * COLUMU_PADDING);
            break;
        }

        for (; j > 0; --j) {
            if (d->headerView->isSectionHidden(j))
                continue;

            d->headerView->resizeSection(j, model()->columnWidth(j) + RIGHT_PADDING + LIST_MODE_RIGHT_MARGIN + 2 * COLUMU_PADDING);
            break;
        }

        if (d->firstVisibleColumn != i) {
            if (d->firstVisibleColumn > 0)
                d->headerView->resizeSection(d->firstVisibleColumn, model()->columnWidth(d->firstVisibleColumn) + 2 * COLUMU_PADDING);

            d->firstVisibleColumn = i;
        }

        if (d->lastVisibleColumn != j) {
            if (d->lastVisibleColumn > 0)
                d->headerView->resizeSection(d->lastVisibleColumn, model()->columnWidth(d->lastVisibleColumn) + 2 * COLUMU_PADDING);

            d->lastVisibleColumn = j;
        }
    }
}

void DFileView::popupHeaderViewContextMenu(const QPoint &pos)
{
    D_D(DFileView);
    const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(rootIndex());
    QMenu *menu = new QMenu();

    if (fileInfo && fileInfo->columnIsCompact()) {
        /*contextmenu of headview for sort function*/
        int column = d->headerView->logicalIndexAt(pos.x());

        const QList<int> &childRoles = fileInfo->userColumnChildRoles(column);

        if (childRoles.isEmpty()) {
            menu->deleteLater();

            return;
        }

        for (int i = 0; i < childRoles.count() * 2; ++i) {
            int childRole = childRoles.at(i / 2);
            QAction *action = new QAction(menu);

            if (i % 2 == 0) {
                action->setText(fileInfo->userColumnDisplayName(childRole).toString());
            } else if (i % 2 == 1) {
                action->setText(fileInfo->userColumnDisplayName(childRole).toString() + tr("(Reverse)"));
            }

            action->setCheckable(true);

            if (!d->headerView->isSectionHidden(d->headerView->sortIndicatorSection())
                    && model()->sortRole() == childRole) {
                if (i % 2 == 1 && model()->sortOrder() == Qt::DescendingOrder) {
                    action->setChecked(true);
                } if (i % 2 == 0 && model()->sortOrder() == Qt::AscendingOrder) {
                    action->setChecked(true);
                }
            }

            connect(action, &QAction::triggered, this, [this, action, column, i, d, childRoles] {
                if (i % 2 == 0) {
                    sortByRole(childRoles.at(i / 2), Qt::AscendingOrder);
                } else if (i % 2 == 1) {
                    sortByRole(childRoles.at(i / 2), Qt::DescendingOrder);
                }
            });

            menu->addAction(action);
        }
    } else {
        for (int i = 1; i < d->headerView->count(); ++i) {
            QAction *action = new QAction(menu);

            action->setText(model()->columnNameByRole(model()->columnToRole(i)).toString());
            action->setCheckable(true);
            action->setChecked(!d->headerView->isSectionHidden(i));

            connect(action, &QAction::triggered, this, [this, action, i, d] {
                action->setChecked(!action->isChecked());
                d->columnForRoleHiddenMap[action->text()] = action->isChecked();

                d->headerView->setSectionHidden(i, action->isChecked());
                updateColumnWidth();
            });

            menu->addAction(action);
        }
    }

    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void DFileView::onModelStateChanged(int state)
{
    D_D(DFileView);

    DFMEvent event(this);

    event.setWindowId(windowId());
    event.setData(rootUrl());

    if (state == DFileSystemModel::Busy) {
        QString tipText;

        if (const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(rootIndex())) {
            tipText = fileInfo->loadingTip();
        }

        d->statusBar->setLoadingIncatorVisible(state == DFileSystemModel::Busy, tipText);

        setContentLabel(QString());

        if (d->headerView) {
            d->headerView->setAttribute(Qt::WA_TransparentForMouseEvents);
        }
    } else if (state == DFileSystemModel::Idle) {
        d->statusBar->setLoadingIncatorVisible(state == DFileSystemModel::Busy);

        if (!d->preSelectionUrls.isEmpty()) {
            const QModelIndex &index = model()->index(d->preSelectionUrls.first());

            setCurrentIndex(index);
            scrollTo(index, PositionAtTop);
        }

        for (const DUrl &url : d->preSelectionUrls) {
            selectionModel()->select(model()->index(url), QItemSelectionModel::Select);
        }

        d->preSelectionUrls.clear();

        delayUpdateStatusBar();
        updateContentLabel();

        if (d->headerView) {
            d->headerView->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        }
    }

    // 通知对应的主窗口
    notifyStateChanged();
}

void DFileView::updateContentLabel()
{
    if (model()->state() != DFileSystemModel::Idle
            || model()->canFetchMore(rootIndex())) {
        setContentLabel(QString());

        return;
    }

    int count = this->count();

    if (count <= 0) {
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(rootIndex());

        if (fileInfo) {
            setContentLabel(fileInfo->subtitleForEmptyFloder());

            return;
        }
    }

    setContentLabel(QString());
}

void DFileView::updateToolBarActions(QWidget *widget, QString theme)
{
    Q_UNUSED(theme)
    D_D(DFileView);
    if (widget == this){
        QAction *icon_view_mode_action;
        QAction *list_view_mode_action;
        const QList<QAction*> actions = d->toolbarActionGroup->actions();

        if (actions.count() > 1) {
            icon_view_mode_action = actions.first();
            list_view_mode_action = actions.at(1);
        } else {
            icon_view_mode_action = new QAction(this);
            list_view_mode_action = new QAction(this);

            icon_view_mode_action->setCheckable(true);
            icon_view_mode_action->setChecked(true);

            list_view_mode_action->setCheckable(true);

            d->toolbarActionGroup->addAction(icon_view_mode_action);
            d->toolbarActionGroup->addAction(list_view_mode_action);
        }

        QIcon icon_view_mode_icon;
        icon_view_mode_icon.addFile(ThemeConfig::instace()->string("FileView", "iconview.icon"));
        icon_view_mode_icon.addFile(ThemeConfig::instace()->string("FileView", "iconview.icon", ThemeConfig::Hover), QSize(), QIcon::Active);
        icon_view_mode_icon.addFile(ThemeConfig::instace()->string("FileView", "iconview.icon", ThemeConfig::Checked), QSize(), QIcon::Normal, QIcon::On);
        icon_view_mode_action->setIcon(icon_view_mode_icon);

        QIcon list_view_mode_icon;
        list_view_mode_icon.addFile(ThemeConfig::instace()->string("FileView", "listview.icon"));
        list_view_mode_icon.addFile(ThemeConfig::instace()->string("FileView", "listview.icon", ThemeConfig::Hover), QSize(), QIcon::Active);
        list_view_mode_icon.addFile(ThemeConfig::instace()->string("FileView", "listview.icon", ThemeConfig::Checked), QSize(), QIcon::Normal, QIcon::On);
        list_view_mode_action->setIcon(list_view_mode_icon);
    }
}

void DFileView::refresh()
{
    model()->refresh();
}

int DFileViewPrivate::iconModeColumnCount(int itemWidth) const
{
    Q_Q(const DFileView);

//    int frameAroundContents = 0;
//    if (q->style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents))
//        frameAroundContents = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2;

    int horizontalMargin = /*q->verticalScrollBarPolicy()==Qt::ScrollBarAsNeeded
            ? q->style()->pixelMetric(QStyle::PM_ScrollBarExtent, 0, q->verticalScrollBar()) + frameAroundContents
            : */0;

    int contentWidth = q->maximumViewportSize().width();

    if (itemWidth <= 0)
        itemWidth = q->itemSizeHint().width() + q->spacing() * 2;

    return (contentWidth - horizontalMargin - 1) / itemWidth;
}

QVariant DFileViewPrivate::fileViewStateValue(const DUrl &url, const QString &key, const QVariant &defalutValue)
{
    return DFMApplication::appObtuselySetting()->value("FileViewState", url).toMap().value(key, defalutValue);
}

void DFileViewPrivate::setFileViewStateValue(const DUrl &url, const QString &key, const QVariant &value)
{
    QVariantMap map = DFMApplication::appObtuselySetting()->value("FileViewState", url).toMap();

    map[key] = value;

    DFMApplication::appObtuselySetting()->setValue("FileViewState", url, map);
}

void DFileViewPrivate::updateHorizontalScrollBarPosition()
{
    Q_Q(DFileView);

    QWidget *widget = static_cast<QWidget*>(q->horizontalScrollBar()->parentWidget());

    // 更新横向滚动条的位置，将它显示在状态栏上面（此处没有加防止陷入死循环的处理）
    widget->move(widget->x(), q->height() - statusBar->height() - widget->height());
}

void DFileViewPrivate::pureResizeEvent(QResizeEvent *event)
{
    Q_Q(DFileView);

    if (!allowedAdjustColumnSize) {
        // auto switch list mode
        if (currentViewMode == DFileView::ListMode
                && DFMApplication::instance()->appAttribute(DFMApplication::AA_ViewAutoCompace).toBool()) {
            if (q->model()->setColumnCompact(event->size().width() < 600)) {
                q->updateListHeaderViewProperty();
                q->doItemsLayout();
            }
        }
    } else {
        doFileNameColResize();
    }
}

void DFileViewPrivate::doFileNameColResize()
{
    Q_Q(DFileView);

    if (allowedAdjustColumnSize && headerView && adjustFileNameCol) {
        int fileNameColRole = q->model()->roleToColumn(DFileSystemModel::FileDisplayNameRole);
        int columnCount = headerView->count();
        int columnWidthSumOmitFileName = 0;
        for (int i = 0; i < columnCount; ++i) {
            if (i == fileNameColRole || headerView->isSectionHidden(i))
                continue;
            columnWidthSumOmitFileName += q->columnWidth(i);
        }

        int targetWidth = q->width() - columnWidthSumOmitFileName;
        if (targetWidth >= headerView->minimumSectionSize()) {
            headerView->resizeSection(fileNameColRole, q->width() - columnWidthSumOmitFileName);
        }
    }
}

void DFileViewPrivate::toggleHeaderViewSnap(bool on)
{
    adjustFileNameCol = on;
    //    DFMApplication::appObtuselySetting()->setValue("WindowManager", "HeaderViewSnapped", on);
}

void DFileViewPrivate::_q_onSectionHandleDoubleClicked(int logicalIndex)
{
    Q_Q(DFileView);

    if (q->model()->state() != DFileSystemModel::Idle)
        return;

    int row_count = q->model()->rowCount();

    if (row_count < 1) {
        return;
    }

    QStyleOptionViewItem option = q->viewOptions();

    option.rect.setWidth(QWIDGETSIZE_MAX);
    option.rect.setHeight(q->itemSizeHint().height());

    int column_max_width = 0;

    for (int i = 0; i < row_count; ++i) {
        const QModelIndex &index = q->model()->index(i, 0);
        const QList<QRect> &list = q->itemDelegate()->paintGeomertys(option, index, true);

        // 第0列为文件名列，此列比较特殊，因为前面还有文件图标占用了一部分空间
        int width = 0;

        if (logicalIndex == 0) {
            width = list.at(1).right() + COLUMU_PADDING / 2;
        } else {
            width = list.at(logicalIndex + 1).width() + COLUMU_PADDING * 2;
        }

        if (width > column_max_width) {
            column_max_width = width;
        }
    }

    for (int i = headerView->count() - 1; i >= 0; --i) {
        if (headerView->isSectionHidden(i))
            continue;

        // 最后一列要多加上视图的右margin
        if (i == logicalIndex)
            column_max_width += LIST_MODE_RIGHT_MARGIN;

        break;
    }

    headerView->resizeSection(logicalIndex, column_max_width);
}

#include "moc_dfileview.cpp"
