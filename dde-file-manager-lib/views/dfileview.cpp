#include "dfileview.h"
#include "fileitem.h"
#include "filemenumanager.h"
#include "dfileitemdelegate.h"
#include "dfilemenu.h"
#include "windowmanager.h"
#include "dfilemanagerwindow.h"
#include "xdndworkaround.h"
#include "dstatusbar.h"
#include "app/global.h"
#include "app/fmevent.h"
#include "app/filemanagerapp.h"
#include "app/filesignalmanager.h"

#include "fileoperations/filejob.h"

#include "controllers/appcontroller.h"
#include "controllers/filecontroller.h"
#include "controllers/fileservices.h"
#include "controllers/fmstatemanager.h"
#include "controllers/pathmanager.h"

#include "models/fileinfo.h"
#include "models/dfileselectionmodel.h"
#include "models/dfilesystemmodel.h"

#include "shutil/fileutils.h"
#include "shutil/iconprovider.h"
#include "shutil/mimesappsmanager.h"

#include "widgets/singleton.h"

#include <dthememanager.h>
#include <dscrollbar.h>
#include <anchors.h>
#include <dslider.h>

#include <QWheelEvent>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>
#include <QX11Info>
#include <QUrlQuery>
#include <QProcess>
#include <QFrame>
#include <QLabel>
#include <QActionGroup>
#include <QContextMenuEvent>

DWIDGET_USE_NAMESPACE

#define ICON_VIEW_SPACING 5
#define LIST_VIEW_SPACING 1

#define DEFAULT_HEADER_SECTION_WIDTH 140

#define LIST_VIEW_ICON_SIZE 28

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
    QList<int> iconSizes;

    DFileView::ViewMode defaultViewMode = DFileView::IconMode;
    DFileView::ViewMode currentViewMode = DFileView::IconMode;

    int currentIconSizeIndex = 1;

    QRect selectedGeometry;
    QWidget *selectionRectWidget = Q_NULLPTR;
    bool selectionRectVisible = true;

    int horizontalOffset = 0;

    QTimer* keyboardSearchTimer;
    QString keyboardSearchKeys;

    QSize itemSizeHint;

    /// move cursor later selecte index when pressed key shift
    QModelIndex lastCursorIndex;

    /// cut state indexs
    static QSet<DUrl> cutUrlSet;

    /// list mode column visible
    QMap<QString, bool> columnForRoleHiddenMap;

    int firstVisibleColumn = -1;
    int lastVisibleColumn = -1;

    DUrlList preSelectionUrls;

    Anchors<QLabel> contentLabel = Q_NULLPTR;

    /// file additional icon
    QIcon lockIcon;
    QIcon linkIcon;
    QIcon unreadableIcon;

    QModelIndex mouseLastPressedIndex;

    /// drag drop
    QModelIndex dragMoveHoverIndex;

    DSlider *scalingSlider = NULL;

    Q_DECLARE_PUBLIC(DFileView)
};

QSet<DUrl> DFileViewPrivate::cutUrlSet;

DFileView::DFileView(QWidget *parent)
    : DListView(parent)
    , d_ptr(new DFileViewPrivate(this))
{
    D_THEME_INIT_WIDGET(DFileView);
    D_D(DFileView);

    d->displayAsActionGroup = new QActionGroup(this);
    d->sortByActionGroup = new QActionGroup(this);
    d->openWithActionGroup = new QActionGroup(this);
    initUI();
    initDelegate();
    initModel();
    initActions();
    initKeyboardSearchTimer();
    initConnects();
}

DFileView::~DFileView()
{
    disconnect(this, &DFileView::rowCountChanged, this, &DFileView::updateStatusBar);
    disconnect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &DFileView::updateStatusBar);
    disconnect(this, &DFileView::rowCountChanged, this, &DFileView::updateContentLabel);
}

void DFileView::initUI()
{
    D_D(DFileView);

    d->iconSizes << 48 << 64 << 96 << 128 << 256;

    setSpacing(ICON_VIEW_SPACING);
    setResizeMode(QListView::Adjust);
    setOrientation(QListView::LeftToRight, true);
    setIconSize(currentIconSize());
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

    d->linkIcon = QIcon(":/images/images/link_large.png");
    d->lockIcon = QIcon(":/images/images/lock_large.png");
    d->unreadableIcon = QIcon(":/images/images/unreadable.svg");

    d->statusBar = new DStatusBar(this);
    d->scalingSlider = d->statusBar->scalingSlider();
    d->scalingSlider->setValue(d->currentIconSizeIndex);
    addFooterWidget(d->statusBar);
}

void DFileView::initDelegate()
{
    setItemDelegate(new DFileItemDelegate(this));
}

void DFileView::initModel()
{
    setModel(new DFileSystemModel(this));
#ifndef CLASSICAL_SECTION
    setSelectionModel(new DFileSelectionModel(model(), this));
#endif
}

void DFileView::initConnects()
{
    D_D(DFileView);

    connect(this, &DFileView::doubleClicked,
            this, [this] (const QModelIndex &index) {
        if (!Global::keyCtrlIsPressed() && !Global::keyShiftIsPressed())
            openIndex(index);
    }, Qt::QueuedConnection);


    connect(fileSignalManager, &FileSignalManager::requestRename,
            this, static_cast<void (DFileView::*)(const FMEvent&)>(&DFileView::edit));
    connect(fileSignalManager, &FileSignalManager::requestViewSelectAll,
            this, &DFileView::selectAll);
    connect(fileSignalManager, &FileSignalManager::requestSelectFile,
            this, static_cast<bool (DFileView::*)(const FMEvent&)>(&DFileView::select));
    connect(fileSignalManager, &FileSignalManager::requestSelectRenameFile,
            this, &DFileView::selectAndRename);
    connect(this, &DFileView::rowCountChanged, this, &DFileView::updateStatusBar);

    connect(d->displayAsActionGroup, &QActionGroup::triggered, this, &DFileView::dislpayAsActionTriggered);
    connect(d->sortByActionGroup, &QActionGroup::triggered, this, &DFileView::sortByActionTriggered);
    connect(d->openWithActionGroup, &QActionGroup::triggered, this, &DFileView::openWithActionTriggered);
    connect(d->keyboardSearchTimer, &QTimer::timeout, this, &DFileView::clearKeyBoardSearchKeys);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &DFileView::updateStatusBar);
    connect(fileSignalManager, &FileSignalManager::requestFoucsOnFileView, this, &DFileView::setFoucsOnFileView);
    connect(fileSignalManager, &FileSignalManager::requestFreshFileView, this, &DFileView::refreshFileView);

    connect(itemDelegate(), &DFileItemDelegate::commitData, this, &DFileView::handleCommitData);
    connect(model(), &DFileSystemModel::dataChanged, this, &DFileView::handleDataChanged);
    connect(model(), &DFileSystemModel::rootUrlDeleted, this, &DFileView::updateContentLabel, Qt::QueuedConnection);
    connect(model(), &DFileSystemModel::stateChanged, this, &DFileView::onModelStateChanged);

    connect(fileIconProvider, &IconProvider::themeChanged, model(), &DFileSystemModel::update);
    connect(fileIconProvider, &IconProvider::iconChanged, this, [this] (const QString &filePath) {
        update(model()->index(DUrl::fromLocalFile(filePath)));
    });

    if (!d->cutUrlSet.capacity()) {
        d->cutUrlSet.reserve(1);

        connect(qApp->clipboard(), &QClipboard::dataChanged, [] {
            DFileViewPrivate::cutUrlSet.clear();

            const QByteArray &data = qApp->clipboard()->mimeData()->data("x-special/gnome-copied-files");

            if (!data.startsWith("cut"))
                return;

            for (const QUrl &url : qApp->clipboard()->mimeData()->urls()) {
                DFileViewPrivate::cutUrlSet << url;
            }
        });
    }

    connect(qApp->clipboard(), &QClipboard::dataChanged, this, [this] {
        FileIconItem *item = qobject_cast<FileIconItem*>(itemDelegate()->editingIndexWidget());

        if (item)
            item->setOpacity(isCutIndex(itemDelegate()->editingIndex()) ? 0.3 : 1);

        if (itemDelegate()->expandedIndex().isValid()) {
            item = itemDelegate()->expandedIndexWidget();

            item->setOpacity(isCutIndex(itemDelegate()->expandedIndex()) ? 0.3 : 1);
        }

        update();
    });

    connect(d->scalingSlider, &DSlider::valueChanged,this,&DFileView::setIconSizeWithIndex);
}

void DFileView::initActions()
{
    QAction *copy_action = new QAction(this);

    copy_action->setAutoRepeat(false);
    copy_action->setShortcut(QKeySequence::Copy);

    connect(copy_action, &QAction::triggered,
            this, [this] {
        fileService->copyFiles(selectedUrls());
    });

    QAction *cut_action = new QAction(this);

    cut_action->setAutoRepeat(false);
    cut_action->setShortcut(QKeySequence::Cut);

    connect(cut_action, &QAction::triggered,
            this, [this] {
        fileService->cutFiles(selectedUrls());
    });

    QAction *paste_action = new QAction(this);

    paste_action->setShortcut(QKeySequence::Paste);

    connect(paste_action, &QAction::triggered,
            this, [this] {
        FMEvent event;

        event = currentUrl();
        event = windowId();
        event = FMEvent::FileView;
        fileService->pasteFile(event);
    });

    addAction(copy_action);
    addAction(cut_action);
    addAction(paste_action);
}

void DFileView::initKeyboardSearchTimer()
{
    D_D(DFileView);

    d->keyboardSearchTimer = new QTimer(this);
    d->keyboardSearchTimer->setInterval(500);
}

DFileSystemModel *DFileView::model() const
{
    return qobject_cast<DFileSystemModel*>(DListView::model());
}

DFileItemDelegate *DFileView::itemDelegate() const
{
    return qobject_cast<DFileItemDelegate*>(DListView::itemDelegate());
}

DStatusBar *DFileView::statusBar() const
{
    D_DC(DFileView);

    return d->statusBar;
}

DUrl DFileView::currentUrl() const
{
    return model()->getUrlByIndex(rootIndex());
}

DUrlList DFileView::selectedUrls() const
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

    updateItemSizeHint();
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
        QWidget *widget = itemDelegate()->expandedIndexWidget();

        if (widget->isVisible() && widget->geometry().contains(point)) {
            return itemDelegate()->expandedIndex();
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

    return d->itemSizeHint;
}

bool DFileView::isCutIndex(const QModelIndex &index) const
{
    D_DC(DFileView);

    const AbstractFileInfoPointer &fileInfo = model()->fileInfo(index);

    if (!fileInfo || !fileInfo->canRedirectionFileUrl())
        return d->cutUrlSet.contains(model()->getUrlByIndex(index));

    return d->cutUrlSet.contains(fileInfo->redirectedFileUrl());
}

bool DFileView::isActiveIndex(const QModelIndex &index) const
{
    D_DC(DFileView);

    return d->dragMoveHoverIndex == index;
}

QList<QIcon> DFileView::fileAdditionalIcon(const QModelIndex &index) const
{
    D_DC(DFileView);

    QList<QIcon> icons;
    const AbstractFileInfoPointer &fileInfo = model()->fileInfo(index);

    if (!fileInfo)
        return icons;

    if (fileInfo->isSymLink()) {
        icons << d->linkIcon;
    }

    if (!fileInfo->isWritable())
        icons << d->lockIcon;

    if (!fileInfo->isReadable())
        icons << d->unreadableIcon;

    return icons;
}

void DFileView::preHandleCd(const FMEvent &event)
{
    qDebug() << event;
    if (event.fileUrl().isNetWorkFile()){
        emit fileSignalManager->requestFetchNetworks(event);
        return;
    }else if (event.fileUrl().isSMBFile()){
        emit fileSignalManager->requestFetchNetworks(event);
        return;
    }
    cd(event);
}

void DFileView::cd(const FMEvent &event)
{
    if (event.windowId() != windowId())
        return;

    cd(event.fileUrl());
}

void DFileView::cd(const DUrl &url)
{
    if (url.isEmpty())
        return;

    itemDelegate()->hideAllIIndexWidget();

    clearSelection();

    if (!url.isSearchFile()){
        setFocus();
    }

    if (setCurrentUrl(url)) {
        FMEvent e;
        e = FMEvent::FileView;
        e = windowId();
        e = currentUrl();
        emit fileSignalManager->currentUrlChanged(e);
    }
}

void DFileView::cdUp(const FMEvent &event)
{
    AbstractFileInfoPointer fileInfo = model()->fileInfo(rootIndex());

    const DUrl &oldCurrentUrl = this->currentUrl();
    const DUrl& parentUrl = fileInfo ? fileInfo->parentUrl() : DUrl::parentUrl(oldCurrentUrl);
    const_cast<FMEvent&>(event) = parentUrl;

    cd(event);
}

void DFileView::edit(const FMEvent &event)
{
    if(event.windowId() != windowId())
        return;

    DUrl fileUrl = event.fileUrl();

    if(fileUrl.isEmpty())
        return;

    const QModelIndex &index = model()->index(fileUrl);

    edit(index, QAbstractItemView::EditKeyPressed, 0);
}

bool DFileView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    DUrl fileUrl = model()->getUrlByIndex(index);

    if (fileUrl.isEmpty() || selectedIndexCount() > 1 || (trigger == SelectedClicked && Global::keyShiftIsPressed()))
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

bool DFileView::select(const FMEvent &event)
{
    if (event.windowId() != windowId()) {
        return false;
    }

    select(event.fileUrlList());

    return true;
}

void DFileView::select(const DUrlList &list)
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

void DFileView::selectAndRename(const FMEvent &event)
{
    bool isSelected = select(event);

    if (isSelected) {
        FMEvent e = event;

        if (!e.fileUrlList().isEmpty())
            e = e.fileUrlList().first();

        appController->actionRename(e);
    }
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

void DFileView::selectAll(int windowId)
{
    if(windowId != WindowManager::getWindowId(this))
        return;

    DListView::selectAll();
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

void DFileView::wheelEvent(QWheelEvent *event)
{
    if(isIconViewMode() && Global::keyCtrlIsPressed()) {
        if(event->angleDelta().y() > 0) {
            enlargeIcon();
        } else {
            shrinkIcon();
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

    fmevent = urls;
    fmevent = FMEvent::FileView;
    fmevent = windowId();
    fmevent = currentUrl();

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
            cdUp(fmevent);

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
                fmevent = DUrlList() << fmevent.fileUrl();

            appController->actionProperty(fmevent);

            return;
        case Qt::Key_Up:
            cdUp(fmevent);

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
            cdUp(fmevent);

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
            fmevent = DUrl::fromLocalFile(QDir::homePath());

            cd(fmevent);

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

        event = this->windowId();
        event = FMEvent::FileView;

        fileSignalManager->requestBack(event);

        break;
    }
    case Qt::ForwardButton: {
        FMEvent event;

        event = this->windowId();
        event = FMEvent::FileView;

        fileSignalManager->requestForward(event);

        break;
    }
    case Qt::LeftButton: {
        bool isEmptyArea = this->isEmptyArea(event->pos());

        setDragEnabled(!isEmptyArea);

        if (isEmptyArea) {
            if (!Global::keyCtrlIsPressed()) {
                itemDelegate()->hideExpandedIndex();
                clearSelection();
            }

            if (canShowSelectionRect())
                d->selectionRectWidget->show();

            d->selectedGeometry.setTop(event->pos().y() + verticalOffset());
            d->selectedGeometry.setLeft(event->pos().x() + horizontalOffset());

            connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &DFileView::updateSelectionRect);
        } else if (Global::keyCtrlIsPressed()) {
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

    if (d->mouseLastPressedIndex.isValid() && Global::keyCtrlIsPressed()) {
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
    event = fileInfo->fileUrl();
    event = windowId();
    event = FMEvent::FileView;

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
    bool isEmptyArea = this->isEmptyArea(event->pos());

    const QModelIndex &index = indexAt(event->pos());

    if (isEmptyArea  && !selectionModel()->isSelected(index)) {
        itemDelegate()->hideExpandedIndex();
        clearSelection();
        showEmptyAreaMenu();
    } else {
        const QModelIndexList &list = selectedIndexes();

        if (!list.contains(index)) {
            setCurrentIndex(index);
        }

        showNormalMenu(index);
    }
}

//bool DFileView::event(QEvent *event)
//{
//    switch (event->type()) {
//    case QEvent::MouseButtonPress: {
//        QMouseEvent *e = static_cast<QMouseEvent*>(event);

//        const QPoint &pos = viewport()->mapFromParent(e->pos());
//        bool isEmptyArea = this->isEmptyArea(pos);

//        if (e->button() == Qt::LeftButton) {
//            if (isEmptyArea && !Global::keyCtrlIsPressed()) {
//                clearSelection();
//                itemDelegate()->hideAllIIndexWidget();
//            }

//            if (isEmptyArea) {
//                if (canShowSElectionRect())
//                    d->selectionRectWidget->show();

//                d->pressedPos = static_cast<QMouseEvent*>(event)->pos();
//            }
//        } else if (e->button() == Qt::RightButton) {
//            if (isEmptyArea  && !selectionModel()->isSelected(indexAt(pos))) {
//                clearSelection();
//                showEmptyAreaMenu();
//            } else {
//                if (!hasFocus() && this->childAt(pos)->hasFocus())
//                    return DListView::event(event);

//                const QModelIndex &index = indexAt(pos);
//                const QModelIndexList &list = selectedIndexes();

//                if (!list.contains(index)) {
//                    setCurrentIndex(index);
//                }

//                showNormalMenu(index);
//            }
//        }
//        break;
//    }
//    case QEvent::MouseMove: {
//        if (d->selectionRectWidget->isHidden())
//            break;

//        const QPoint &pos = static_cast<QMouseEvent*>(event)->pos();
//        QRect rect;

//        rect.adjust(qMin(d->pressedPos.x(), pos.x()), qMin(d->pressedPos.y(), pos.y()),
//                    qMax(pos.x(), d->pressedPos.x()), qMax(pos.y(), d->pressedPos.y()));

//        d->selectionRectWidget->setGeometry(rect);

////        rect.moveTopLeft(viewport()->mapFromParent(rect.topLeft()));

////        setSelection(rect, QItemSelectionModel::Current|QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect);

//        break;
//    }
//    case QEvent::MouseButtonRelease: {
//        d->selectionRectWidget->resize(0, 0);
//        d->selectionRectWidget->hide();

//        break;
//    }
//    default:
//        break;
//    }

//    return DListView::event(event);
//}

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

    // Try to support XDS
    // NOTE: in theory, it's not possible to implement XDS with pure Qt.
    // We achieved this with some dirty XCB/XDND workarounds.
    // Please refer to XdndWorkaround::clientMessage() in xdndworkaround.cpp for details.
    if (QX11Info::isPlatformX11() && event->mimeData()->hasFormat("XdndDirectSave0")) {
        event->setDropAction(Qt::CopyAction);
        //    const QWidget* targetWidget = childView()->viewport();
        // these are dynamic QObject property set by our XDND workarounds in xworkaround.cpp.
        //    xcb_window_t dndSource = xcb_window_t(targetWidget->property("xdnd::lastDragSource").toUInt());
        //    xcb_timestamp_t dropTimestamp = (xcb_timestamp_t)targetWidget->property("xdnd::lastDropTime").toUInt();

        xcb_window_t dndSource = xcb_window_t(XdndWorkaround::lastDragSource);
//        xcb_timestamp_t dropTimestamp = (xcb_timestamp_t)XdndWorkaround::lastDropTime;
        // qDebug() << "XDS: source window" << dndSource << dropTimestamp;
        if (dndSource != 0) {
            xcb_connection_t* conn = QX11Info::connection();
            xcb_atom_t XdndDirectSaveAtom = XdndWorkaround::internAtom("XdndDirectSave0", 15);
            xcb_atom_t textAtom = XdndWorkaround::internAtom("text/plain", 10);

            // 1. get the filename from XdndDirectSave property of the source window


            QByteArray basename = XdndWorkaround::windowProperty(dndSource, XdndDirectSaveAtom, textAtom, 1024);

            // 2. construct the fill URI for the file, and update the source window property.

            QByteArray fileUri;

            const QModelIndex &index = indexAt(event->pos());
            const AbstractFileInfoPointer &fileInfo = model()->fileInfo(index.isValid() ? index : rootIndex());

            if (fileInfo && fileInfo->fileUrl().isLocalFile() && fileInfo->isDir()) {
                fileUri.append(fileInfo->fileUrl().toString() + "/" + basename);
            }

            XdndWorkaround::setWindowProperty(dndSource,  XdndDirectSaveAtom, textAtom, (void*)fileUri.constData(), fileUri.length());

            // 3. send to XDS selection data request with type "XdndDirectSave" to the source window and
            //    receive result from the source window. (S: success, E: error, or F: failure)
            QByteArray result = event->mimeData()->data("XdndDirectSave0");
            Q_UNUSED(result)
//            qDebug() <<"result" << result;
            // NOTE: there seems to be some bugs in file-roller so it always replies with "E" even if the
            //       file extraction is finished successfully. Anyways, we ignore any error at the moment.
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
    if (Global::keyShiftIsPressed()) {
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
        if (Global::keyShiftIsPressed()) {
            index = DListView::moveCursor(cursorAction, modifiers);

            if (index == d->lastCursorIndex) {
                index = index.sibling(index.row() - 1, index.column());
            }
        } else {
            index = current.sibling(current.row() - 1, current.column());
        }

        break;
    case MoveRight:
        if (Global::keyShiftIsPressed()) {
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

bool DFileView::isEmptyArea(const QModelIndex &index, const QPoint &pos) const
{
    if(index.isValid() && selectionModel()->selectedIndexes().contains(index)) {
        return false;
    } else {
        const QRect &rect = visualRect(index);

        if(!rect.contains(pos))
            return true;

        QStyleOptionViewItem option = viewOptions();

        option.rect = rect;

        const QList<QRect> &geometry_list = itemDelegate()->paintGeomertys(option, index);

        for(const QRect &rect : geometry_list) {
            if(rect.contains(pos)) {
                return false;
            }
        }
    }

    return true;
}

QSize DFileView::currentIconSize() const
{
    D_DC(DFileView);

    int size = d->iconSizes.value(d->currentIconSizeIndex);

    return QSize(size, size);
}

void DFileView::enlargeIcon()
{
    D_D(DFileView);

    if(d->currentIconSizeIndex < d->iconSizes.count() - 1){
        ++d->currentIconSizeIndex;
        d->scalingSlider->setValue(d->currentIconSizeIndex);
    }

    setIconSize(currentIconSize());
}

void DFileView::shrinkIcon()
{
    D_D(DFileView);

    if(d->currentIconSizeIndex > 0){
        --d->currentIconSizeIndex;
        d->scalingSlider->setValue(d->currentIconSizeIndex);
    }

    setIconSize(currentIconSize());
}

void DFileView::openIndex(const QModelIndex &index)
{
   if (model()->hasChildren(index)) {
        FMEvent event;

        event = model()->getUrlByIndex(index);
        event = FMEvent::FileView;
        event = windowId();

        cd(event);
    } else {
        emit fileService->openFile(model()->getUrlByIndex(index));
    }
}

void DFileView::keyboardSearch(const QString &search)
{
    D_D(DFileView);

    d->keyboardSearchKeys.append(search);
    d->keyboardSearchTimer->start();
    QModelIndexList matchModelIndexListCaseSensitive = model()->match(rootIndex(), DFileSystemModel::FilePinyinName, d->keyboardSearchKeys, -1,
                                                                      Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap | Qt::MatchCaseSensitive | Qt::MatchRecursive));
    foreach (const QModelIndex& index, matchModelIndexListCaseSensitive) {
        QString absolutePath = FileInfo(model()->getUrlByIndex(index).path()).absolutePath();
        if (absolutePath == currentUrl().path()){
            setCurrentIndex(index);
            scrollTo(index, PositionAtTop);
            return;
        }
    }

    QModelIndexList matchModelIndexListNoCaseSensitive = model()->match(rootIndex(), DFileSystemModel::FilePinyinName, d->keyboardSearchKeys, -1,
                                                                        Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap | Qt::MatchRecursive));
    foreach (const QModelIndex& index, matchModelIndexListNoCaseSensitive) {
        QString absolutePath = FileInfo(model()->getUrlByIndex(index).path()).absolutePath();
        if (absolutePath == currentUrl().path()){
            setCurrentIndex(index);
            scrollTo(index, PositionAtTop);
            return;
        }
    }

}

bool DFileView::setCurrentUrl(DUrl fileUrl)
{
    D_D(DFileView);

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


    if(currentUrl == fileUrl)
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

void DFileView::clearKeyBoardSearchKeys()
{
    D_D(DFileView);

    d->keyboardSearchKeys.clear();
    d->keyboardSearchTimer->stop();
}

void DFileView::setFoucsOnFileView(const FMEvent &event)
{
    if (event.windowId() == windowId())
        setFocus();
}

void DFileView::refreshFileView(const FMEvent &event)
{
    if (event.windowId() != windowId()){
        return;
    }
    model()->refresh();
}

void DFileView::clearSelection()
{
    QListView::clearSelection();
}

void DFileView::handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    DListView::dataChanged(topLeft, bottomRight, roles);
}

void DFileView::updateStatusBar()
{
    if (model()->state() != DFileSystemModel::Idle)
        return;

    FMEvent event;
    event = windowId();
    event = selectedUrls();
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

    if (text.isEmpty()) {
        if (d->contentLabel) {
            d->contentLabel->deleteLater();
            d->contentLabel = Q_NULLPTR;
        }

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

void DFileView::setIconSizeWithIndex(const int index)
{
    D_D(DFileView);

    d->currentIconSizeIndex = index;
    setIconSize(currentIconSize());
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
        setIconSize(currentIconSize());
        setOrientation(QListView::LeftToRight, true);
        setSpacing(ICON_VIEW_SPACING);

        break;
    }
    case ListMode: {
        itemDelegate()->hideAllIIndexWidget();

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

        setIconSize(QSize(LIST_VIEW_ICON_SIZE, LIST_VIEW_ICON_SIZE));
        setOrientation(QListView::TopToBottom, false);
        setSpacing(LIST_VIEW_SPACING);

        break;
    }
    case ExtendMode: {
        itemDelegate()->hideAllIIndexWidget();
        if(!d->headerView) {
            d->headerView = new QHeaderView(Qt::Horizontal);

            updateExtendHeaderViewProperty();

            d->headerView->setHighlightSections(true);
            d->headerView->setSectionsClickable(true);
            d->headerView->setSortIndicatorShown(true);
            d->headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            if(selectionModel()) {
                d->headerView->setSelectionModel(selectionModel());
            }

            connect(d->headerView, &QHeaderView::sectionResized,
                    this, static_cast<void (DFileView::*)()>(&DFileView::update));
            connect(d->headerView, &QHeaderView::sortIndicatorChanged,
                    model(), &QAbstractItemModel::sort);
        }
        setColumnWidth(0, 200);
        setIconSize(QSize(30, 30));
        setOrientation(QListView::TopToBottom, false);
        setSpacing(LIST_VIEW_SPACING);
        break;
    }
    default:
        break;
    }

    updateItemSizeHint();

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
    event = currentUrl();
    event = urls;
    event = windowId();
    event = FMEvent::FileView;
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
        const QSet<MenuAction> &disableList = FileMenuManager::getDisableActionList(list);

        menu = FileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);

        DAction *openWithAction = menu->actionAt(FileMenuManager::getActionString(MenuActionType::OpenWith));
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
        const QSet<MenuAction> &disableList = FileMenuManager::getDisableActionList(list);
        menu = FileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);
    }

    FMEvent event;

    event = info->fileUrl();
    event = list;
    event = windowId();
    event = FMEvent::FileView;

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

void DFileView::updateItemSizeHint()
{
    D_D(DFileView);

    if (isIconViewMode()) {
        int width = iconSize().width() * 1.8;

        d->itemSizeHint = QSize(width, iconSize().height() + 2 * TEXT_PADDING  + ICON_MODE_ICON_SPACING + LIST_VIEW_SPACING + 3 * TEXT_LINE_HEIGHT);
    } else {
        d->itemSizeHint = QSize(-1, iconSize().height() * 1.1);
    }
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

    event = windowId();
    event = currentUrl();

    emit fileSignalManager->loadingIndicatorShowed(event, state == DFileSystemModel::Busy);

    if (state == DFileSystemModel::Busy) {
        setContentLabel(QString());

        disconnect(this, &DFileView::rowCountChanged, this, &DFileView::updateContentLabel);

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

        connect(this, &DFileView::rowCountChanged, this, &DFileView::updateContentLabel);

        if (d->headerView) {
            d->headerView->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        }
    }
}

void DFileView::updateContentLabel()
{
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
    if (event->source() == this && !Global::keyCtrlIsPressed()) {
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
