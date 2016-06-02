#include "dfileview.h"
#include "dfilesystemmodel.h"
#include "fileitem.h"
#include "filemenumanager.h"
#include "dfileitemdelegate.h"
#include "fileinfo.h"
#include "dfilemenu.h"
#include "dscrollbar.h"
#include "windowmanager.h"

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
#include "dfilemanagerwindow.h"
#include "../shutil/fileutils.h"
#include <dthememanager.h>

#include <QWheelEvent>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>


DWIDGET_USE_NAMESPACE

bool DFileView::CtrlIsPressed = false;


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

    setSpacing(5);
    setResizeMode(QListView::Adjust);
    setOrientation(QListView::LeftToRight, true);
    setIconSize(currentIconSize());
    setTextElideMode(Qt::ElideMiddle);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionRectVisible(true);
    setEditTriggers(QListView::EditKeyPressed);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBar(new DScrollBar);
}

void DFileView::initDelegate()
{
    setItemDelegate(new DFileItemDelegate(this));
}

void DFileView::initModel()
{
    setModel(new DFileSystemModel(this));
}

void DFileView::initConnects()
{
    connect(this, &DFileView::doubleClicked,
            this, &DFileView::openIndex);
    connect(fileSignalManager, &FileSignalManager::requestChangeCurrentUrl,
            this, &DFileView::cd);

    connect(fileSignalManager, &FileSignalManager::requestRename,
            this, static_cast<void (DFileView::*)(const FMEvent&)>(&DFileView::edit));
    connect(fileSignalManager, &FileSignalManager::requestViewSelectAll,
            this, &DFileView::selectAll);
    connect(fileSignalManager, &FileSignalManager::requestSelectFile,
            this, &DFileView::select);

    connect(m_displayAsActionGroup, &QActionGroup::triggered, this, &DFileView::dislpayAsActionTriggered);
    connect(m_sortByActionGroup, &QActionGroup::triggered, this, &DFileView::sortByActionTriggered);
    connect(m_openWithActionGroup, &QActionGroup::triggered, this, &DFileView::openWithActionTriggered);
    connect(m_keyboardSearchTimer, &QTimer::timeout, this, &DFileView::clearKeyBoardSearchKeys);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &DFileView::handleSelectionChanged);
    connect(fileSignalManager, &FileSignalManager::requestFoucsOnFileView, this, &DFileView::setFoucsOnFileView);

    connect(itemDelegate(), &DFileItemDelegate::commitData, this, &DFileView::handleCommitData);
    connect(model(), &DFileSystemModel::dataChanged, this, &DFileView::handleDataChanged);
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

    QAction *delete_action = new QAction(this);

    delete_action->setAutoRepeat(false);
    delete_action->setShortcut(QKeySequence::Delete);

    connect(delete_action, &QAction::triggered,
            this, [this] {
        if (selectedUrls().count() > 0){
            if (selectedUrls().at(0).isTrashFile()){
                FMEvent event;
                event = currentUrl();
                event = selectedUrls();
                event = windowId();
                event = FMEvent::FileView;
                fileService->deleteFiles(selectedUrls(), event);
            }else{
                fileService->moveToTrash(selectedUrls());
            }
        }
    });

    addAction(copy_action);
    addAction(cut_action);
    addAction(paste_action);
    addAction(delete_action);
}

void DFileView::initKeyboardSearchTimer()
{
    m_keyboardSearchTimer = new QTimer(this);
    m_keyboardSearchTimer->setInterval(100);
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

bool DFileView::isIconViewMode()
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

QList<int> DFileView::columnRoleList() const
{
    return m_columnRoles;
}

int DFileView::selectedIndexCount() const
{
    return selectedIndexes().count();
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

    updateViewportMargins();
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

void DFileView::setSelectedItemCount(int count)
{
    FMEvent event;
    event = windowId();
    if (count == 0){
        emit fileSignalManager->statusBarItemsCounted(event, this->count());
    }else{
        emit fileSignalManager->statusBarItemsSelected(event, count);
    }
}


void DFileView::cd(const FMEvent &event)
{
    setFocus();
    clearSelection();
    if(event.windowId() != windowId())
        return;

    const DUrl &fileUrl = event.fileUrl();

    if(fileUrl.isEmpty())
        return;


    if(setCurrentUrl(fileUrl))
    {
        FMEvent e = event;
        e = currentUrl();
        emit fileSignalManager->currentUrlChanged(e);
//        updateStatusBar();
    }

    if (FMStateManager::SortStates.contains(fileUrl)){
        sort(FMStateManager::SortStates.value(fileUrl));
    }
}

void DFileView::cdUp(const FMEvent &event)
{
    const DUrl& parentUrl = DUrl::parentUrl(currentUrl());
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

    edit(index);
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

void DFileView::select(const FMEvent &event)
{
    if(event.windowId() != windowId()) {
        return;
    }

    const QModelIndex &index = model()->index(event.fileUrl());

    selectionModel()->select(index, QItemSelectionModel::Select);

    scrollTo(index);

    appController->actionRename(event);
}

void DFileView::setViewMode(DFileView::ViewMode mode)
{
    if(mode != m_defaultViewMode)
        m_defaultViewMode = mode;

    switchViewMode(mode);
}

void DFileView::sort(int role)
{
    model()->setSortRole(role);
    model()->sort();

    FMStateManager::cacheSortState(currentUrl(), role);
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
    dAction->setChecked(true);
    MenuAction type = (MenuAction)dAction->data().toInt();
    qDebug() << action << type;
    switch(type){
        case MenuAction::Name:
            sort(DFileSystemModel::FileDisplayNameRole);
            break;
        case MenuAction::Size:
            sort(DFileSystemModel::FileSizeRole);
            break;
        case MenuAction::Type:
            sort(DFileSystemModel::FileMimeTypeRole);
            break;
        case MenuAction::CreatedDate:
            sort(DFileSystemModel::FileCreatedRole);
            break;
        case MenuAction::LastModifiedDate:
            sort(DFileSystemModel::FileLastModifiedRole);
            break;
        default:
            break;
    }
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
    if(isIconViewMode() && CtrlIsPressed) {
        if(event->angleDelta().y() > 0) {
            enlargeIcon();
        } else {
            shrinkIcon();
        }

        event->accept();
    } else {
        DListView::wheelEvent(event);
    }
}

void DFileView::keyPressEvent(QKeyEvent *event)
{
    qDebug() << event->modifiers() << event->key();
    const DUrlList& urls = selectedUrls();
    FMEvent fmevent;
    fmevent = urls;
    if (urls.size() == 1){
        fmevent = urls.at(0);
    }
    fmevent = FMEvent::FileView;
    fmevent = windowId();

    if (event->modifiers() == Qt::NoModifier){
        if (event->key() == Qt::Key_Return){
            appController->actionOpen(fmevent);
        }else if (event->key() == Qt::Key_Backspace){
            cdUp(fmevent);
        }else if (event->key() == Qt::Key_F1){
            appController->actionHelp(fmevent);
        }else if (event->key() == Qt::Key_Left){
            if (selectedIndexes().count() > 0){
                QModelIndex index = selectedIndexes().last();
                if (index.row() - 1 >= 0){
                    QModelIndex cindex = index.sibling(index.row() - 1, index.column());
                    selectionModel()->setCurrentIndex(cindex, QItemSelectionModel::SelectCurrent);
                }
                return;
            }
        }else if (event->key() == Qt::Key_Right){
            if (selectedIndexes().count() > 0){
                QModelIndex index = selectedIndexes().last();
                if ((index.row() + 1) < count()){
                    QModelIndex cindex = index.sibling((index.row() + 1), index.column());
                    if (cindex.isValid()){
                        selectionModel()->setCurrentIndex(cindex, QItemSelectionModel::SelectCurrent);
                    }
               }
               return;
            }
        }
    }else if (event->modifiers() == Qt::ControlModifier){
        CtrlIsPressed = true;
        if (event->key() == Qt::Key_Down){
            appController->actionOpen(fmevent);
        }else if (event->key() == Qt::Key_Up){
            cdUp(fmevent);
        }else if (event->key() == Qt::Key_F){
            fmevent = currentUrl();
            appController->actionctrlF(fmevent);
        }else if (event->key() == Qt::Key_L){
            fmevent = currentUrl();
            appController->actionctrlL(fmevent);
        }else if (event->key() == Qt::Key_N){
            appController->actionNewWindow(fmevent);
        }else if (event->key() == Qt::Key_H){
            model()->toggleHiddenFiles(currentUrl());
        }else if (event->key() == Qt::Key_I){
            appController->actionProperty(fmevent);
        }
    }else if (event->modifiers() == Qt::ShiftModifier){
        if (event->key() == Qt::Key_Delete){
            if (selectedUrls().size() > 0){
                fileService->deleteFiles(selectedUrls(), fmevent);
            }
        }
    }else if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)){
        qDebug() << event->modifiers();
        if (event->key() == Qt::Key_N){
            fmevent = currentUrl();
            clearSelection();
            appController->actionNewFolder(fmevent);
        }if (event->key() == Qt::Key_Question){
            fmevent = currentUrl();
            appController->actionShowHotkeyHelp(fmevent);
        }
    }else if (event->modifiers() == Qt::AltModifier){
        fmevent = currentUrl();
        if (event->key() == Qt::Key_F4){
            appController->actionExitCurrentWindow(fmevent);
        }else if (event->key() == Qt::Key_Left){
            appController->actionBack(fmevent);
        }else if (event->key() == Qt::Key_Right){
            appController->actionForward(fmevent);
        }
    }else if (event->modifiers() == Qt::KeypadModifier){
        if (event->key() == Qt::Key_Enter){
            appController->actionOpen(fmevent);
        }
    }
    DListView::keyPressEvent(event);
}

void DFileView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control){
        CtrlIsPressed = false;
    }
    DListView::keyReleaseEvent(event);
}

void DFileView::showEvent(QShowEvent *event)
{
    DListView::showEvent(event);

    setFocus();
}

void DFileView::mousePressEvent(QMouseEvent *event)
{
    m_pressed = event->pos();

    bool isEmptyArea = this->isEmptyArea(event->pos());

    if (event->button() == Qt::LeftButton) {
        setDragEnabled(!isEmptyArea);
    }

    event->ignore();

    if(!(event->buttons() & Qt::RightButton))
        DListView::mousePressEvent(event);
}

void DFileView::mouseMoveEvent(QMouseEvent *event)
{
    QRect rect(m_pressed, event->pos() + QPoint(horizontalOffset(), verticalOffset()));
    m_elasticBand = rect.normalized();
    DListView::mouseMoveEvent(event);
}

void DFileView::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = QPoint(0, 0);
    m_elasticBand = QRect();
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


    FMEvent event;
    event = fileInfo->fileUrl();
    event = windowId();
    event = FMEvent::FileView;


    if(lineEdit) {
        if (fileInfo->fileName() == lineEdit->text()){
            return;
        }

        DUrl old_url = fileInfo->fileUrl();
        DUrl new_url = DUrl(fileInfo->scheme() + "://" + fileInfo->absolutePath()
                            + "/" + lineEdit->text());

        /// later rename file.
        TIMER_SINGLESHOT(0, {
                             fileService->renameFile(old_url, new_url, event);
                         }, old_url, new_url, event)
        return;
    }

    FileIconItem *item = qobject_cast<FileIconItem*>(editor);

    if(item) {
        if (fileInfo->fileName() == item->edit->toPlainText()){
            return;
        }
        fileService->renameFile(fileInfo->fileUrl(),
                                DUrl(fileInfo->scheme() + "://" + fileInfo->absolutePath()
                                     + "/" + item->edit->toPlainText()), event);
    }
}

void DFileView::focusInEvent(QFocusEvent *event)
{
    DListView::focusInEvent(event);
    itemDelegate()->commitDataAndCloseActiveEditor();
}

void DFileView::resizeEvent(QResizeEvent *event)
{
    updateViewportMargins();

    DListView::resizeEvent(event);
}

void DFileView::paintEvent(QPaintEvent *event)
{
    DListView::paintEvent(event);
    if (selectedIndexes().count() == 0){
        updateStatusBar();
    }
}

bool DFileView::event(QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);

        const QPoint &pos = viewport()->mapFrom(this, e->pos());
        bool isEmptyArea = this->isEmptyArea(pos);

        if (e->button() == Qt::LeftButton) {
            if (isEmptyArea) {
                clearSelection();
            }
        } else if (e->button() == Qt::RightButton) {
            if (isEmptyArea) {
                clearSelection();
                showEmptyAreaMenu();
            } else {
                const QModelIndexList &list = selectedIndexes();
                const QModelIndex &index = indexAt(pos);

                if (!list.contains(index)) {
                    selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
                }

                showNormalMenu(index);
            }
        }
    }

    return DListView::event(event);
}

void DFileView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);
    if (selectedIndexes().count() >=2 && m_elasticBand.isValid()){
        QItemSelection itemSelection(selectedIndexes().first(), selectedIndexes().last());
        selectionModel()->select(itemSelection, command);
    }
}

bool DFileView::isEmptyArea(const QPoint &pos) const
{
    QModelIndex index = indexAt(pos);

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
            clearSelection();
            selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
            scrollTo(index);
            return;
        }
    }

    QModelIndexList matchModelIndexListNoCaseSensitive = model()->match(rootIndex(), 0, m_keyboardSearchKeys, -1, Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap |
                                                                                                   Qt::MatchRecursive));
    foreach (const QModelIndex& index, matchModelIndexListNoCaseSensitive) {
        QString absolutePath = FileInfo(model()->getUrlByIndex(index).path()).absolutePath();
        if (absolutePath == currentUrl().path()){
            clearSelection();
            selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
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

    QModelIndex index = model()->index(fileUrl);

    if(!index.isValid())
        index = model()->setRootUrl(fileUrl);

    model()->setActiveIndex(index);
    setRootIndex(index);

//    updateListHeaderViewProperty();

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

void DFileView::handleSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    setSelectedItemCount(selectedIndexes().count());
}

void DFileView::setFoucsOnFileView(const FMEvent &event)
{
    if (event.windowId() == windowId())
        setFocus();
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

void DFileView::updateViewportMargins()
{
    QMargins margins = viewportMargins();

    if(isIconViewMode()) {
        int contentWidth = width();

        double itemWidth = iconSize().width() * 1.95 + spacing();
        int itemColumn = contentWidth / itemWidth;
        int gapWidth = (contentWidth - itemWidth * itemColumn) / 2 + 2 * spacing();

//        m_horizontalOffset = -gapWidth;
        setViewportMargins(gapWidth, 0, 0, 0);

    } else {
        margins.setLeft(10);
        margins.setRight(10);
        setViewportMargins(margins);
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

    switch (mode) {
    case IconMode: {
        clearHeardView();
        m_columnRoles.clear();
        setIconSize(currentIconSize());
        setOrientation(QListView::LeftToRight, true);
        setSpacing(5);

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
            m_headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

            if(selectionModel()) {
                m_headerView->setSelectionModel(selectionModel());
            }

            connect(m_headerView, &QHeaderView::sectionResized,
                    this, static_cast<void (DFileView::*)()>(&DFileView::update));
            connect(m_headerView, &QHeaderView::sortIndicatorChanged,
                    model(), &QAbstractItemModel::sort);
        }

        addHeaderWidget(m_headerView);

        setIconSize(QSize(30, 30));
        setOrientation(QListView::TopToBottom, false);
        setSpacing(0);

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
        setSpacing(0);
        break;
    }
    default:
        break;
    }

    emit viewModeChanged(mode);
}

void DFileView::showEmptyAreaMenu()
{
    DFileMenu *menu;
    const QModelIndex &index = rootIndex();

    const AbstractFileInfoPointer &info = model()->fileInfo(index);
    const QVector<MenuAction> &actions = info->menuActionList(AbstractFileInfo::SpaceArea);
    const QMap<MenuAction, QVector<MenuAction> > &subActions = info->subMenuActionList();

    menu = FileMenuManager::genereteMenuByKeys(actions, FileMenuManager::getDisableActionList(model()->getUrlByIndex(index)), true, subActions);

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
        if (model()->sortRole() == DFileSystemModel::FileDisplayNameRole){
            sortBySubMenu->actionAt(fileMenuManger->getActionString(MenuAction::Name))->setChecked(true);
        }else if (model()->sortRole() == DFileSystemModel::FileSizeRole){
            sortBySubMenu->actionAt(fileMenuManger->getActionString(MenuAction::Size))->setChecked(true);
        }else if (model()->sortRole() == DFileSystemModel::FileMimeTypeRole){
            sortBySubMenu->actionAt(fileMenuManger->getActionString(MenuAction::Type))->setChecked(true);
        }else if (model()->sortRole() == DFileSystemModel::FileCreatedRole){
            sortBySubMenu->actionAt(fileMenuManger->getActionString(MenuAction::CreatedDate))->setChecked(true);
        }else if (model()->sortRole() == DFileSystemModel::FileLastModifiedRole){
            sortBySubMenu->actionAt(fileMenuManger->getActionString(MenuAction::LastModifiedDate))->setChecked(true);
        }
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
    if (list.length() == 1){
        const AbstractFileInfoPointer &info = model()->fileInfo(index);
        const QVector<MenuAction> &actions = info->menuActionList(AbstractFileInfo::SingleFile);
        const QMap<MenuAction, QVector<MenuAction> > &subActions = info->subMenuActionList();
        const QVector<MenuAction> disableList;

        menu = FileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);

        DFileMenu* openWithMenu =  qobject_cast<DFileMenu*>(menu->actionAt(1)->menu());
        if (openWithMenu){
            QString url = info->absoluteFilePath();
            QMimeType mimeType = mimeAppsManager->getMimeType(url);
            QStringList recommendApps = mimeAppsManager->MimeApps.value(MimesAppsManager::getMimeTypeByFileName(url));;
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
                action->setProperty("url", list.at(0).path());
                openWithMenu->addAction(action);
                m_openWithActionGroup->addAction(action);
            }
            DAction* action = new DAction(fileMenuManger->getActionString(MenuAction::OpenWithCustom), 0);
            action->setData((int)MenuAction::OpenWithCustom);
            openWithMenu->addAction(action);
        }
    }else{
        const AbstractFileInfoPointer &info = model()->fileInfo(index);


        bool isSystemPathIncluded = false;
        foreach (DUrl url, list) {
            if (systemPathManager->isSystemPath(url.toLocalFile())){
                isSystemPathIncluded = true;
            }
        }

        QVector<MenuAction> actions;
        if (isSystemPathIncluded)
            actions = info->menuActionList(AbstractFileInfo::MultiFilesSystemPathIncluded);
        else
            actions = info->menuActionList(AbstractFileInfo::MultiFiles);
        const QMap<MenuAction, QVector<MenuAction> > subActions;
        const QVector<MenuAction> disableList;
        menu = FileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);
    }

    FMEvent event;
    event = model()->getUrlByIndex(index);
    event = list;
    event = windowId();
    event = FMEvent::FileView;
    menu->setEvent(event);

    menu->exec();
    menu->deleteLater();
}

void DFileView::updateListHeaderViewProperty()
{
    if(!m_headerView)
        return;
    m_headerView->setModel(Q_NULLPTR);
    m_headerView->setModel(model());
    m_headerView->setSectionResizeMode(QHeaderView::Fixed);
    m_headerView->setSectionResizeMode(0, QHeaderView::Stretch);
    m_headerView->setDefaultSectionSize(200);
    m_headerView->setMinimumSectionSize(200);
    m_headerView->resizeSection(2, 100);
    m_headerView->resizeSection(3, 100);

    m_columnRoles.clear();
    for (int i = 0; i < m_headerView->count(); ++i)
        m_columnRoles << model()->getRoleByColumn(i);
}

void DFileView::updateExtendHeaderViewProperty()
{
    if(!m_headerView)
        return;
    m_headerView->setModel(Q_NULLPTR);
    m_headerView->setModel(model());
    m_headerView->setSectionResizeMode(QHeaderView::Fixed);
    m_headerView->setSectionResizeMode(0, QHeaderView::Stretch);
    m_headerView->setDefaultSectionSize(100);
    m_headerView->setMinimumSectionSize(200);

    m_columnRoles.clear();
    m_columnRoles << model()->getRoleByColumn(0);
}
