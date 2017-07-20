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
#include "dfmeventdispatcher.h"
#include "app/define.h"
#include "app/filesignalmanager.h"

#include "interfaces/dfmglobal.h"
#include "interfaces/diconitemdelegate.h"
#include "interfaces/dlistitemdelegate.h"
#include "interfaces/dfmsetting.h"
#include "settings.h"
#include "dcrumbwidget.h"
#include "controllers/fmstatemanager.h"

#include "controllers/appcontroller.h"
#include "dfileservices.h"
#include "controllers/fmstatemanager.h"
#include "controllers/pathmanager.h"

#include "models/dfileselectionmodel.h"
#include "dfilesystemmodel.h"

#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "shutil/viewstatesmanager.h"
#include "fileoperations/filejob.h"

#include "singleton.h"
#include "interfaces/dfilemenumanager.h"
#include <dthememanager.h>
#include <anchors.h>

#include <QLineEdit>
#include <QTextEdit>
#include <QUrlQuery>
#include <QActionGroup>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMimeData>
#include <QScrollBar>

DWIDGET_USE_NAMESPACE

#define ICON_VIEW_SPACING 5
#define LIST_VIEW_SPACING 1

#define DEFAULT_HEADER_SECTION_WIDTH 140

class DFileViewPrivate
{
public:
    DFileViewPrivate(DFileView *qq)
        : q_ptr(qq) {}

    int iconModeColumnCount(int itemWidth = 0) const;

    DFileView *q_ptr;

    DFileMenuManager* fileMenuManager;
    QHeaderView *headerView = Q_NULLPTR;
    DStatusBar* statusBar=NULL;

    QActionGroup* displayAsActionGroup;
    QActionGroup* sortByActionGroup;
    QActionGroup* openWithActionGroup;

    QList<int> columnRoles;

    const DFileView::ViewMode defaultViewMode = DFileView::IconMode;
    DFileView::ViewMode currentViewMode = DFileView::IconMode;

    int horizontalOffset = 0;

    /// move cursor later selecte index when pressed key shift
    QModelIndex lastCursorIndex;

    /// list mode column visible
    QMap<QString, bool> columnForRoleHiddenMap;

    int firstVisibleColumn = -1;
    int lastVisibleColumn = -1;

    DUrlList preSelectionUrls;

    Anchors<QLabel> contentLabel = Q_NULLPTR;

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

    initUI();
    initModel();
    initDelegate();
    initConnects();

    d->statusBar->scalingSlider()->setValue(globalSetting->iconSizeIndex());
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
    return model()->getUrlByIndex(rootIndex());
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

QPair<int, Qt::SortOrder> DFileView::getSortRoles() const
{
    return QPair<int, Qt::SortOrder>(model()->sortRole(), model()->sortOrder());
}

void DFileView::setSortRoles(const int &role, const Qt::SortOrder &order)
{
    model()->setSortRole(role, order);
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
        rect.setLeft(LIST_VIEW_SPACING);
        rect.setRight(viewport()->width() - LIST_VIEW_SPACING - 1);
        rect.setTop(index.row() * (item_size.height() + LIST_VIEW_SPACING * 2) + LIST_VIEW_SPACING);
        rect.setHeight(item_size.height());
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

    return itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex());
}

bool DFileView::isDropTarget(const QModelIndex &index) const
{
    D_DC(DFileView);

    return d->dragMoveHoverIndex == index;
}

bool DFileView::cd(const DUrl &url)
{
    return setRootUrl(url);
}

bool DFileView::cdUp()
{
    const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(rootIndex());

    const DUrl &oldCurrentUrl = rootUrl();
    const DUrl& parentUrl = fileInfo ? fileInfo->parentUrl() : DUrl::parentUrl(oldCurrentUrl);

    return cd(parentUrl);
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
#endif
#ifdef SW_LABEL
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

        if (index == root || !index.isValid())
            continue;

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

void DFileView::setViewMode(DFileView::ViewMode mode)
{
    D_D(DFileView);

    switchViewMode(mode);
    emit viewStateChanged();
}

void DFileView::sortByColumn(int column)
{
    D_D(DFileView);

    Qt::SortOrder order = (model()->sortColumn() == column && model()->sortOrder() == Qt::AscendingOrder)
                            ? Qt::DescendingOrder
                            : Qt::AscendingOrder;

    if (d->headerView) {
        QSignalBlocker blocker(d->headerView);
        Q_UNUSED(blocker)
        d->headerView->setSortIndicator(column, order);
    }

    sort(column, order);

    FMStateManager::cacheSortState(rootUrl(), model()->sortRole(), order);
}

void DFileView::sort(int column, Qt::SortOrder order)
{
    D_D(DFileView);

    model()->setSortColumn(column, order);

    d->oldSelectedUrls = this->selectedUrls();

    if (!d->oldSelectedUrls.isEmpty())
        d->oldCurrentUrl = model()->getUrlByIndex(currentIndex());

    clearSelection();
    model()->sort();
    emit viewStateChanged();
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
    D_DC(DFileView);

    QAction* dAction = static_cast<QAction*>(action);

    sortByColumn(d->sortByActionGroup->actions().indexOf(dAction));
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
    if(isIconViewMode() && DFMGlobal::keyCtrlIsPressed()) {
        if(event->angleDelta().y() > 0) {
            increaseIcon();
        } else {
            decreaseIcon();
        }
        emit viewStateChanged();
        event->accept();
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
            d->fileViewHelper->showPreviewFileDialog();
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
            DFileManagerWindow* w = qobject_cast<DFileManagerWindow*>(WindowManager::getWindowById(windowId()));
            if(!w){
                cdUp();
            } else{
                const DUrl& url = w->getToolBar()->getCrumWidget()->backUrl();
                cd(url);
            }
        }
            return;
        case Qt::Key_Delete:
            fileService->moveToTrash(this, urls);
            break;
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
        case Qt::Key_N:{
            appController->actionNewWindow(dMakeEventPointer<DFMUrlListBaseEvent>(this, urls.isEmpty() ? DUrlList() << DUrl() : urls));
            return;
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
            const QString& path = globalSetting->newTabPath();
            if(selectedIndexCount() == 1 && model()->fileInfo(selectedIndexes().first())->isDir()){
                url = model()->fileInfo(selectedIndexes().first())->fileUrl();
            } else{
                if(path != "Current Path")
                    url = DUrl::fromUserInput(path);
                else
                    url = rootUrl();
            }
            DFMEventDispatcher::instance()->processEvent<DFMOpenNewTabEvent>(this, url);
            return;
        }
        default: break;
        }
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
        bool isEmptyArea = d->fileViewHelper->isEmptyArea(event->pos());

        setDragDropMode(DragDrop);

        if (isEmptyArea) {
            if (!DFMGlobal::keyCtrlIsPressed()) {
                itemDelegate()->hideNotEditingIndexWidget();
                clearSelection();
                setDragDropMode(DropOnly);
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

void DFileView::mouseReleaseEvent(QMouseEvent *event)
{
    D_D(DFileView);

    d->dragMoveHoverIndex = QModelIndex();

    if (d->mouseLastPressedIndex.isValid() && DFMGlobal::keyCtrlIsPressed()) {
        if (d->mouseLastPressedIndex == indexAt(event->pos()))
            selectionModel()->select(d->mouseLastPressedIndex, QItemSelectionModel::Deselect);
    }

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

    for (int i = rande.first; i <= rande.second; ++i) {
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(model()->index(i, 0));

        if (fileInfo) {
            fileInfo->makeToActive();
            if (fileWatcher)
                fileWatcher->setEnabledSubfileWatcher(fileInfo->fileUrl());
        }
    }

    d->visibleIndexRande = rande;
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
    if(action == globalSetting->openFileAction())
        if(!DFMGlobal::keyCtrlIsPressed() && !DFMGlobal::keyShiftIsPressed())
            openIndex(index);
}

void DFileView::setIconSizeBySizeIndex(const int &sizeIndex)
{
    statusBar()->scalingSlider()->setValue(sizeIndex);
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

    if (!fileInfo)
        return;

    DFMEventDispatcher::instance()->processEvent<DFMChangeCurrentUrlEvent>(this, fileInfo->goToUrlWhenDeleted(), window());
}

void DFileView::freshView()
{
    model()->refresh(rootUrl());
}

void DFileView::loadViewState(const DUrl& url)
{
    Q_D(DFileView);
    ViewState viewState = viewStatesManager->viewstate(url);

    if(viewState.isValid()){
        switchViewMode(viewState.viewMode);
        setSortRoles(viewState.sortRole, viewState.sortOrder);
        if(viewState.viewMode != ListMode)
            d->statusBar->scalingSlider()->setValue(viewState.iconSize);
    } else{
        statusBar()->scalingSlider()->setValue(globalSetting->iconSizeIndex());
        switchViewMode(d->defaultViewMode);
    }

    //update list header view property when current view mode is list mode
    if(d->currentViewMode == ListMode){
        updateListHeaderViewProperty();
    }
}

void DFileView::saveViewState()
{
    //filter url that we are not interesting on
    const DUrl& url = rootUrl();
    if(url.isSearchFile() || url.isEmpty()
            || !url.isValid() || url.isComputerFile()){
        return;
    }

    ViewState viewState;
    QPair<int, Qt::SortOrder> roles = getSortRoles();
    viewState.viewMode = viewMode();
    viewState.iconSize = statusBar()->scalingSlider()->value();
    viewState.sortRole = roles.first;
    viewState.sortOrder = roles.second;

    viewStatesManager->saveViewState(url, viewState);
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

    d->verticalScrollBar->setFixedSize(8, event->size().height());
    if(d->currentViewMode == IconMode){
        d->verticalScrollBar->move(event->size().width(), 0);
    } else if (d->currentViewMode == ListMode){
        d->verticalScrollBar->move(event->size().width(), d->headerView->height());
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

        if (!fileInfo || !fileInfo->isWritable()) {
            event->ignore();

            return;
        }
    }

    preproccessDropEvent(event);

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

    d->dragMoveHoverIndex = d->fileViewHelper->isEmptyArea(event->pos()) ? QModelIndex() : indexAt(event->pos());

    if (d->dragMoveHoverIndex.isValid()) {
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(d->dragMoveHoverIndex);

        if (fileInfo) {
            if (!fileInfo->canDrop()) {
                d->dragMoveHoverIndex = QModelIndex();
            } else if(!fileInfo->supportedDropActions().testFlag(event->dropAction())) {
                d->dragMoveHoverIndex = QModelIndex();

                update();
                return event->ignore();
            }
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

    preproccessDropEvent(event);

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

        if (isSelected(index))
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

        break;
    default:
        index = DListView::moveCursor(cursorAction, modifiers);
        break;
    }

    if (index.isValid()) {
        d->lastCursorIndex = index;

        return index;
    }

    d->lastCursorIndex = current;

    return current;
}

void DFileView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    for (const QModelIndex &index : selectedIndexes()) {
        if (index.parent() != parent)
            continue;

        if (index.row() >= start && index.row() <= end) {
            selectionModel()->select(index, QItemSelectionModel::Clear);
        }
    }

    DListView::rowsAboutToBeRemoved(parent, start, end);
}

void DFileView::rowsInserted(const QModelIndex &parent, int start, int end)
{
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
    if(e->type() == QEvent::KeyPress){
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

    return DListView::event(e);
}

void DFileView::onShowHiddenFileChanged()
{
    QDir::Filters filters;
    if (globalSetting->isShowedHiddenOnView())
        filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
    else
        filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
    model()->setFilters(filters);
}

void DFileView::initDelegate()
{
    D_D(DFileView);

    setItemDelegate(new DIconItemDelegate(d->fileViewHelper));
    d->statusBar->scalingSlider()->setValue(itemDelegate()->iconSizeLevel());
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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    DListView::setSelectionRectVisible(true);

    d->displayAsActionGroup = new QActionGroup(this);
    d->sortByActionGroup = new QActionGroup(this);
    d->openWithActionGroup = new QActionGroup(this);
    d->fileViewHelper = new FileViewHelper(this);

    d->statusBar = new DStatusBar(this);
    d->statusBar->scalingSlider()->setPageStep(1);
    d->statusBar->scalingSlider()->setTickInterval(1);

    addFooterWidget(d->statusBar);

    if (DFMGlobal::isRootUser()) {
        d->statusBar->setStyleSheet("DStatusBar{"
                                       "background: #f9f9fa;"
                                    "}");
    }

    d->verticalScrollBar = verticalScrollBar();
    d->verticalScrollBar->setParent(this);

    QAction *icon_view_mode_action = new QAction(this);
    QIcon icon_view_mode_icon;
    icon_view_mode_icon.addFile(":/icons/images/icons/icon_view_normal.png");
    icon_view_mode_icon.addFile(":/icons/images/icons/icon_view_hover.png", QSize(), QIcon::Active);
    icon_view_mode_icon.addFile(":/icons/images/icons/icon_view_checked.png", QSize(), QIcon::Normal, QIcon::On);
    icon_view_mode_action->setIcon(icon_view_mode_icon);
    icon_view_mode_action->setCheckable(true);
    icon_view_mode_action->setChecked(true);

    QAction *list_view_mode_action = new QAction(this);
    QIcon list_view_mode_icon;
    list_view_mode_icon.addFile(":/icons/images/icons/list_view_normal.png");
    list_view_mode_icon.addFile(":/icons/images/icons/list_view_hover.png", QSize(), QIcon::Active);
    list_view_mode_icon.addFile(":/icons/images/icons/list_view_checked.png", QSize(), QIcon::Normal, QIcon::On);
    list_view_mode_action->setIcon(list_view_mode_icon);
    list_view_mode_action->setCheckable(true);

    d->toolbarActionGroup = new QActionGroup(this);
    d->toolbarActionGroup->addAction(icon_view_mode_action);
    d->toolbarActionGroup->addAction(list_view_mode_action);
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

    connect(fileSignalManager, &FileSignalManager::requestChangeIconSizeBySizeIndex, this, &DFileView::setIconSizeBySizeIndex);
    connect(fileSignalManager, &FileSignalManager::showHiddenOnViewChanged, this, &DFileView::onShowHiddenFileChanged);
    connect(fileSignalManager, &FileSignalManager::requestFreshAllFileView, this, &DFileView::freshView);

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
}

void DFileView::increaseIcon()
{
    D_D(DFileView);

    int iconSizeLevel = itemDelegate()->increaseIcon();

    if (iconSizeLevel >= 0) {
        QSignalBlocker blocker(d->statusBar->scalingSlider());

        Q_UNUSED(blocker)
        d->statusBar->scalingSlider()->setValue(iconSizeLevel);
    }
}

void DFileView::decreaseIcon()
{
    D_D(DFileView);

    int iconSizeLevel = itemDelegate()->decreaseIcon();

    if (iconSizeLevel >= 0) {
        QSignalBlocker blocker(d->statusBar->scalingSlider());

        Q_UNUSED(blocker)
        d->statusBar->scalingSlider()->setValue(iconSizeLevel);
    }
}

void DFileView::openIndex(const QModelIndex &index)
{
    D_D(DFileView);

    const DUrl &url = model()->getUrlByIndex(index);

    DFMOpenUrlEvent::DirOpenMode mode = globalSetting->isAllwayOpenOnNewWindow() ? DFMOpenUrlEvent::ForceOpenNewWindow : DFMOpenUrlEvent::OpenInCurrentWindow;

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

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, fileUrl);

    if (!info) {
        qDebug() << "This scheme isn't support";
        return false;
    }

    fileUrl = info->fileUrl();

    if(info->canRedirectionFileUrl()) {
        const DUrl old_url = fileUrl;

        fileUrl = info->redirectedFileUrl();

        qDebug() << "url redirected, from:" << old_url << "to:" << fileUrl;
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

    const QPair<int, int> &sort_config = FMStateManager::SortStates.value(fileUrl, QPair<int, int>(DFileSystemModel::FileDisplayNameRole, Qt::AscendingOrder));

    model()->setSortRole(sort_config.first, (Qt::SortOrder)sort_config.second);

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

        d->headerView = Q_NULLPTR;
    }
}

void DFileView::clearSelection()
{
    QListView::clearSelection();
}

void DFileView::setContentLabel(const QString &text)
{
    D_D(DFileView);

    if (text.isEmpty() && d->contentLabel) {
        d->contentLabel->deleteLater();
        d->contentLabel = Q_NULLPTR;

        return;
    }

    if (!d->contentLabel) {
        d->contentLabel = new QLabel(this);

        d->contentLabel->show();
        d->contentLabel.setCenterIn(this);
        d->contentLabel->setObjectName("contentLabel");
        d->contentLabel->setStyleSheet(this->styleSheet());
        d->contentLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    }

    d->contentLabel->setText(text);
    d->contentLabel->adjustSize();
}

void DFileView::setMenuActionWhitelist(const QSet<MenuAction> &actionList)
{
    Q_D(DFileView);

    d->menuWhitelist = actionList;
}

void DFileView::setMenuActionBlacklist(const QSet<MenuAction> &actionList)
{
    Q_D(DFileView);

    d->menuBlacklist = actionList;
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

    switch (mode) {
    case IconMode: {
        clearHeardView();
        d->columnRoles.clear();
        setOrientation(QListView::LeftToRight, true);
        setSpacing(ICON_VIEW_SPACING);
        setItemDelegate(new DIconItemDelegate(d->fileViewHelper));
        d->statusBar->scalingSlider()->show();
        itemDelegate()->setIconSizeByIconSizeLevel(d->statusBar->scalingSlider()->value());
        d->toolbarActionGroup->actions().first()->setChecked(true);
        break;
    }
    case ListMode: {
        setItemDelegate(new DListItemDelegate(d->fileViewHelper));

        if(!d->headerView) {
            d->headerView = new QHeaderView(Qt::Horizontal);

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
                    this, &DFileView::sort);
            connect(d->headerView, &QHeaderView::customContextMenuRequested,
                    this, &DFileView::popupHeaderViewContextMenu);

            d->headerView->setAttribute(Qt::WA_TransparentForMouseEvents, model()->state() == DFileSystemModel::Busy);
        }

        addHeaderWidget(d->headerView);

        setOrientation(QListView::TopToBottom, false);
        setSpacing(LIST_VIEW_SPACING);
        d->statusBar->scalingSlider()->hide();
        d->toolbarActionGroup->actions().at(1)->setChecked(true);
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

        QAction *currentViewModeAction = 0;

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

        QAction *action = sortBySubMenu->actionAt(model()->sortColumn());

        if (action)
            action->setChecked(true);
    }


    DFileMenuManager::loadEmptyAreaPluginMenu(menu, rootUrl());
    DFileMenuManager::loadEmptyAreaExtensionMenu(menu, rootUrl());

    if (!menu){
        return;
    }

    menu->setEventData(rootUrl(), selectedUrls(), windowId(), this);
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

    if(list.size() == 1){
        if (!info->isReadable() && !info->isSymLink())
            disableList << MenuAction::Copy;

        if (!info->isWritable() && !info->isFile() && !info->isSymLink())
            disableList << MenuAction::Delete;

        if (!indexFlags.testFlag(Qt::ItemIsEditable))
            disableList << MenuAction::Rename ;
    }

    menu = DFileMenuManager::createNormalMenu(info->fileUrl(), list, disableList, unusedList, windowId());

    if (!menu){
        return;
    }

    menu->setEventData(rootUrl(), selectedUrls(), windowId(), this);
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
    d->headerView->setSectionResizeMode(QHeaderView::Fixed);
    d->headerView->setDefaultSectionSize(DEFAULT_HEADER_SECTION_WIDTH);
    d->headerView->setMinimumSectionSize(DEFAULT_HEADER_SECTION_WIDTH);
    d->headerView->setSortIndicator(model()->sortColumn(), model()->sortOrder());

    d->columnRoles.clear();

    for (int i = 0; i < d->headerView->count(); ++i) {
        d->columnRoles << model()->columnToRole(i);

        int column_width = model()->columnWidth(i);

        if (column_width >= 0) {
            d->headerView->resizeSection(i, column_width + COLUMU_PADDING * 2);
        } else {
            d->headerView->setSectionResizeMode(i, QHeaderView::Stretch);
        }

        const QString &column_name = model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();

        if (!d->columnForRoleHiddenMap.contains(column_name)) {
            d->headerView->setSectionHidden(i, !model()->columnDefaultVisibleForRole(model()->columnToRole(i)));
        } else {
            d->headerView->setSectionHidden(i, d->columnForRoleHiddenMap.value(column_name));
        }
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
    d->headerView->setSectionResizeMode(QHeaderView::Fixed);
    d->headerView->setSectionResizeMode(0, QHeaderView::Stretch);
    d->headerView->setDefaultSectionSize(DEFAULT_HEADER_SECTION_WIDTH);
    d->headerView->setMinimumSectionSize(DEFAULT_HEADER_SECTION_WIDTH);

    d->columnRoles.clear();
    d->columnRoles << model()->columnToRole(0);
}

void DFileView::updateColumnWidth()
{
    D_D(DFileView);

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

void DFileView::popupHeaderViewContextMenu(const QPoint &/*pos*/)
{
    D_D(DFileView);

    QMenu *menu = new QMenu();

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
}

void DFileView::updateContentLabel()
{
    if (model()->state() != DFileSystemModel::Idle)
        return;

    int count = this->count();
    const DUrl &currentUrl = this->rootUrl();

    if (count <= 0) {
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(this, currentUrl);

        if (fileInfo)
            setContentLabel(fileInfo->subtitleForEmptyFloder());
    } else {
        setContentLabel(QString());
    }
}

void DFileView::preproccessDropEvent(QDropEvent *event) const
{
    if (event->source() == this && !DFMGlobal::keyCtrlIsPressed()) {
        event->setDropAction(Qt::MoveAction);
    } else {
        DAbstractFileInfoPointer info = model()->fileInfo(indexAt(event->pos()));

        if (!info)
            info = model()->fileInfo(rootIndex());

        if (info && !info->supportedDropActions().testFlag(event->dropAction())) {
            QList<Qt::DropAction> actions;

            actions.reserve(3);
            actions << Qt::CopyAction << Qt::MoveAction << Qt::LinkAction;

            for (Qt::DropAction action : actions) {
                if (event->possibleActions().testFlag(action) && info->supportedDropActions().testFlag(action)) {
                    event->setDropAction(action);

                    break;
                }
            }
        }
    }
}

void DFileView::refresh()
{
    model()->refresh();
}

int DFileViewPrivate::iconModeColumnCount(int itemWidth) const
{
    Q_Q(const DFileView);

    int frameAroundContents = 0;
    if (q->style()->styleHint(QStyle::SH_ScrollView_FrameOnlyAroundContents))
        frameAroundContents = q->style()->pixelMetric(QStyle::PM_DefaultFrameWidth) * 2;

    int horizontalMargin = q->verticalScrollBarPolicy()==Qt::ScrollBarAsNeeded
            ? q->style()->pixelMetric(QStyle::PM_ScrollBarExtent, 0, q->verticalScrollBar()) + frameAroundContents
            : 0;

    int contentWidth = q->maximumViewportSize().width();

    if (itemWidth <= 0)
        itemWidth = q->itemSizeHint().width() + q->spacing() * 2;

    return (contentWidth - horizontalMargin - 1) / itemWidth;
}
