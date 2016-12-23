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
#include "app/define.h"
#include "app/filesignalmanager.h"

#include "interfaces/dfmglobal.h"
#include "interfaces/diconitemdelegate.h"
#include "interfaces/dlistitemdelegate.h"

#include "controllers/appcontroller.h"
#include "dfileservices.h"
#include "controllers/fmstatemanager.h"
#include "controllers/pathmanager.h"

#include "models/dfileselectionmodel.h"
#include "dfilesystemmodel.h"

#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"

#include "widgets/singleton.h"
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

    DFileView::ViewMode defaultViewMode = DFileView::IconMode;
    DFileView::ViewMode currentViewMode = DFileView::IconMode;

    QRect selectedGeometry;
    QRect drawSelectionGeometry;
    bool enableDrawSelectionRect = false;
    bool selectionRectVisible = true;
    bool dragEnabled = true;

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

    /// file operator function filter
    DFileService::FileOperatorTypes fileOperatorWhitelist;
    DFileService::FileOperatorTypes fileOperatorBlacklist;

    QSet<DFileView::SelectionMode> enabledSelectionModes;

    FileViewHelper *fileViewHelper;

    Q_DECLARE_PUBLIC(DFileView)
};

DFileView::DFileView(QWidget *parent)
    : DListView(parent)
    , d_ptr(new DFileViewPrivate(this))
{
    D_THEME_INIT_WIDGET(DFileView);
    D_D(DFileView);

    d_ptr->enabledSelectionModes << NoSelection << SingleSelection
                                 << MultiSelection << ExtendedSelection
                                 << ContiguousSelection;

    initUI();
    initDelegate();
    initModel();
    initConnects();
}

DFileView::~DFileView()
{
    disconnect(this, &DFileView::rowCountChanged, this, &DFileView::onRowCountChanged);
    disconnect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &DFileView::updateStatusBar);
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
    DUrlList list;

    for(const QModelIndex &index : selectedIndexes()) {
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

int DFileView::windowId() const
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

int DFileView::getSortRoles() const
{
    return model()->sortRole();
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

bool DFileView::dragEnabled() const
{
    Q_D(const DFileView);

    return d->dragEnabled && DListView::dragEnabled();
}

bool DFileView::cd(const DUrl &url)
{
    if (url.isEmpty())
        return false;

    itemDelegate()->hideAllIIndexWidget();

    clearSelection();

    if (!url.isSearchFile()){
        setFocus();
    }

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

    if (trigger == SelectedClicked) {
        QStyleOptionViewItem option = viewOptions();

        option.rect = visualRect(index);

        const QRect &file_name_rect = itemDelegate()->fileNameRect(option, index);

        if (!file_name_rect.contains(static_cast<QMouseEvent*>(event)->pos()))
            return false;
    }

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

    if(mode != d->defaultViewMode)
        d->defaultViewMode = mode;

    switchViewMode(mode);
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
    d->oldCurrentUrl = model()->getUrlByIndex(currentIndex());

    clearSelection();
    model()->sort();
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
}

QSet<QAbstractItemView::SelectionMode> DFileView::enabledSelectionModes() const
{
    Q_D(const DFileView);

    return d->enabledSelectionModes;
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

    const DUrlList& urls = selectedUrls();

    DFMEvent fmevent;
    fmevent << urls;
    fmevent << DFMEvent::FileView;
    fmevent << windowId();
    fmevent << rootUrl();

    switch(type){
        case MenuAction::IconView:
            emit fileSignalManager->requestChangeIconViewMode(fmevent);
            break;
        case MenuAction::ListView:
            emit fileSignalManager->requestChangeListViewMode(fmevent);
            break;
    case MenuAction::ExtendView:
            emit fileSignalManager->requestChangeExtendViewMode(fmevent);
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
    fileService->openFileByApp(fileUrl, app);
}

void DFileView::onRowCountChanged()
{
#ifndef CLASSICAL_SECTION
    static_cast<DFileSelectionModel*>(selectionModel())->m_selectedList.clear();
#endif

    updateStatusBar();
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

        event->accept();
    } else {
        verticalScrollBar()->setSliderPosition(verticalScrollBar()->sliderPosition() - event->angleDelta().y());
    }
}

void DFileView::keyPressEvent(QKeyEvent *event)
{
    D_D(DFileView);

    const DUrlList& urls = selectedUrls();

    DFMEvent fmevent;

    fmevent << urls;
    fmevent << DFMEvent::FileView;
    fmevent << windowId();
    fmevent << rootUrl();

    switch (event->modifiers()) {
    case Qt::NoModifier:
    case Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (!itemDelegate()->editingIndex().isValid()) {
                appController->actionOpen(fmevent);

                return;
            }

            break;
        case Qt::Key_Backspace:
            cdUp();

            return;
        case Qt::Key_F1:
            appController->actionHelp(fmevent);

            return;
        case Qt::Key_F5:
            model()->refresh();

            return;
        case Qt::Key_Delete:
            fileService->moveToTrash(fmevent);
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
        case Qt::Key_F:
            appController->actionctrlF(fmevent);

            return;
        case Qt::Key_L:
            appController->actionctrlL(fmevent);

            return;
        case Qt::Key_N:
            appController->actionNewWindow(fmevent);

            return;
        case Qt::Key_H:
            d->preSelectionUrls = urls;

            itemDelegate()->hideAllIIndexWidget();
            clearSelection();

            model()->toggleHiddenFiles(fmevent.fileUrl());

            return;
        case Qt::Key_I:
            if (fmevent.fileUrlList().isEmpty())
                fmevent << (DUrlList() << fmevent.fileUrl());

            appController->actionProperty(fmevent);

            return;
        case Qt::Key_Up:
            cdUp();

            return;
        case Qt::Key_Down:
            appController->actionOpen(fmevent);

            return;
        case Qt::Key_Left:
            appController->actionBack(fmevent);

            return;
        case Qt::Key_Right:
            appController->actionForward(fmevent);

            return;
        case Qt::Key_T:
            emit fileSignalManager->requestOpenInNewTab(fmevent);
            return;
        case Qt::Key_W:
            emit fileSignalManager->requestCloseCurrentTab(fmevent);
            return;
        case Qt::Key_Tab:
            emit DFileView::requestActiveNextTab();
            return;
        default: break;
        }

        break;
    case Qt::ShiftModifier:
        if (event->key() == Qt::Key_Delete) {
            if (fmevent.fileUrlList().isEmpty())
                return;

            fileService->deleteFiles(fmevent);

            return;
        } else if (event->key() == Qt::Key_T) {
            appController->actionOpenInTerminal(fmevent);

            return;
        }

        break;
    case Qt::ControlModifier | Qt::ShiftModifier:
        if (event->key() == Qt::Key_Backtab){
            emit DFileView::requestActivePreviousTab();
            return;
        } if (event->key() == Qt::Key_N) {
            if (itemDelegate()->editingIndex().isValid())
                return;

            clearSelection();
            appController->actionNewFolder(fmevent);

            return;
        } if (event->key() == Qt::Key_Question) {
            appController->actionShowHotkeyHelp(fmevent);

            return;
        }

        break;
    case Qt::AltModifier:
        switch (event->key()) {
        case Qt::Key_Up:
            cdUp();

            return;
        case Qt::Key_Down:
            appController->actionOpen(fmevent);

            return;
        case Qt::Key_Left:
            appController->actionBack(fmevent);

            return;
        case Qt::Key_Right:
            appController->actionForward(fmevent);

            return;
        case Qt::Key_Home:
            cd(DUrl::fromLocalFile(QDir::homePath()));

            return;
        default: break;
        }

        break;
    default: break;
    }

    DListView::keyPressEvent(event);
}

void DFileView::showEvent(QShowEvent *event)
{
    DListView::showEvent(event);

    setFocus();
}

void DFileView::mousePressEvent(QMouseEvent *event)
{
    D_D(DFileView);

    switch (event->button()) {
    case Qt::BackButton: {
        DFMEvent event;

        event << this->windowId();
        event << DFMEvent::FileView;

        fileSignalManager->requestBack(event);

        break;
    }
    case Qt::ForwardButton: {
        DFMEvent event;

        event << this->windowId();
        event << DFMEvent::FileView;

        fileSignalManager->requestForward(event);

        break;
    }
    case Qt::LeftButton: {
        bool isEmptyArea = d->fileViewHelper->isEmptyArea(event->pos());

        d->dragEnabled = !isEmptyArea;

        if (isEmptyArea) {
            if (!DFMGlobal::keyCtrlIsPressed()) {
                itemDelegate()->hideNotEditingIndexWidget();
                clearSelection();
            }

            if (canShowSelectionRect()) {
                d->enableDrawSelectionRect = true;
            }

            d->selectedGeometry.setTop(event->pos().y() + verticalOffset());
            d->selectedGeometry.setLeft(event->pos().x() + horizontalOffset());

            connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &DFileView::updateSelectionRect);
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

    if (dragEnabled() || event->buttons() != Qt::LeftButton
            || selectionMode() == NoSelection || selectionMode() == SingleSelection) {
        return DListView::mouseMoveEvent(event);
    }

    updateSelectionRect();
    doAutoScroll();
}

void DFileView::mouseReleaseEvent(QMouseEvent *event)
{
    D_D(DFileView);

    d->dragMoveHoverIndex = QModelIndex();

    disconnect(verticalScrollBar(), &QScrollBar::valueChanged, this, &DFileView::updateSelectionRect);

    if (d->mouseLastPressedIndex.isValid() && DFMGlobal::keyCtrlIsPressed()) {
        if (d->mouseLastPressedIndex == indexAt(event->pos()))
            selectionModel()->select(d->mouseLastPressedIndex, QItemSelectionModel::Deselect);
    }

    if (dragEnabled())
        return DListView::mouseReleaseEvent(event);

    d->enableDrawSelectionRect = false;
    viewport()->update(d->drawSelectionGeometry);
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

void DFileView::updateStatusBar()
{
    Q_D(DFileView);
    if (model()->state() != DFileSystemModel::Idle)
        return;

    DFMEvent event;
    event << windowId();
    event << selectedUrls();
    int count = selectedIndexCount();

    if (count == 0){
        d->statusBar->itemCounted(event, this->count());
    }else{
        d->statusBar->itemSelected(event, count);
    }
}

void DFileView::onRootUrlDeleted(const DUrl &rootUrl)
{
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(rootUrl);

    if (!fileInfo)
        return;

    DFMEvent event;

    event << windowId();
    event << DFMEvent::FileView;
    event << fileInfo->goToUrlWhenDeleted();

    fileSignalManager->requestChangeCurrentUrl(event);
}

void DFileView::focusInEvent(QFocusEvent *event)
{
    Q_D(const DFileView);

    DListView::focusInEvent(event);
    itemDelegate()->commitDataAndCloseActiveEditor();

    /// set menu actions filter
    DFileMenuManager::setActionWhitelist(d->menuWhitelist);
    DFileMenuManager::setActionBlacklist(d->menuBlacklist);

    /// set file operator function filter
    DFileService::instance()->setFileOperatorWhitelist(d->fileOperatorWhitelist);
    DFileService::instance()->setFileOperatorBlacklist(d->fileOperatorBlacklist);
}

void DFileView::resizeEvent(QResizeEvent *event)
{
    DListView::resizeEvent(event);

    updateHorizontalOffset();

    if (itemDelegate()->editingIndex().isValid())
        doItemsLayout();

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
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(url);

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

    d->dragMoveHoverIndex = indexAt(event->pos());

    if (d->dragMoveHoverIndex.isValid()) {
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(d->dragMoveHoverIndex);

        if (fileInfo) {
            if (!fileInfo->isDir()) {
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
        QModelIndex index = indexAt(event->pos());

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
        QRect tmp_rect;

        tmp_rect.setCoords(qMin(rect.left(), rect.right()), qMin(rect.top(), rect.bottom()),
                           qMax(rect.left(), rect.right()), qMax(rect.top(), rect.bottom()));

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

void DFileView::paintEvent(QPaintEvent *event)
{
    DListView::paintEvent(event);

    Q_D(DFileView);

    if (d->enableDrawSelectionRect) {
        QPainter pa(viewport());

        pa.fillRect(d->drawSelectionGeometry, QColor(43, 167, 248, 0.3 * 255));
        pa.setPen(QColor(30, 126, 255, 0.2 * 255));
        pa.drawRect(d->drawSelectionGeometry.adjusted(0, 0, -1, -1));
    }
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

    DListView::setSelectionRectVisible(false);

    d->displayAsActionGroup = new QActionGroup(this);
    d->sortByActionGroup = new QActionGroup(this);
    d->openWithActionGroup = new QActionGroup(this);
    d->fileViewHelper = new FileViewHelper(this);

    d->statusBar = new DStatusBar(this);
    d->statusBar->scalingSlider()->setPageStep(1);
    d->statusBar->scalingSlider()->setTickInterval(1);

    addFooterWidget(d->statusBar);
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

    connect(this, &DFileView::doubleClicked,
            this, [this] (const QModelIndex &index) {
        if (!DFMGlobal::keyCtrlIsPressed() && !DFMGlobal::keyShiftIsPressed())
            openIndex(index);
    }, Qt::QueuedConnection);

    connect(this, &DFileView::rowCountChanged, this, &DFileView::onRowCountChanged, Qt::QueuedConnection);

    connect(d->displayAsActionGroup, &QActionGroup::triggered, this, &DFileView::dislpayAsActionTriggered);
    connect(d->sortByActionGroup, &QActionGroup::triggered, this, &DFileView::sortByActionTriggered);
    connect(d->openWithActionGroup, &QActionGroup::triggered, this, &DFileView::openWithActionTriggered);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &DFileView::updateStatusBar);
    connect(model(), &DFileSystemModel::dataChanged, this, &DFileView::handleDataChanged);
    connect(model(), &DFileSystemModel::stateChanged, this, &DFileView::onModelStateChanged);
    connect(model(), &DFileSystemModel::rootUrlDeleted, this, &DFileView::onRootUrlDeleted);

    connect(this, &DFileView::iconSizeChanged, this, &DFileView::updateHorizontalOffset, Qt::QueuedConnection);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &DFileView::updateModelActiveIndex);
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

    DFMEvent event;
    DUrl url = model()->getUrlByIndex(index);
    DUrlList urls;
    urls << url;
    event << url;
    event << urls;
    event << DFMEvent::FileView;
    event << windowId();
    DFileService::instance()->openUrl(event);
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

    DUrl fileUrl = url;

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(fileUrl);

    if (!info){
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
    } else if (const DAbstractFileInfoPointer &current_file_info = DFileService::instance()->createFileInfo(rootUrl)) {
        QList<DUrl> ancestors;

        if (current_file_info->isAncestorsUrl(fileUrl, &ancestors)) {
            d->preSelectionUrls << (ancestors.count() > 1 ? ancestors.at(1) : rootUrl);
        }
    }

    QModelIndex index = model()->setRootUrl(fileUrl);

    setRootIndex(index);

    if (!model()->canFetchMore(index)) {
        updateContentLabel();
    }

    const QPair<int, int> &sort_config = FMStateManager::SortStates.value(fileUrl, QPair<int, int>(DFileSystemModel::FileDisplayNameRole, Qt::AscendingOrder));

    model()->setSortRole(sort_config.first, (Qt::SortOrder)sort_config.second);

    if (d->currentViewMode == ListMode) {
        updateListHeaderViewProperty();
    }

    if(info) {
        ViewModes modes = (ViewModes)info->supportViewMode();

        DFileManagerWindow *fmWindow = qobject_cast<DFileManagerWindow*>(window());

        //view mode support handler
        if (fmWindow) {
            if (testViewMode(modes, ListMode))
                fmWindow->getToolBar()->setListModeButtonEnabled(true);
            else{
                fmWindow->getToolBar()->setListModeButtonEnabled(false);
            }

            if(testViewMode(modes, IconMode))
                fmWindow->getToolBar()->setIconModeButtonEnabled(true);
            else
                fmWindow->getToolBar()->setIconModeButtonEnabled(false);
        }

        if(!testViewMode(modes, d->defaultViewMode)) {
            if(testViewMode(modes, IconMode)) {
                switchViewMode(IconMode);
            } else if(testViewMode(modes, ListMode)) {
                switchViewMode(ListMode);
            } else if(testViewMode(modes, ExtendMode)) {
                switchViewMode(ExtendMode);
            }
        } else {
            switchViewMode(d->defaultViewMode);
        }
    }
    emit rootUrlChanged(fileUrl);

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

void DFileView::setSelectionRectVisible(bool visible)
{
    D_D(DFileView);

    d->selectionRectVisible = visible;
}

bool DFileView::isSelectionRectVisible() const
{
    D_DC(DFileView);

    return d->selectionRectVisible;
}

bool DFileView::canShowSelectionRect() const
{
    D_DC(DFileView);

    return d->selectionRectVisible && selectionMode() != NoSelection && selectionMode() != SingleSelection;
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

void DFileView::setFileOperatorWhitelist(int fileOperatorFlags)
{
    Q_D(DFileView);

    d->fileOperatorWhitelist = (DFileService::FileOperatorTypes)fileOperatorFlags;
}

void DFileView::setFileOperatorBlacklist(int fileOperatorFlags)
{
    Q_D(DFileView);

    d->fileOperatorBlacklist = (DFileService::FileOperatorTypes)fileOperatorFlags;
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

    DUrlList urls;
    urls.append(rootUrl());

    DFMEvent event;
    event << rootUrl();
    event << urls;
    event << windowId();
    event << DFMEvent::FileView;
    menu->setEvent(event);
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

    const DAbstractFileInfoPointer &info = model()->fileInfo(index);
    QSet<MenuAction> disableList;
    QSet<MenuAction> unusedList;

    if(list.size() == 1){
        if(!info->isReadable())
            disableList << MenuAction::Copy;

        if(!info->isWritable() && !info->isFile())
            disableList << MenuAction::Delete;

        if (!indexFlags.testFlag(Qt::ItemIsEditable))
                disableList << MenuAction::Rename ;
    }

    menu = DFileMenuManager::createNormalMenu(info->fileUrl(), list, disableList, unusedList, windowId());

    if (!menu){
        return;
    }

    DFMEvent event;

    event << info->fileUrl();
    event << list;
    event << windowId();
    event << DFMEvent::FileView;

    menu->setEvent(event);
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

    DFMEvent event;

    event << windowId();
    event << rootUrl();

    emit fileSignalManager->loadingIndicatorShowed(event, state == DFileSystemModel::Busy);

    if (state == DFileSystemModel::Busy) {
        setContentLabel(QString());

        if (d->headerView) {
            d->headerView->setAttribute(Qt::WA_TransparentForMouseEvents);
        }
    } else if (state == DFileSystemModel::Idle) {
        if (!d->preSelectionUrls.isEmpty()) {
            const QModelIndex &index = model()->index(d->preSelectionUrls.first());

            setCurrentIndex(index);
            scrollTo(index, PositionAtTop);
        }

        for (const DUrl &url : d->preSelectionUrls) {
            selectionModel()->select(model()->index(url), QItemSelectionModel::Select);
        }

        d->preSelectionUrls.clear();

        updateStatusBar();
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
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(currentUrl);

        if (fileInfo)
            setContentLabel(fileInfo->subtitleForEmptyFloder());
    } else {
        setContentLabel(QString());
    }
}

void DFileView::updateSelectionRect()
{
    D_D(DFileView);

    if (dragEnabled() || selectionMode() == NoSelection || selectionMode() == SingleSelection)
        return;

    QPoint pos = mapFromGlobal(QCursor::pos());

    if (d->enableDrawSelectionRect) {
        QRect rect;
        QPoint pressedPos = viewport()->mapToParent(d->selectedGeometry.topLeft());

        pressedPos.setX(pressedPos.x() - horizontalOffset());
        pressedPos.setY(pressedPos.y() - verticalOffset());

        rect.setCoords(qMin(pressedPos.x(), pos.x()), qMin(pressedPos.y(), pos.y()),
                       qMax(pos.x(), pressedPos.x()), qMax(pos.y(), pressedPos.y()));

        viewport()->update(d->drawSelectionGeometry);
        d->drawSelectionGeometry = rect;
    }

    pos = viewport()->mapFromParent(pos);

    d->selectedGeometry.setBottom(pos.y() + verticalOffset());
    d->selectedGeometry.setRight(pos.x() + horizontalOffset());

    setSelection(d->selectedGeometry, QItemSelectionModel::Current|QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect);
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
