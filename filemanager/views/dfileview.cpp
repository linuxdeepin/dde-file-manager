#include "dfileview.h"
#include "dfilesystemmodel.h"
#include "fileitem.h"
#include "filemenumanager.h"
#include "dfileitemdelegate.h"
#include "fileinfo.h"
#include "dfilemenu.h"
#include "dscrollbar.h"
#include "windowmanager.h"
#include "dfilemanagerwindow.h"
#include "dfileselectionmodel.h"

#include "../app/global.h"
#include "../app/fmevent.h"
#include "../app/filemanagerapp.h"
#include "../app/filesignalmanager.h"

#include "../controllers/appcontroller.h"
#include "../controllers/filecontroller.h"
#include "../controllers/fileservices.h"
#include "../controllers/filejob.h"
#include "../controllers/fmstatemanager.h"

#include "../models/dfilesystemmodel.h"

#include "../shutil/fileutils.h"

#include <dthememanager.h>

#include <QWheelEvent>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>

DWIDGET_USE_NAMESPACE

#define ICON_VIEW_SPACING 5
#define LIST_VIEW_SPACING 0

#define DEFAULT_HEADER_SECTION_WIDTH 140

#define LIST_VIEW_ICON_SIZE 28

QSet<DUrl> DFileView::m_cutUrlSet;

DFileView::DFileView(QWidget *parent) : DListView(parent)
{
    D_THEME_INIT_WIDGET(DFileView);
    m_displayAsActionGroup = new QActionGroup(this);
    m_sortByActionGroup = new QActionGroup(this);
    m_openWithActionGroup = new QActionGroup(this);
    initUI();
    initDelegate();
    initModel();
    initActions();
    initKeyboardSearchTimer();
    initConnects();
}

DFileView::~DFileView()
{

}

void DFileView::initUI()
{
    m_iconSizes << 48 << 64 << 96 << 128 << 256;

    setSpacing(ICON_VIEW_SPACING);
    setResizeMode(QListView::Adjust);
    setOrientation(QListView::LeftToRight, true);
    setIconSize(currentIconSize());
    setTextElideMode(Qt::ElideMiddle);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(false);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QListView::EditKeyPressed | QListView::SelectedClicked);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBar(new DScrollBar);
    DListView::setSelectionRectVisible(false);

    m_selectionRectWidget = new QWidget(this);
    m_selectionRectWidget->hide();
    m_selectionRectWidget->resize(0, 0);
    m_selectionRectWidget->setObjectName("SelectionRect");
    m_selectionRectWidget->raise();
}

void DFileView::initDelegate()
{
    setItemDelegate(new DFileItemDelegate(this));
}

void DFileView::initModel()
{
    setModel(new DFileSystemModel(this));
    setSelectionModel(new DFileSelectionModel(model(), this));
}

void DFileView::initConnects()
{
    connect(this, &DFileView::doubleClicked,
            this, &DFileView::openIndex);
    connect(fileSignalManager, &FileSignalManager::fetchNetworksSuccessed,
            this, &DFileView::cd);
    connect(fileSignalManager, &FileSignalManager::requestChangeCurrentUrl,
            this, &DFileView::preHandleCd);

    connect(fileSignalManager, &FileSignalManager::requestRename,
            this, static_cast<void (DFileView::*)(const FMEvent&)>(&DFileView::edit));
    connect(fileSignalManager, &FileSignalManager::requestViewSelectAll,
            this, &DFileView::selectAll);
    connect(fileSignalManager, &FileSignalManager::requestSelectFile,
            this, &DFileView::select);
    connect(fileSignalManager, &FileSignalManager::requestSelectRenameFile,
            this, &DFileView::selectAndRename);

    connect(m_displayAsActionGroup, &QActionGroup::triggered, this, &DFileView::dislpayAsActionTriggered);
    connect(m_sortByActionGroup, &QActionGroup::triggered, this, &DFileView::sortByActionTriggered);
    connect(m_openWithActionGroup, &QActionGroup::triggered, this, &DFileView::openWithActionTriggered);
    connect(m_keyboardSearchTimer, &QTimer::timeout, this, &DFileView::clearKeyBoardSearchKeys);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &DFileView::handleSelectionChanged);
    connect(model(), &DFileSystemModel::rowsInserted, this, &DFileView::handleSelectionChanged);
    connect(model(), &DFileSystemModel::rowsRemoved, this, &DFileView::handleSelectionChanged);
    connect(fileSignalManager, &FileSignalManager::requestFoucsOnFileView, this, &DFileView::setFoucsOnFileView);
    connect(fileSignalManager, &FileSignalManager::requestFreshFileView, this, &DFileView::refreshFileView);

    connect(itemDelegate(), &DFileItemDelegate::commitData, this, &DFileView::handleCommitData);
    connect(model(), &DFileSystemModel::dataChanged, this, &DFileView::handleDataChanged);
    connect(model(), &DFileSystemModel::rootUrlDeleted, this, &DFileView::updateContentLabel, Qt::QueuedConnection);
    connect(model(), &DFileSystemModel::childrenUpdated, this, &DFileView::updateContentLabel, Qt::QueuedConnection);

    if (!m_cutUrlSet.capacity()) {
        m_cutUrlSet.reserve(1);

        connect(qApp->clipboard(), &QClipboard::dataChanged, [] {
            DFileView::m_cutUrlSet.clear();

            if (qApp->clipboard()->mimeData()->text() != "cut")
                return;

            for (const QUrl &url : qApp->clipboard()->mimeData()->urls()) {
                DFileView::m_cutUrlSet << url;
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

    connect(model(), &DFileSystemModel::childrenUpdated, this, &DFileView::onChildrenUpdated);
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
    m_keyboardSearchTimer = new QTimer(this);
    m_keyboardSearchTimer->setInterval(500);
}

DFileSystemModel *DFileView::model() const
{
    return qobject_cast<DFileSystemModel*>(DListView::model());
}

DFileItemDelegate *DFileView::itemDelegate() const
{
    return qobject_cast<DFileItemDelegate*>(DListView::itemDelegate());
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
    return m_headerView ? m_headerView->sectionSize(column) : 100;
}

void DFileView::setColumnWidth(int column, int width)
{
    if(!m_headerView)
        return;

    m_headerView->resizeSection(column, width);
}

int DFileView::columnCount() const
{
    return m_headerView ? m_headerView->count() : 1;
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

    return (width() - ICON_VIEW_SPACING * 2.5) / itemWidth;
}

QList<int> DFileView::columnRoleList() const
{
    return m_columnRoles;
}

int DFileView::windowId() const
{
    return window()->winId();
}

void DFileView::startKeyboardSearch(int windowId, const QString &key)
{
    if(windowId != this->windowId())
        return;

    keyboardSearch(key);
}

void DFileView::stopKeyboardSearch(int windowId)
{
    if(windowId != this->windowId())
        return;

    stopSearch();
}

void DFileView::setIconSize(const QSize &size)
{
    DListView::setIconSize(size);

    updateItemSizeHint();
    updateHorizontalOffset();
}

DFileView::ViewMode DFileView::getDefaultViewMode()
{
    return m_defaultViewMode;
}

int DFileView::getSortRoles()
{
    return model()->sortRole();
}

bool DFileView::testViewMode(ViewModes modes, DFileView::ViewMode mode)
{
    return (modes | mode) == modes;
}

int DFileView::horizontalOffset() const
{
    return m_horizontalOffset;
}

bool DFileView::isSelected(const QModelIndex &index) const
{
    return static_cast<DFileSelectionModel*>(selectionModel())->isSelected(index);
}

QModelIndexList DFileView::selectedIndexes() const
{
    return static_cast<DFileSelectionModel*>(selectionModel())->selectedIndexes();
}

QModelIndex DFileView::indexAt(const QPoint &point) const
{
    if (isIconViewMode()) {
        QWidget *widget = itemDelegate()->expandedIndexWidget();

        if (widget->isVisible() && widget->geometry().contains(point)) {
            return itemDelegate()->expandedIndex();
        }
    }

    QWidget *widget = itemDelegate()->editingIndexWidget();

    if (widget && widget->geometry().contains(point)) {
        return itemDelegate()->editingIndex();
    }

    QPoint pos = QPoint(point.x() + horizontalOffset(), point.y() + verticalOffset());
    QSize item_size = itemSizeHint();

    if (pos.y() % (item_size.height() + spacing() * 2) < spacing())
        return QModelIndex();

    int index = -1;

    if (item_size.width() == -1) {
        index = pos.y() / (item_size.height() + LIST_VIEW_SPACING * 2);
    } else {
        int item_width = item_size.width() + ICON_VIEW_SPACING * 2;

        if (pos.x() % item_width <= ICON_VIEW_SPACING)
            return QModelIndex();

        int row_index = pos.y() / (item_size.height() + ICON_VIEW_SPACING * 2);
        int column_count = (width() - ICON_VIEW_SPACING * 2.5) / item_width;
        int column_index = pos.x() / item_width;

        if (column_index >= column_count)
            return QModelIndex();

        index = row_index * column_count + column_index;
    }

    return rootIndex().child(index, 0);
}

bool DFileView::isCutIndex(const QModelIndex &index) const
{
    return m_cutUrlSet.contains(model()->getUrlByIndex(index));
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

    const DUrl &fileUrl = event.fileUrl();

    if (fileUrl.isEmpty())
        return;

    itemDelegate()->hideAllIIndexWidget();
    clearSelection();

    if (!event.fileUrl().isSearchFile()){
        setFocus();
    }

    if (setCurrentUrl(fileUrl)) {
        FMEvent e = event;
        e = currentUrl();
        emit fileSignalManager->currentUrlChanged(e);
        updateStatusBar();
    }
}

void DFileView::cdUp(const FMEvent &event)
{
    AbstractFileInfoPointer fileInfo = model()->fileInfo(rootIndex());

    const DUrl& parentUrl = fileInfo ? fileInfo->parentUrl() : DUrl::parentUrl(currentUrl());
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
    if (fileUrl.isEmpty())
        return false;
    if (systemPathManager->isSystemPath(fileUrl.path()))
        return false;
    return DListView::edit(index, trigger, event);
}

bool DFileView::select(const FMEvent &event)
{
    if(event.windowId() != windowId()) {
        return false;
    }

    const QModelIndex &index = model()->index(event.fileUrl());

    setCurrentIndex(index);

    scrollTo(index);

    return true;
}

void DFileView::selectAndRename(const FMEvent &event)
{
    bool isSelected = select(event);
    if (isSelected)
        appController->actionRename(event);
}

void DFileView::setViewMode(DFileView::ViewMode mode)
{
    if(mode != m_defaultViewMode)
        m_defaultViewMode = mode;

    switchViewMode(mode);
}

void DFileView::sortByRole(int role)
{
    if (m_headerView) {
        m_headerView->setSortIndicator(model()->roleToColumn(role), Qt::AscendingOrder);
    } else {
        model()->setSortRole(role);
        model()->sort();
    }

    FMStateManager::cacheSortState(currentUrl(), role);
}

void DFileView::sortByColumn(int column)
{
    if (m_headerView) {
        m_headerView->setSortIndicator(column, Qt::AscendingOrder);
    } else {
        model()->setSortColumn(column);
        model()->sort();
    }

    FMStateManager::cacheSortState(currentUrl(), model()->columnToRole(column));
}

void DFileView::selectAll(int windowId)
{
    if(windowId != WindowManager::getWindowId(window()))
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
        case MenuAction::ExtendView:
            static_cast<DMainWindow*>(WindowManager::getWindowById(windowId()))->fileManagerWindow()->setExtendView();
            break;
        default:
            break;
    }
}

void DFileView::sortByActionTriggered(QAction *action)
{
    DAction* dAction = static_cast<DAction*>(action);

    sortByColumn(m_sortByActionGroup->actions().indexOf(dAction));
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
    const DUrlList& urls = selectedUrls();
    FMEvent fmevent;
    fmevent = urls;
    if (urls.size() == 1){
        fmevent = urls.at(0);
    }
    fmevent = FMEvent::FileView;
    fmevent = windowId();
    fmevent = currentUrl();

    switch (event->modifiers()) {
    case Qt::NoModifier:
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
            oldSelectedUrllist = urls;

            itemDelegate()->hideAllIIndexWidget();
            clearSelection();

            model()->toggleHiddenFiles(fmevent.fileUrl());

            return;
        case Qt::Key_I:
            if (fmevent.fileUrlList().isEmpty())
                fmevent = DUrlList() << fmevent.fileUrl();

            appController->actionProperty(fmevent);

            return;
        default: break;
        }

        break;
    case Qt::ShiftModifier:
        if (event->key() == Qt::Key_Delete) {
            if (fmevent.fileUrlList().isEmpty())
                fmevent = DUrlList() << fmevent.fileUrl();

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
    case Qt::KeypadModifier:
        if (event->key() == Qt::Key_Enter && !itemDelegate()->editingIndex().isValid()) {
            appController->actionOpen(fmevent);

            return;
        }

        break;
    default: break;
    }

    DListView::keyPressEvent(event);
}

void DFileView::keyReleaseEvent(QKeyEvent *event)
{
    DListView::keyReleaseEvent(event);
}

void DFileView::showEvent(QShowEvent *event)
{
    DListView::showEvent(event);

    setFocus();
}

void DFileView::mousePressEvent(QMouseEvent *event)
{
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

            if (canShowSElectionRect())
                m_selectionRectWidget->show();

            m_pressedPos = viewport()->mapToParent(static_cast<QMouseEvent*>(event)->pos());
        }

        break;
    }
    case Qt::RightButton: {
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

        return;
    }
    default: break;
    }

    DListView::mousePressEvent(event);
}

void DFileView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_selectionRectWidget->isHidden())
        return DListView::mouseMoveEvent(event);

    const QPoint &pos = viewport()->mapToParent(static_cast<QMouseEvent*>(event)->pos());
    QRect rect;

    rect.adjust(qMin(m_pressedPos.x(), pos.x()), qMin(m_pressedPos.y(), pos.y()),
                qMax(pos.x(), m_pressedPos.x()), qMax(pos.y(), m_pressedPos.y()));

    m_selectionRectWidget->setGeometry(rect);

    setSelection(rect, QItemSelectionModel::Current|QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect);
}

void DFileView::mouseReleaseEvent(QMouseEvent *event)
{
    m_selectionRectWidget->resize(0, 0);
    m_selectionRectWidget->hide();

    DListView::mouseReleaseEvent(event);
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
    DUrl new_url = DUrl(fileInfo->scheme() + "://" + fileInfo->absolutePath()
                        + "/" + QUrl::toPercentEncoding(new_file_name));

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
//                    m_selectionRectWidget->show();

//                m_pressedPos = static_cast<QMouseEvent*>(event)->pos();
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
//        if (m_selectionRectWidget->isHidden())
//            break;

//        const QPoint &pos = static_cast<QMouseEvent*>(event)->pos();
//        QRect rect;

//        rect.adjust(qMin(m_pressedPos.x(), pos.x()), qMin(m_pressedPos.y(), pos.y()),
//                    qMax(pos.x(), m_pressedPos.x()), qMax(pos.y(), m_pressedPos.y()));

//        m_selectionRectWidget->setGeometry(rect);

////        rect.moveTopLeft(viewport()->mapFromParent(rect.topLeft()));

////        setSelection(rect, QItemSelectionModel::Current|QItemSelectionModel::Rows|QItemSelectionModel::ClearAndSelect);

//        break;
//    }
//    case QEvent::MouseButtonRelease: {
//        m_selectionRectWidget->resize(0, 0);
//        m_selectionRectWidget->hide();

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

    DListView::dragEnterEvent(event);
}

void DFileView::dragMoveEvent(QDragMoveEvent *event)
{
    if (dragDropMode() == InternalMove
        && (event->source() != this || !(event->possibleActions() & Qt::MoveAction)))
        QAbstractItemView::dragMoveEvent(event);
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

        tmp_rect.adjust(qMin(rect.left(), rect.right()), qMin(rect.top(), rect.bottom()),
                        qMax(rect.left(), rect.right()), qMax(rect.top(), rect.bottom()));

        int offset_horizontal = m_currentViewMode == ListMode ? width() : itemSizeHint().width();
        int offset_verizontal = itemSizeHint().height();

        QModelIndex index1;
        QModelIndex index2;

        if (m_currentViewMode == ListMode) {
            for (int j = tmp_rect.top(); tmp_rect.bottom() - j > 0; j += offset_verizontal) {
                index1 = indexAt(QPoint(tmp_rect.left(), j));

                if (index1.isValid())
                    break;
            }

            if (!index1.isValid()) {
                index1 = indexAt(QPoint(tmp_rect.left(), tmp_rect.bottom()));

                if (!index1.isValid()) {
                    return;
                }
            }

            for (int j = tmp_rect.bottom(); j - tmp_rect.top() > 0; j -= offset_verizontal) {
                index2 = indexAt(QPoint(tmp_rect.left(), j));

                if (index2.isValid())
                    goto selection;
            }

            index2 = indexAt(QPoint(tmp_rect.left(), tmp_rect.top()));

            if (index2.isValid())
                goto selection;

            return;
        }

        for (int j = tmp_rect.top(); tmp_rect.bottom() - j > 0; j += offset_verizontal) {
            for (int i = tmp_rect.left(); tmp_rect.right() - i > 0; i += offset_horizontal) {
                index1 = indexAt(QPoint(i, j));

                if (index1.isValid())
                    goto find_index2;
            }

            index1 = indexAt(QPoint(tmp_rect.right(), j));

            if (index1.isValid())
                goto find_index2;
        }

        for (int i = tmp_rect.left(); tmp_rect.right() - i > 0; i += offset_horizontal) {
            index1 = indexAt(QPoint(i, tmp_rect.bottom()));

            if (index1.isValid())
                goto find_index2;
        }

        index1 = indexAt(QPoint(tmp_rect.right(), tmp_rect.bottom()));

        if (index1.isValid())
            goto find_index2;

        return;

find_index2:
        for (int j = tmp_rect.bottom(); j - tmp_rect.top() > 0; j -= offset_verizontal) {
            for (int i = tmp_rect.right(); i - tmp_rect.left() > 0; i -= offset_horizontal) {
                index2 = indexAt(QPoint(i, j));

                if (index2.isValid())
                    goto selection;
            }

            index2 = indexAt(QPoint(tmp_rect.left(), j));

            if (index2.isValid())
                goto selection;
        }

        for (int i = tmp_rect.right(); i - tmp_rect.left() > 0; i -= offset_horizontal) {
            index2 = indexAt(QPoint(i, tmp_rect.top()));

            if (index2.isValid())
                goto selection;
        }

        index2 = indexAt(QPoint(tmp_rect.left(), tmp_rect.top()));

        if (index2.isValid())
            goto selection;

        return;

selection:
        selectionModel()->select(QItemSelection(index1, index2), flags);

        return;
    }

    DListView::setSelection(rect, flags);
}

QModelIndex DFileView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    QModelIndex current = currentIndex();

    if (!current.isValid()) {
        m_lastCursorIndex = DListView::moveCursor(cursorAction, modifiers);

        return m_lastCursorIndex;
    }

    if (rectForIndex(current).isEmpty()) {
        m_lastCursorIndex = model()->index(0, 0, rootIndex());

        return m_lastCursorIndex;
    }

    QModelIndex index;

    switch (cursorAction) {
    case MoveLeft:
        if (Global::keyShiftIsPressed()) {
            index = DListView::moveCursor(cursorAction, modifiers);

            if (index == m_lastCursorIndex) {
                index = index.sibling(index.row() - 1, index.column());
            }
        } else {
            index = current.sibling(current.row() - 1, current.column());
        }

        break;
    case MoveRight:
        if (Global::keyShiftIsPressed()) {
            index = DListView::moveCursor(cursorAction, modifiers);

            if (index == m_lastCursorIndex) {
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
        m_lastCursorIndex = index;

        return index;
    }

    m_lastCursorIndex = current;

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

        const QList<QRect> &geometry_list = itemDelegate()->paintGeomertyss(option, index);

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
    int size = m_iconSizes.value(m_currentIconSizeIndex);

    return QSize(size, size);
}

void DFileView::enlargeIcon()
{
    if(m_currentIconSizeIndex < m_iconSizes.count() - 1)
        ++m_currentIconSizeIndex;

    setIconSize(currentIconSize());
}

void DFileView::shrinkIcon()
{
    if(m_currentIconSizeIndex > 0)
        --m_currentIconSizeIndex;

    setIconSize(currentIconSize());
}

void DFileView::openIndex(const QModelIndex &index)
{
    qDebug() << index << model()->hasChildren(index);
    if(model()->hasChildren(index)){
        FMEvent event;

        event = model()->getUrlByIndex(index);
        event = FMEvent::FileView;
        event = windowId();
        qDebug() << event;
        emit fileSignalManager->requestChangeCurrentUrl(event);
    } else {
        emit fileService->openFile(model()->getUrlByIndex(index));
    }
}

void DFileView::keyboardSearch(const QString &search)
{
    m_keyboardSearchKeys.append(search);
    m_keyboardSearchTimer->start();
    QModelIndexList matchModelIndexListCaseSensitive = model()->match(rootIndex(), 0, m_keyboardSearchKeys, -1, Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap |
                                                                                                   Qt::MatchCaseSensitive | Qt::MatchRecursive));
    foreach (const QModelIndex& index, matchModelIndexListCaseSensitive) {
        QString absolutePath = FileInfo(model()->getUrlByIndex(index).path()).absolutePath();
        if (absolutePath == currentUrl().path()){
            setCurrentIndex(index);
            scrollTo(index);
            return;
        }
    }

    QModelIndexList matchModelIndexListNoCaseSensitive = model()->match(rootIndex(), 0, m_keyboardSearchKeys, -1, Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap |
                                                                                                   Qt::MatchRecursive));
    foreach (const QModelIndex& index, matchModelIndexListNoCaseSensitive) {
        QString absolutePath = FileInfo(model()->getUrlByIndex(index).path()).absolutePath();
        if (absolutePath == currentUrl().path()){
            setCurrentIndex(index);
            scrollTo(index);
            return;
        }
    }

}

void DFileView::stopSearch()
{
    DUrl url = currentUrl();

    if(!url.isSearchFile()) {
        return;
    }

    url.setSearchAction(DUrl::StopSearch);

    FMEvent event;

    event = url;
    event = FMEvent::FileView;
    event = windowId();

    FileServices::instance()->getChildren(event);
}

bool DFileView::setCurrentUrl(DUrl fileUrl)
{
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

    stopSearch();

    disconnect(model(), &DFileSystemModel::rowsInserted, this, &DFileView::updateContentLabel);

//    QModelIndex index = model()->index(fileUrl);

//    if(!index.isValid())
    QModelIndex index = model()->setRootUrl(fileUrl);

//    model()->setActiveIndex(index);
    setRootIndex(index);

    if (!model()->canFetchMore(index)) {
        updateContentLabel();
    }

    updateListHeaderViewProperty();

    if(info) {
        ViewModes modes = (ViewModes)info->supportViewMode();

        if(!testViewMode(modes, m_defaultViewMode)) {
            if(testViewMode(modes, IconMode)) {
                switchViewMode(IconMode);
            } else if(testViewMode(modes, ListMode)) {
                switchViewMode(ListMode);
            } else if(testViewMode(modes, ExtendMode)) {
                switchViewMode(ExtendMode);
            }
        } else {
            switchViewMode(m_defaultViewMode);
        }
    } 
    //emit currentUrlChanged(fileUrl);

    if (focusWidget() && focusWidget()->window() == window() && fileUrl.isLocalFile())
        QDir::setCurrent(fileUrl.toLocalFile());

    setSelectionMode(info->supportSelectionMode());

    const DUrl &defaultSelectUrl = DUrl(QUrlQuery(fileUrl.query()).queryItemValue("selectUrl"));

    if (defaultSelectUrl.isValid())
        oldSelectedUrllist << defaultSelectUrl;

    return true;
}

void DFileView::clearHeardView()
{
    if(m_headerView) {
        removeHeaderWidget(0);

        m_headerView = Q_NULLPTR;
    }
}

void DFileView::clearKeyBoardSearchKeys()
{
    m_keyboardSearchKeys.clear();
    m_keyboardSearchTimer->stop();
}

void DFileView::handleSelectionChanged()
{
    FMEvent event;
    event = windowId();
    int count = selectedIndexCount();

    if (count == 0){
        emit fileSignalManager->statusBarItemsCounted(event, this->count());
    }else{
        emit fileSignalManager->statusBarItemsSelected(event, count);
    }
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
    FMEvent fmEvent;
    fmEvent = windowId();
    if (rootIndex().isValid())
        emit fileSignalManager->statusBarItemsCounted(fmEvent, count());
}

void DFileView::setSelectionRectVisible(bool visible)
{
    m_selectionRectVisible = visible;
}

bool DFileView::isSelectionRectVisible() const
{
    return m_selectionRectVisible;
}

bool DFileView::canShowSElectionRect() const
{
    return m_selectionRectVisible && selectionMode() != NoSelection && selectionMode() != SingleSelection;
}

void DFileView::setContentLabel(const QString &text)
{
    if (text.isEmpty()) {
        if (m_contentLabel) {
            m_contentLabel->deleteLater();
            m_contentLabel = Q_NULLPTR;
        }

        return;
    }

    if (!m_contentLabel) {
        m_contentLabel = new QLabel(this);

        m_contentLabel->show();
        m_contentLabel.setCenterIn(this);
        m_contentLabel->setObjectName("contentLabel");
        m_contentLabel->setStyleSheet(this->styleSheet());
    }

    m_contentLabel->setText(text);
    m_contentLabel->adjustSize();
}

void DFileView::updateHorizontalOffset()
{
    if (isIconViewMode()) {
        int contentWidth = width();
        int itemWidth = itemSizeHint().width() + ICON_VIEW_SPACING * 2;
        int itemColumn = (contentWidth - ICON_VIEW_SPACING * 2.5) / itemWidth;

        m_horizontalOffset = -(contentWidth - itemWidth * itemColumn) / 2;
    } else {
        m_horizontalOffset = 0;
    }
}

void DFileView::switchViewMode(DFileView::ViewMode mode)
{
    if (m_currentViewMode == mode) {
        return;
    }

    const AbstractFileInfoPointer &fileInfo = model()->fileInfo(currentUrl());

    if (fileInfo && (fileInfo->supportViewMode() & mode) == 0) {
        return;
    }

    m_currentViewMode = mode;

    itemDelegate()->hideAllIIndexWidget();

    switch (mode) {
    case IconMode: {
        clearHeardView();
        m_columnRoles.clear();
        setIconSize(currentIconSize());
        setOrientation(QListView::LeftToRight, true);
        setSpacing(ICON_VIEW_SPACING);

        break;
    }
    case ListMode: {
        itemDelegate()->hideAllIIndexWidget();

        if(!m_headerView) {
            m_headerView = new QHeaderView(Qt::Horizontal);

            updateListHeaderViewProperty();

            m_headerView->setHighlightSections(true);
            m_headerView->setSectionsClickable(true);
            m_headerView->setSortIndicatorShown(true);
            m_headerView->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            m_headerView->setContextMenuPolicy(Qt::CustomContextMenu);

            if(selectionModel()) {
                m_headerView->setSelectionModel(selectionModel());
            }

            connect(m_headerView, &QHeaderView::sectionResized,
                    this, static_cast<void (DFileView::*)()>(&DFileView::update));
            connect(m_headerView, &QHeaderView::sortIndicatorChanged,
                    model(), &QAbstractItemModel::sort);
            connect(m_headerView, &QHeaderView::customContextMenuRequested,
                    this, &DFileView::popupHeaderViewContextMenu);
        }

        addHeaderWidget(m_headerView);

        setIconSize(QSize(LIST_VIEW_ICON_SIZE, LIST_VIEW_ICON_SIZE));
        setOrientation(QListView::TopToBottom, false);
        setSpacing(LIST_VIEW_SPACING);

        break;
    }
    case ExtendMode: {
        itemDelegate()->hideAllIIndexWidget();
        if(!m_headerView) {
            m_headerView = new QHeaderView(Qt::Horizontal);

            updateExtendHeaderViewProperty();

            m_headerView->setHighlightSections(true);
            m_headerView->setSectionsClickable(true);
            m_headerView->setSortIndicatorShown(true);
            m_headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            if(selectionModel()) {
                m_headerView->setSelectionModel(selectionModel());
            }

            connect(m_headerView, &QHeaderView::sectionResized,
                    this, static_cast<void (DFileView::*)()>(&DFileView::update));
            connect(m_headerView, &QHeaderView::sortIndicatorChanged,
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
    if (currentUrl().isNetWorkFile() || currentUrl().isSMBFile()){
        return;
    }
    DFileMenu *menu;
    const QModelIndex &index = rootIndex();

    const AbstractFileInfoPointer &info = model()->fileInfo(index);
    const QVector<MenuAction> &actions = info->menuActionList(AbstractFileInfo::SpaceArea);
    const QMap<MenuAction, QVector<MenuAction> > &subActions = info->subMenuActionList();

    QSet<MenuAction> disableList = FileMenuManager::getDisableActionList(model()->getUrlByIndex(index));

    if (!count())
        disableList << MenuAction::SelectAll;

    menu = FileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);

    DFileMenu *displayAsSubMenu = static_cast<DFileMenu*>(menu->actionAt(fileMenuManger->getActionString(MenuAction::DisplayAs))->menu());
    DFileMenu *sortBySubMenu = static_cast<DFileMenu*>(menu->actionAt(fileMenuManger->getActionString(MenuAction::SortBy))->menu());

    if (displayAsSubMenu){
        foreach (DAction* action, displayAsSubMenu->actionList()) {
            action->setActionGroup(m_displayAsActionGroup);
        }
        if (m_currentViewMode == IconMode){
            displayAsSubMenu->actionAt(fileMenuManger->getActionString(MenuAction::IconView))->setChecked(true);
        }else if (m_currentViewMode == ListMode){
            displayAsSubMenu->actionAt(fileMenuManger->getActionString(MenuAction::ListView))->setChecked(true);
        }else if (m_currentViewMode == ExtendMode){
            displayAsSubMenu->actionAt(fileMenuManger->getActionString(MenuAction::ExtendView))->setChecked(true);
        }
    }

    if (sortBySubMenu){
        foreach (DAction* action, sortBySubMenu->actionList()) {
            action->setActionGroup(m_sortByActionGroup);
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
    if(!index.isValid())
        return;

    DUrlList list = selectedUrls();
    DFileMenu* menu;

    const AbstractFileInfoPointer &info = model()->fileInfo(index);

    if (list.length() == 1) {
        const QVector<MenuAction> &actions = info->menuActionList(AbstractFileInfo::SingleFile);
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

            foreach (QString app, recommendApps) {
                DAction* action = new DAction(mimeAppsManager->DesktopObjs.value(app).getName(), 0);
                action->setProperty("app", app);
                action->setProperty("url", info->redirectedFileUrl().toLocalFile());
                openWithMenu->addAction(action);
                m_openWithActionGroup->addAction(action);
            }

            DAction* action = new DAction(fileMenuManger->getActionString(MenuAction::OpenWithCustom), 0);
            action->setData((int)MenuAction::OpenWithCustom);
            openWithMenu->addAction(action);
        }
    } else {
        bool isSystemPathIncluded = false;

        foreach (DUrl url, list) {
            const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(url);

            if (systemPathManager->isSystemPath(fileInfo->redirectedFileUrl().toLocalFile())) {
                isSystemPathIncluded = true;
            }
        }

        QVector<MenuAction> actions;
        if (isSystemPathIncluded)
            actions = info->menuActionList(AbstractFileInfo::MultiFilesSystemPathIncluded);
        else
            actions = info->menuActionList(AbstractFileInfo::MultiFiles);
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
    if (!m_headerView)
        return;

    m_headerView->setModel(Q_NULLPTR);
    m_headerView->setModel(model());
    m_headerView->setSectionResizeMode(QHeaderView::Fixed);
    m_headerView->setDefaultSectionSize(DEFAULT_HEADER_SECTION_WIDTH);
    m_headerView->setMinimumSectionSize(DEFAULT_HEADER_SECTION_WIDTH);

    int sort_role = model()->roleToColumn(FMStateManager::SortStates.value(currentUrl(), DFileSystemModel::FileDisplayNameRole));

    m_headerView->setSortIndicator(sort_role, model()->sortOrder());

    m_columnRoles.clear();

    for (int i = 0; i < m_headerView->count(); ++i) {
        m_columnRoles << model()->columnToRole(i);

        int column_width = model()->columnWidth(i);

        if (column_width >= 0) {
            m_headerView->resizeSection(i, column_width);
        } else {
            m_headerView->setSectionResizeMode(i, QHeaderView::Stretch);
        }

        const QString &column_name = model()->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();

        if (!m_columnForRoleHiddenMap.contains(column_name)) {
            m_headerView->setSectionHidden(i, !model()->columnDefaultVisibleForRole(model()->columnToRole(i)));
        } else {
            m_headerView->setSectionHidden(i, m_columnForRoleHiddenMap.value(column_name));
        }
    }

    updateColumnWidth();
}

void DFileView::updateExtendHeaderViewProperty()
{
    if(!m_headerView)
        return;
    m_headerView->setModel(Q_NULLPTR);
    m_headerView->setModel(model());
    m_headerView->setSectionResizeMode(QHeaderView::Fixed);
    m_headerView->setSectionResizeMode(0, QHeaderView::Stretch);
    m_headerView->setDefaultSectionSize(DEFAULT_HEADER_SECTION_WIDTH);
    m_headerView->setMinimumSectionSize(DEFAULT_HEADER_SECTION_WIDTH);

    m_columnRoles.clear();
    m_columnRoles << model()->columnToRole(0);
}

void DFileView::updateItemSizeHint()
{
    if(isIconViewMode()) {
        int width = iconSize().width() * 1.8;

        m_itemSizeHint = QSize(width, width * 1.1);
    } else {
        m_itemSizeHint = QSize(-1, 30);
    }
}

void DFileView::updateColumnWidth()
{
    int column_count = m_headerView->count();
    int i = 0;
    int j = column_count - 1;

    for (; i < column_count; ++i) {
        if (m_headerView->isSectionHidden(i))
            continue;

        m_headerView->resizeSection(i, model()->columnWidth(i) + LIST_MODE_RIGHT_MARGIN + TEXT_PADDING);
        break;
    }

    for (; j > 0; --j) {
        if (m_headerView->isSectionHidden(j))
            continue;

        m_headerView->resizeSection(j, model()->columnWidth(j) + LIST_MODE_RIGHT_MARGIN + TEXT_PADDING);
        break;
    }

    if (firstVisibleColumn != i) {
        if (firstVisibleColumn > 0)
            m_headerView->resizeSection(firstVisibleColumn, model()->columnWidth(firstVisibleColumn));

        firstVisibleColumn = i;
    }

    if (lastVisibleColumn != j) {
        if (lastVisibleColumn > 0)
            m_headerView->resizeSection(lastVisibleColumn, model()->columnWidth(lastVisibleColumn));

        lastVisibleColumn = j;
    }
}

void DFileView::popupHeaderViewContextMenu(const QPoint &/*pos*/)
{
    DMenu *menu = new DMenu();

    for (int i = 1; i < m_headerView->count(); ++i) {
        DAction *action = new DAction(menu);

        action->setText(model()->columnNameByRole(model()->columnToRole(i)).toString());
        action->setCheckable(true);
        action->setChecked(!m_headerView->isSectionHidden(i));

        connect(action, &DAction::triggered, this, [this, action, i] {
            m_columnForRoleHiddenMap[action->text()] = action->isChecked();

            m_headerView->setSectionHidden(i, action->isChecked());

            updateColumnWidth();
        });

        menu->addAction(action);
    }

    menu->exec();
}

void DFileView::onChildrenUpdated()
{
    for (const DUrl &url : oldSelectedUrllist) {
        selectionModel()->select(model()->index(url), QItemSelectionModel::SelectCurrent);
    }

    oldSelectedUrllist.clear();
}

void DFileView::updateContentLabel()
{
    int count = this->count();

    if (count <= 0) {
        const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(currentUrl());

        if (fileInfo->exists())
            setContentLabel(tr("Floder is empty"));
        else
            setContentLabel(tr("File has been moved or deleted"));
    } else {
        setContentLabel(QString());
    }

    connect(model(), &DFileSystemModel::rowsInserted, this, &DFileView::updateContentLabel);
}
