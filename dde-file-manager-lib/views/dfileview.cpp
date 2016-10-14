#include "dfileview.h"
#include "fileitem.h"
#include "filemenumanager.h"
#include "dfilemenu.h"
#include "windowmanager.h"
#include "dstatusbar.h"
#include "fileviewhelper.h"
#include "app/define.h"
#include "app/filemanagerapp.h"
#include "app/filesignalmanager.h"

#include "interfaces/dfmglobal.h"
#include "interfaces/diconitemdelegate.h"
#include "interfaces/dlistitemdelegate.h"

#include "controllers/appcontroller.h"
#include "fileservices.h"
#include "controllers/fmstatemanager.h"
#include "controllers/pathmanager.h"

#include "models/dfileselectionmodel.h"
#include "dfilesystemmodel.h"

#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"

#include "widgets/singleton.h"

#include <dthememanager.h>
#include <dscrollbar.h>
#include <anchors.h>

#include <QLineEdit>
#include <QTextEdit>
#include <QUrlQuery>
#include <QActionGroup>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMimeData>

DWIDGET_USE_NAMESPACE

#define ICON_VIEW_SPACING 5
#define LIST_VIEW_SPACING 1

#define DEFAULT_HEADER_SECTION_WIDTH 140

class DFileViewPrivate
{
public:
    DFileViewPrivate(DFileView *qq)
        : q_ptr(qq) {}

    DFileView *q_ptr;

    FileMenuManager* fileMenuManager;
    QHeaderView *headerView = Q_NULLPTR;
    DStatusBar* statusBar=NULL;

    QActionGroup* displayAsActionGroup;
    QActionGroup* sortByActionGroup;
    QActionGroup* openWithActionGroup;

    QList<int> columnRoles;

    DFileView::ViewMode defaultViewMode = DFileView::IconMode;
    DFileView::ViewMode currentViewMode = DFileView::IconMode;

    QRect selectedGeometry;
    QWidget *selectionRectWidget = Q_NULLPTR;
    bool selectionRectVisible = true;

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

    DSlider *scalingSlider = NULL;
    FileViewHelper *fileViewHelper;

    Q_DECLARE_PUBLIC(DFileView)
};

DFileView::DFileView(QWidget *parent)
    : DListView(parent)
    , d_ptr(new DFileViewPrivate(this))
{
    D_THEME_INIT_WIDGET(DFileView);
    D_D(DFileView);

    d->displayAsActionGroup = new QActionGroup(this);
    d->sortByActionGroup = new QActionGroup(this);
    d->openWithActionGroup = new QActionGroup(this);
    d->fileViewHelper = new FileViewHelper(this);

    initDelegate();
    initUI();
    initModel();
    initConnects();
}

DFileView::~DFileView()
{
    disconnect(this, &DFileView::rowCountChanged, this, &DFileView::onRowCountChanged);
    disconnect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &DFileView::updateStatusBar);
}

void DFileView::initDelegate()
{
    D_D(DFileView);

    setItemDelegate(new DIconItemDelegate(d->fileViewHelper));
}

void DFileView::initUI()
{
    D_D(DFileView);

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
    setVerticalScrollBar(new DScrollBar);

    DListView::setSelectionRectVisible(false);

    d->selectionRectWidget = new QWidget(this);
    d->selectionRectWidget->hide();
    d->selectionRectWidget->resize(0, 0);
    d->selectionRectWidget->setObjectName("SelectionRect");
    d->selectionRectWidget->raise();
    d->selectionRectWidget->setAttribute(Qt::WA_TransparentForMouseEvents);

    d->statusBar = new DStatusBar(this);
    d->scalingSlider = d->statusBar->scalingSlider();
    d->scalingSlider->setPageStep(1);
    d->scalingSlider->setTickInterval(1);
    d->scalingSlider->setMinimum(itemDelegate()->minimumIconSizeLevel());
    d->scalingSlider->setMaximum(itemDelegate()->maximumIconSizeLevel());
    d->scalingSlider->setValue(itemDelegate()->iconSizeLevel());

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

    connect(this, &DFileView::viewModeChanged, this, &DFileView::handleViewModeChanged);
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

    connect(delegate, &DStyledItemDelegate::commitData, this, &DFileView::handleCommitData);
    connect(d->scalingSlider, &DSlider::valueChanged, delegate, &DStyledItemDelegate::setIconSizeByIconSizeLevel);
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

DUrl DFileView::currentUrl() const
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
    return orientation() == Qt::Vertical && isWrapping();
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
    if (!isIconViewMode())
        return 1;

    int itemWidth = itemSizeHint().width() + ICON_VIEW_SPACING * 2;

    return (width() - ICON_VIEW_SPACING * 2.9) / itemWidth;
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
    if (isIconViewMode()) {
        for (QModelIndex &index : itemDelegate()->hasWidgetIndexs()) {
            if (index == itemDelegate()->editingIndex())
                continue;

            QWidget *widget = indexWidget(index);

            if (widget->isVisible() && widget->geometry().contains(point)) {
                return index;
            }
        }
    }

    FileIconItem *item = qobject_cast<FileIconItem*>(itemDelegate()->editingIndexWidget());

    if (item) {
        QRect geometry = item->icon->geometry();

        geometry.moveTopLeft(geometry.topLeft() + item->pos());

        if (geometry.contains(point))
            return itemDelegate()->editingIndex();

        geometry = item->edit->geometry();
        geometry.moveTopLeft(geometry.topLeft() + item->pos());
        geometry.setTop(item->icon->y() + item->icon->height() + item->y());

        if (geometry.contains(point))
            return itemDelegate()->editingIndex();
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
        int column_count = (width() - ICON_VIEW_SPACING * 2.9) / item_width;
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
        int column_count = (width() - ICON_VIEW_SPACING * 2.9) / item_width;
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

        int column_count = (width() - spacing * 2.9) / item_width;
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
    if (url.isEmpty())
        return false;

    itemDelegate()->hideAllIIndexWidget();

    clearSelection();

    if (!url.isSearchFile()){
        setFocus();
    }

    return setCurrentUrl(url);
}

bool DFileView::cdUp()
{
    const AbstractFileInfoPointer &fileInfo = model()->fileInfo(rootIndex());

    const DUrl &oldCurrentUrl = currentUrl();
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

    clearSelection();

    for (const DUrl &url : list) {
        const QModelIndex &index = model()->index(url);

        if (index.isValid()) {
            selectionModel()->select(index, QItemSelectionModel::Select);
        }

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

void DFileView::sortByRole(int role)
{
    D_D(DFileView);

    Qt::SortOrder order = (model()->sortRole() == role && model()->sortOrder() == Qt::AscendingOrder)
                            ? Qt::DescendingOrder
                            : Qt::AscendingOrder;

    if (d->headerView) {
        d->headerView->setSortIndicator(model()->roleToColumn(role), order);
    } else {
        model()->setSortRole(role, order);
        model()->sort();
    }

    FMStateManager::cacheSortState(currentUrl(), role, order);
}

void DFileView::sortByColumn(int column)
{
    sortByRole(model()->columnToRole(column));
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

void DFileView::setFilters(QDir::Filters filters)
{
    model()->setFilters(filters);
}

void DFileView::dislpayAsActionTriggered(QAction *action)
{
    DAction* dAction = static_cast<DAction*>(action);
    dAction->setChecked(true);
    MenuAction type = (MenuAction)dAction->data().toInt();
    switch(type){
        case MenuAction::IconView:
            setViewMode(IconMode);
            break;
        case MenuAction::ListView:
            setViewMode(ListMode);
            break;
        default:
            break;
    }
}

void DFileView::sortByActionTriggered(QAction *action)
{
    D_DC(DFileView);

    DAction* dAction = static_cast<DAction*>(action);

    sortByColumn(d->sortByActionGroup->actions().indexOf(dAction));
}

void DFileView::openWithActionTriggered(QAction *action)
{
    DAction* dAction = static_cast<DAction*>(action);
    QString app = dAction->property("app").toString();
    QString url = dAction->property("url").toString();
    FileUtils::openFileByApp(url, app);
}

void DFileView::onRowCountChanged()
{
#ifndef CLASSICAL_SECTION
    static_cast<DFileSelectionModel*>(selectionModel())->m_selectedList.clear();
#endif

    updateStatusBar();
    updateContentLabel();
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

    FMEvent fmevent;

    fmevent << urls;
    fmevent << FMEvent::FileView;
    fmevent << windowId();
    fmevent << currentUrl();

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
            if (fmevent.fileUrlList().isEmpty()) {
                break;
            }

            if (fmevent.fileUrl().isTrashFile()) {
                fileService->deleteFiles(fmevent.fileUrlList(), fmevent);
            } else {
                fileService->moveToTrash(fmevent.fileUrlList());
            }

            break;
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
        default: break;
        }

        break;
    case Qt::ShiftModifier:
        if (event->key() == Qt::Key_Delete) {
            if (fmevent.fileUrlList().isEmpty())
                return;

            fileService->deleteFiles(fmevent.fileUrlList(), fmevent);

            return;
        } else if (event->key() == Qt::Key_T) {
            appController->actionOpenInTerminal(fmevent);

            return;
        }

        break;
    case Qt::ControlModifier | Qt::ShiftModifier:
        if (event->key() == Qt::Key_N) {
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
        FMEvent event;

        event << this->windowId();
        event << FMEvent::FileView;

        fileSignalManager->requestBack(event);

        break;
    }
    case Qt::ForwardButton: {
        FMEvent event;

        event << this->windowId();
        event << FMEvent::FileView;

        fileSignalManager->requestForward(event);

        break;
    }
    case Qt::LeftButton: {
        bool isEmptyArea = d->fileViewHelper->isEmptyArea(event->pos());

        setDragEnabled(!isEmptyArea);

        if (isEmptyArea) {
            if (!DFMGlobal::keyCtrlIsPressed()) {
                itemDelegate()->hideNotEditingIndexWidget();
                clearSelection();
            }

            if (canShowSelectionRect())
                d->selectionRectWidget->show();

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
    if (dragEnabled())
        return DListView::mouseMoveEvent(event);

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

    d->selectionRectWidget->resize(0, 0);
    d->selectionRectWidget->hide();
}

void DFileView::handleCommitData(QWidget *editor)
{
    if(!editor)
        return;

    const AbstractFileInfoPointer &fileInfo = model()->fileInfo(itemDelegate()->editingIndex());

    if(!fileInfo)
        return;

    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
    FileIconItem *item = qobject_cast<FileIconItem*>(editor);

    FMEvent event;
    event << fileInfo->fileUrl();
    event << windowId();
    event << FMEvent::FileView;

    QString new_file_name = lineEdit ? lineEdit->text() : item ? item->edit->toPlainText() : "";

    new_file_name.remove('/');
    new_file_name.remove(QChar(0));

    if (fileInfo->fileName() == new_file_name || new_file_name.isEmpty()) {
        return;
    }

    DUrl old_url = fileInfo->fileUrl();
    DUrl new_url = fileInfo->getUrlByNewFileName(new_file_name);

    if (lineEdit) {
        /// later rename file.
        TIMER_SINGLESHOT(0, {
                             fileService->renameFile(old_url, new_url, event);
                         }, old_url, new_url, event)
    } else {
        fileService->renameFile(old_url, new_url, event);
    }
}

void DFileView::handleViewModeChanged(const DFileView::ViewMode &viewMode)
{
    D_D(DFileView);
    if(viewMode == DFileView::IconMode){
        if(d->statusBar->scalingSlider()->isHidden())
            d->statusBar->scalingSlider()->show();
    }else{
        if(!d->statusBar->scalingSlider()->isHidden())
            d->statusBar->scalingSlider()->hide();
    }
}

void DFileView::focusInEvent(QFocusEvent *event)
{
    DListView::focusInEvent(event);
    itemDelegate()->commitDataAndCloseActiveEditor();

    const DUrl &current_url = currentUrl();

    if (current_url.isLocalFile())
            QDir::setCurrent(current_url.toLocalFile());
}

void DFileView::resizeEvent(QResizeEvent *event)
{
    DListView::resizeEvent(event);

    updateHorizontalOffset();

    if (itemDelegate()->editingIndex().isValid())
        doItemsLayout();
}

void DFileView::contextMenuEvent(QContextMenuEvent *event)
{
    D_DC(DFileView);

    bool isEmptyArea = d->fileViewHelper->isEmptyArea(event->pos());

    const QModelIndex &index = indexAt(event->pos());

    if (isEmptyArea  && !selectionModel()->isSelected(index)) {
        itemDelegate()->hideNotEditingIndexWidget();
        clearSelection();
        showEmptyAreaMenu();
    } else {
        if (!isSelected(index)) {
            setCurrentIndex(index);
        }

        showNormalMenu(index);
    }
}

void DFileView::dragEnterEvent(QDragEnterEvent *event)
{
    for (const DUrl &url : event->mimeData()->urls()) {
        const AbstractFileInfoPointer &fileInfo = FileServices::instance()->createFileInfo(url);

        if (!fileInfo->isWritable()) {
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
        const AbstractFileInfoPointer &fileInfo = model()->fileInfo(d->dragMoveHoverIndex);

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
        const AbstractFileInfoPointer &fileInfo = model()->fileInfo(index.isValid() ? index : rootIndex());

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

void DFileView::increaseIcon()
{
    D_D(DFileView);

    int iconSizeLevel = itemDelegate()->increaseIcon();

    if (iconSizeLevel >= 0) {
        QSignalBlocker blocker(d->scalingSlider);

        Q_UNUSED(blocker)
        d->scalingSlider->setValue(iconSizeLevel);
    }
}

void DFileView::decreaseIcon()
{
    D_D(DFileView);

    int iconSizeLevel = itemDelegate()->decreaseIcon();

    if (iconSizeLevel >= 0) {
        QSignalBlocker blocker(d->scalingSlider);

        Q_UNUSED(blocker)
        d->scalingSlider->setValue(iconSizeLevel);
    }
}

void DFileView::openIndex(const QModelIndex &index)
{
    D_D(DFileView);

   if (model()->hasChildren(index)) {
        FMEvent event;

        event << model()->getUrlByIndex(index);
        event << FMEvent::FileView;
        event << windowId();

        d->fileViewHelper->preHandleCd(event);
    } else {
        emit fileService->openFile(model()->getUrlByIndex(index));
    }
}

void DFileView::keyboardSearch(const QString &search)
{
    D_D(DFileView);

    if (search.isEmpty())
        return;

    d->fileViewHelper->keyboardSearch(search.toLocal8Bit().at(0));
}

bool DFileView::setCurrentUrl(const DUrl &url)
{
    D_D(DFileView);

    DUrl fileUrl = url;

    if (fileUrl.isTrashFile() && fileUrl.path().isEmpty()) {
        fileUrl.setPath("/");
    }

    const AbstractFileInfoPointer &info = FileServices::instance()->createFileInfo(fileUrl);

    if (!info){
        qDebug() << "This scheme isn't support";
        return false;
    }

    if(info->canRedirectionFileUrl()) {
        const DUrl old_url = fileUrl;

        fileUrl = info->redirectedFileUrl();

        qDebug() << "url redirected, from:" << old_url << "to:" << fileUrl;
    }

    qDebug() << "cd: current url:" << currentUrl() << "to url:" << fileUrl;

    const DUrl &currentUrl = this->currentUrl();


    if(currentUrl == fileUrl/* && !info->isShared()*/)
        return false;

//    QModelIndex index = model()->index(fileUrl);

//    if(!index.isValid())
    QModelIndex index = model()->setRootUrl(fileUrl);

//    model()->setActiveIndex(index);
    setRootIndex(index);

    if (!model()->canFetchMore(index)) {
        updateContentLabel();
    }

    if (d->currentViewMode == ListMode) {
        updateListHeaderViewProperty();
    } else {
        const QPair<int, int> &sort_config = FMStateManager::SortStates.value(fileUrl, QPair<int, int>(DFileSystemModel::FileDisplayNameRole, Qt::AscendingOrder));

        model()->setSortRole(sort_config.first, (Qt::SortOrder)sort_config.second);
    }

    if(info) {
        ViewModes modes = (ViewModes)info->supportViewMode();

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
    emit currentUrlChanged(fileUrl);

    if (focusWidget() && focusWidget()->window() == window() && fileUrl.isLocalFile())
        QDir::setCurrent(fileUrl.toLocalFile());

    setSelectionMode(info->supportSelectionMode());

    const DUrl &defaultSelectUrl = DUrl(QUrlQuery(fileUrl.query()).queryItemValue("selectUrl"));

    if (defaultSelectUrl.isValid()) {
        d->preSelectionUrls << defaultSelectUrl;
    } else {
        DUrl oldCurrentUrl = currentUrl;

        forever {
            const DUrl &tmp_url = oldCurrentUrl.parentUrl();

            if (tmp_url == fileUrl || !tmp_url.isValid())
                break;

            oldCurrentUrl = tmp_url;
        }

        d->preSelectionUrls << oldCurrentUrl;
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

void DFileView::handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    DListView::dataChanged(topLeft, bottomRight, roles);

    for (int i = topLeft.row(); i <= bottomRight.row(); ++i) {
        update(model()->index(i, 0));
    }
}

void DFileView::updateStatusBar()
{
    if (model()->state() != DFileSystemModel::Idle)
        return;

    FMEvent event;
    event << windowId();
    event << selectedUrls();
    int count = selectedIndexCount();

    if (count == 0){
        emit fileSignalManager->statusBarItemsCounted(event, this->count());
    }else{
        emit fileSignalManager->statusBarItemsSelected(event, count);
    }
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

void DFileView::updateHorizontalOffset()
{
    D_D(DFileView);

    if (isIconViewMode()) {
        int contentWidth = width();
        int itemWidth = itemSizeHint().width() + ICON_VIEW_SPACING * 2;
        int itemColumn = (contentWidth - ICON_VIEW_SPACING * 2.9) / itemWidth;

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

    const AbstractFileInfoPointer &fileInfo = model()->fileInfo(currentUrl());

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
        d->scalingSlider->setMinimum(itemDelegate()->minimumIconSizeLevel());
        d->scalingSlider->setMaximum(itemDelegate()->maximumIconSizeLevel());
        d->scalingSlider->setValue(itemDelegate()->iconSizeLevel());

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
                    model(), &QAbstractItemModel::sort);
            connect(d->headerView, &QHeaderView::customContextMenuRequested,
                    this, &DFileView::popupHeaderViewContextMenu);

            d->headerView->setAttribute(Qt::WA_TransparentForMouseEvents, model()->state() == DFileSystemModel::Busy);
        }

        addHeaderWidget(d->headerView);

        setOrientation(QListView::TopToBottom, false);
        setSpacing(LIST_VIEW_SPACING);

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

void DFileView::showEmptyAreaMenu()
{
    D_D(DFileView);

    const QModelIndex &index = rootIndex();
    const AbstractFileInfoPointer &info = model()->fileInfo(index);
    const QVector<MenuAction> &actions = info->menuActionList(AbstractFileInfo::SpaceArea);

    if (actions.isEmpty())
        return;

    const QMap<MenuAction, QVector<MenuAction> > &subActions = info->subMenuActionList();

    QSet<MenuAction> disableList = FileMenuManager::getDisableActionList(model()->getUrlByIndex(index));
    const bool& tabAddable = WindowManager::tabAddableByWinId(windowId());
    if(!tabAddable)
        disableList << MenuAction::OpenInNewTab;

    if (!count())
        disableList << MenuAction::SelectAll;

    DFileMenu *menu = FileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);
    DAction *tmp_action = menu->actionAt(fileMenuManger->getActionString(MenuAction::DisplayAs));
    DFileMenu *displayAsSubMenu = static_cast<DFileMenu*>(tmp_action ? tmp_action->menu() : Q_NULLPTR);
    tmp_action = menu->actionAt(fileMenuManger->getActionString(MenuAction::SortBy));
    DFileMenu *sortBySubMenu = static_cast<DFileMenu*>(tmp_action ? tmp_action->menu() : Q_NULLPTR);

    for (QAction *action : d->displayAsActionGroup->actions()) {
        d->displayAsActionGroup->removeAction(action);
    }

    if (displayAsSubMenu){
        foreach (DAction* action, displayAsSubMenu->actionList()) {
            action->setActionGroup(d->displayAsActionGroup);
        }
        if (d->currentViewMode == IconMode){
            displayAsSubMenu->actionAt(fileMenuManger->getActionString(MenuAction::IconView))->setChecked(true);
        }else if (d->currentViewMode == ListMode){
            displayAsSubMenu->actionAt(fileMenuManger->getActionString(MenuAction::ListView))->setChecked(true);
        }else if (d->currentViewMode == ExtendMode){
            displayAsSubMenu->actionAt(fileMenuManger->getActionString(MenuAction::ExtendView))->setChecked(true);
        }
    }

    for (QAction *action : d->sortByActionGroup->actions()) {
        d->sortByActionGroup->removeAction(action);
    }

    if (sortBySubMenu){
        foreach (DAction* action, sortBySubMenu->actionList()) {
            action->setActionGroup(d->sortByActionGroup);
        }

        DAction *action = sortBySubMenu->actionAt(model()->sortColumn());

        if (action)
            action->setChecked(true);
    }

    DUrlList urls;
    urls.append(currentUrl());

    FMEvent event;
    event << currentUrl();
    event << urls;
    event << windowId();
    event << FMEvent::FileView;
    menu->setEvent(event);


    menu->exec();
    menu->deleteLater();
}


void DFileView::showNormalMenu(const QModelIndex &index)
{
    D_D(DFileView);

    if(!index.isValid())
        return;

    DUrlList list = selectedUrls();

    DFileMenu* menu;

    const AbstractFileInfoPointer &info = model()->fileInfo(index);

    if (list.length() == 1) {
        const QVector<MenuAction> &actions = info->menuActionList(AbstractFileInfo::SingleFile);

        if (actions.isEmpty())
            return;

        const QMap<MenuAction, QVector<MenuAction> > &subActions = info->subMenuActionList();
        QSet<MenuAction> disableList = FileMenuManager::getDisableActionList(list);
        const bool& tabAddable = WindowManager::tabAddableByWinId(windowId());
        if(!tabAddable)
            disableList << MenuAction::OpenInNewTab;

        menu = FileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);

        DAction *openWithAction = menu->actionAt(FileMenuManager::getActionString(DFMGlobal::OpenWith));
        DFileMenu* openWithMenu = openWithAction ? qobject_cast<DFileMenu*>(openWithAction->menu()) : Q_NULLPTR;

        if (openWithMenu) {
            QMimeType mimeType = info->mimeType();
            QStringList recommendApps = mimeAppsManager->MimeApps.value(mimeType.name());

            foreach (QString name, mimeType.aliases()) {
                QStringList apps = mimeAppsManager->MimeApps.value(name);
                foreach (QString app, apps) {
                    if (!recommendApps.contains(app)){
                        recommendApps.append(app);
                    }
                }
            }

            for (QAction *action : d->openWithActionGroup->actions()) {
                d->openWithActionGroup->removeAction(action);
            }

            foreach (QString app, recommendApps) {
                DAction* action = new DAction(mimeAppsManager->DesktopObjs.value(app).getLocalName(), 0);
                action->setProperty("app", app);
                action->setProperty("url", info->redirectedFileUrl().toLocalFile());
                openWithMenu->addAction(action);
                d->openWithActionGroup->addAction(action);
            }

            DAction* action = new DAction(fileMenuManger->getActionString(MenuAction::OpenWithCustom), 0);
            action->setData((int)MenuAction::OpenWithCustom);
            openWithMenu->addAction(action);
        }
    } else {
        bool isSystemPathIncluded = false;
        bool isAllCompressedFiles = true;

        foreach (DUrl url, list) {
            const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(url);

            if(!FileUtils::isArchive(url.path()))
                isAllCompressedFiles = false;

            if (systemPathManager->isSystemPath(fileInfo->redirectedFileUrl().toLocalFile())) {
                isSystemPathIncluded = true;
            }
        }

        QVector<MenuAction> actions;

        if (isSystemPathIncluded)
            actions = info->menuActionList(AbstractFileInfo::MultiFilesSystemPathIncluded);
        else
            actions = info->menuActionList(AbstractFileInfo::MultiFiles);

        if (actions.isEmpty())
            return;

        if(isAllCompressedFiles)
            actions<<MenuAction::Decompress<<MenuAction::DecompressHere;

        const QMap<MenuAction, QVector<MenuAction> > subActions;
        QSet<MenuAction> disableList = FileMenuManager::getDisableActionList(list);
        const bool& tabAddable = WindowManager::tabAddableByWinId(windowId());
        if(!tabAddable)
            disableList << MenuAction::OpenInNewTab;
        menu = FileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);
    }

    FMEvent event;

    event << info->redirectedFileUrl();
    event << list;
    event << windowId();
    event << FMEvent::FileView;

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

    const QPair<int, int> &sort_config = FMStateManager::SortStates.value(currentUrl(), QPair<int, int>(DFileSystemModel::FileDisplayNameRole, Qt::AscendingOrder));
    int sort_column = model()->roleToColumn(sort_config.first);

    d->headerView->setSortIndicator(sort_column, Qt::SortOrder(sort_config.second));

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

    DMenu *menu = new DMenu();

    for (int i = 1; i < d->headerView->count(); ++i) {
        DAction *action = new DAction(menu);

        action->setText(model()->columnNameByRole(model()->columnToRole(i)).toString());
        action->setCheckable(true);
        action->setChecked(!d->headerView->isSectionHidden(i));

        connect(action, &DAction::triggered, this, [this, action, i, d] {
            d->columnForRoleHiddenMap[action->text()] = action->isChecked();

            d->headerView->setSectionHidden(i, action->isChecked());

            updateColumnWidth();
        });

        menu->addAction(action);
    }

    menu->exec();
}

void DFileView::onModelStateChanged(int state)
{
    D_D(DFileView);

    FMEvent event;

    event << windowId();
    event << currentUrl();

    emit fileSignalManager->loadingIndicatorShowed(event, state == DFileSystemModel::Busy);

    if (state == DFileSystemModel::Busy) {
        setContentLabel(QString());

        if (d->headerView) {
            d->headerView->setAttribute(Qt::WA_TransparentForMouseEvents);
        }
    } else if (state == DFileSystemModel::Idle) {
        for (const DUrl &url : d->preSelectionUrls) {
            selectionModel()->select(model()->index(url), QItemSelectionModel::SelectCurrent);
        }

        if (!d->preSelectionUrls.isEmpty()) {
            scrollTo(model()->index(d->preSelectionUrls.first()), PositionAtTop);
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
    const DUrl &currentUrl = this->currentUrl();

    if (count <= 0) {
        const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(currentUrl);

        setContentLabel(fileInfo->subtitleForEmptyFloder());
    } else {
        setContentLabel(QString());
    }
}

void DFileView::updateSelectionRect()
{
    D_D(DFileView);

    if (dragEnabled())
        return;

    QPoint pos = mapFromGlobal(QCursor::pos());

    if (d->selectionRectWidget->isVisible()) {
        QRect rect;
        QPoint pressedPos = viewport()->mapToParent(d->selectedGeometry.topLeft());

        pressedPos.setX(pressedPos.x() - horizontalOffset());
        pressedPos.setY(pressedPos.y() - verticalOffset());

        rect.setCoords(qMin(pressedPos.x(), pos.x()), qMin(pressedPos.y(), pos.y()),
                       qMax(pos.x(), pressedPos.x()), qMax(pos.y(), pressedPos.y()));

        d->selectionRectWidget->setGeometry(rect);
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
        AbstractFileInfoPointer info = model()->fileInfo(indexAt(event->pos()));

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
