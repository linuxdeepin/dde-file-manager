/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "canvasgridview.h"

#include <QPainter>
#include <QResizeEvent>
#include <QDebug>
#include <QPainterPath>
#include <QScrollBar>
#include <QTextEdit>
#include <QUrlQuery>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QMimeData>
#include <QProcess>
#include <QApplication>
#include <QScreen>
#include <QAction>
#include <QDir>
#include <QStandardPaths>
#include <QPropertyAnimation>
#include <danchors.h>
#include <DUtil>
#include <DApplication>
#define private public
#include <private/qhighdpiscaling_p.h>

#include <durl.h>
#include <dfmglobal.h>
#include <dfilesystemmodel.h>
#include <diconitemdelegate.h>
#include <dlistitemdelegate.h>
#include <dfileviewhelper.h>
#include <dfileservices.h>
#include <dfilemenu.h>
#include <dfilemenumanager.h>
#include <dfilewatcher.h>
#include <dfmapplication.h>
#include <dfmsettings.h>
#include <dgiosettings.h>
#include <dfiledragclient.h>

#include "../util/dde/desktopinfo.h"
#include "../model/dfileselectionmodel.h"
#include "../presenter/gridmanager.h"
#include "../presenter/apppresenter.h"
#include "../presenter/display.h"
#include "../presenter/dfmsocketinterface.h"
#include "../desktop.h"
#include "../dbus/dbusdock.h"
#include "../config/config.h"
#include "backgroundhelper.h"
#include "screen/screenhelper.h"

#include "interfaces/private/mergeddesktop_common_p.h"
#include "util/xcb/xcb.h"
#include "private/canvasviewprivate.h"
#include "canvasviewhelper.h"
#include "watermaskframe.h"
#include "desktopitemdelegate.h"

#include "app/define.h"
#include "controllers/mergeddesktopcontroller.h"

std::atomic<bool> CanvasGridView::m_flag{ false };
QMap<DMD_TYPES, bool> CanvasGridView::virtualEntryExpandState;

static const QMap<int, int> kSortActions = {{MenuAction::Name, DFileSystemModel::FileDisplayNameRole}
                                            ,{MenuAction::Size, DFileSystemModel::FileSizeRole}
                                            ,{MenuAction::Type, DFileSystemModel::FileMimeTypeRole}
                                            ,{MenuAction::LastModifiedDate, DFileSystemModel::FileLastModifiedRole}
                                           };

void startProcessDetached(const QString &program,
                          const QStringList &arguments = QStringList(),
                          QIODevice::OpenMode mode = QIODevice::ReadWrite)
{
    QProcess *process = new QProcess();
    process->start(program, arguments, mode);
//    qDebug() << process->program() << process->arguments();
    process->closeReadChannel(QProcess::StandardOutput);
    process->closeReadChannel(QProcess::StandardError);
    process->connect(process, static_cast < void(QProcess::*)(int) > (&QProcess::finished),
    process, [ = ](int) {
        process->deleteLater();
    });
}

DWIDGET_USE_NAMESPACE

CanvasGridView::CanvasGridView(const QString &screen, QWidget *parent)
    : QAbstractItemView(parent)
    , d(new CanvasViewPrivate)
    , m_screenName(screen)
{
    initUI();
    initConnection();
}

CanvasGridView::~CanvasGridView()
{

}

QRect CanvasGridView::visualRect(const QModelIndex &index) const
{
    auto url = model()->getUrlByIndex(index);
    auto gridPos = GridManager::instance()->position(m_screenNum, url.toString());

    auto x = gridPos.x() * d->cellWidth + d->viewMargins.left();
    auto y = gridPos.y() * d->cellHeight + d->viewMargins.top();
    return QRect(x, y, d->cellWidth, d->cellHeight);
}

QModelIndex CanvasGridView::indexAt(const QPoint &point) const
{
    auto gridPos = gridAt(point);
    auto localFile =  GridManager::instance()->itemId(m_screenNum, gridPos.x(), gridPos.y());
    auto rowIndex = model()->index(DUrl(localFile));
    QPoint pos = QPoint(point.x() + horizontalOffset(), point.y() + verticalOffset());
    auto list = itemPaintGeomertys(rowIndex);

    for (QModelIndex &index : itemDelegate()->hasWidgetIndexs()) {
        if (index == itemDelegate()->editingIndex()) {
            QWidget *widget = itemDelegate()->editingIndexWidget();
            if (widget && widget->isVisible() && widget->geometry().contains(point)) {
                return index;
            }
            continue;
        }

        QWidget *widget = indexWidget(index);

        if (widget && widget->isVisible() && widget->geometry().contains(point)) {
            return index;
        }
    }

    for (const QRect &rect : list) {
        if (rect.contains(pos)) {
            return rowIndex;
        }
    }

    return QModelIndex();
}

void CanvasGridView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    Q_UNUSED(hint);
    QRect area = viewport()->rect();
    QRect rect = visualRect(index);

    if (rect.left() < area.left()) {
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() + rect.left() - area.left());
    } else if (rect.right() > area.right()) {
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() + qMin(
                rect.right() - area.right(), rect.left() - area.left()));
    }

    if (rect.top() < area.top()) {
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() + rect.top() - area.top());
    } else if (rect.bottom() > area.bottom()) {
        verticalScrollBar()->setValue(
            verticalScrollBar()->value() + qMin(
                rect.bottom() - area.bottom(), rect.top() - area.top()));
    }

    update();
}

QModelIndex CanvasGridView::moveCursorGrid(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
//    qDebug() << modifiers <<  d->currentCursorIndex;

    auto selectionModel = this->selectionModel();
    auto headIndex = firstIndex();
    auto tailIndex = lastIndex();

    QModelIndex current = d->currentCursorIndex;
    if (!current.isValid() || !selectionModel->isSelected(current)) {
        return headIndex;
    }
    auto url = model()->getUrlByIndex(current);

    auto pos = GridManager::instance()->position(m_screenNum, url.toString());
    auto newCoord = Coordinate(pos);

    switch (cursorAction) {
    case MoveLeft:
        while (pos.x() >= 0) {
            newCoord = newCoord.moveLeft();
            pos = newCoord.position();
            if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                break;
            }
        }
        break;
    case MoveRight:
        while (pos.x() < d->colCount) {
            newCoord = newCoord.moveRight();
            pos = newCoord.position();
            if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                break;
            }
        }
        break;
    case MovePrevious:
    case MoveUp:
        while (pos.y() >= 0 && pos.x() >= 0) {
            newCoord = newCoord.moveUp();
            pos = newCoord.position();
            if (pos.y() < 0) {
                newCoord = Coordinate(pos.x() - 1, d->rowCount - 1);
                pos = newCoord.position();
            }
            if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                break;
            }
        }
        break;
    case MoveNext:
    case MoveDown:
        while (pos.y() < d->rowCount && pos.x() < d->colCount) {
            newCoord = newCoord.moveDown();
            pos = newCoord.position();
            if (pos.y() >= d->rowCount) {
                newCoord = Coordinate(pos.x() + 1, 0);
                pos = newCoord.position();
            }
            if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                break;
            }
        }
        break;
    case MoveHome:
    case MovePageUp: {
        if (modifiers == Qt::ShiftModifier) {
            QItemSelection selection;
            while (pos.y() >= 0 && pos.x() >= 0) {
                newCoord = newCoord.moveUp();
                pos = newCoord.position();
                if (pos.y() < 0) {
                    newCoord = Coordinate(pos.x() - 1, d->rowCount - 1);
                    pos = newCoord.position();
                }
                if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                    auto localFile = GridManager::instance()->itemId(m_screenNum, pos.x(), pos.y());
//                    if(localFile.isEmpty())
//                        return QModelIndex();
                    auto index = model()->index(DUrl(localFile));

                    QItemSelectionRange selectionRange(index);
                    selection.push_back(selectionRange);
                }
            }
            QAbstractItemView::selectionModel()->select(selection, QItemSelectionModel::Select);
        }
        return headIndex;
    }
    case MoveEnd:
    case MovePageDown: {
        if (modifiers == Qt::ShiftModifier) {
            QItemSelection selection;
            while (pos.y() < d->rowCount && pos.x() < d->colCount) {
                newCoord = newCoord.moveDown();
                pos = newCoord.position();
                if (pos.y() >= d->rowCount) {
                    newCoord = Coordinate(pos.x() + 1, 0);
                    pos = newCoord.position();
                }
                if (!GridManager::instance()->isEmpty(m_screenNum, pos.x(), pos.y())) {
                    auto localFile = GridManager::instance()->itemId(m_screenNum, pos.x(), pos.y());
//                    if(localFile.isEmpty())
//                        return QModelIndex();
                    auto index = model()->index(DUrl(localFile));

                    QItemSelectionRange selectionRange(index);
                    selection.push_back(selectionRange);
                }
            }

            QAbstractItemView::selectionModel()->select(selection, QItemSelectionModel::Select);
        }
        return tailIndex;
    }
    }

    if (!d->isVaildCoordinate(newCoord)) {
        return current;
    }

    auto localFile =  GridManager::instance()->itemId(m_screenNum, pos.x(), pos.y());
    auto newIndex = model()->index(DUrl(localFile));
    if (newIndex.isValid()) {
        return newIndex;
    }

    qDebug() << selectedUrls();
    return current;
}

void CanvasGridView::updateHiddenItems()
{
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, currentUrl());
    if (!info) {
        qDebug() << "CanvasGridView::updateHiddenItems(): currentUrl() scheme is not supported";
        return;
    }

    QList<DAbstractFileInfoPointer> infoList = DFileService::instance()->getChildren(this, currentUrl(),
                                                                                     QStringList(), model()->filters());
    auto existItems = GridManager::instance()->itemIds(m_screenNum);

    QStringList items;
    for (const DAbstractFileInfoPointer &info : infoList) {
        QString hideItem = info->fileUrl().toString();
        existItems.removeAll(hideItem);
        if (!GridManager::instance()->contains(m_screenNum, hideItem)) {
            GridManager::instance()->add(m_screenNum, hideItem);
        }
    }

    for (auto nonexistItem : existItems) {
        GridManager::instance()->remove(m_screenNum, nonexistItem);
    }

    if (GridManager::instance()->shouldArrange()) {
        GridManager::instance()->reArrange();
    }
}

void CanvasGridView::setGeometry(const QRect &rect)
{
#if 0 //old
    if (parentWidget()) {
        QAbstractItemView::setGeometry(QRect(0, 0, rect.width(), rect.height()));
    } else {
        QAbstractItemView::setGeometry(rect);
    }
#endif
    //!防止获取到的屏幕区域是0x0的时候崩溃
    if (rect.size().width() < 1 || rect.size().height() < 1){
        return;
    }
    else {
        QAbstractItemView::setGeometry(rect);
        updateCanvas();
        d->waterMaskFrame->updatePosition();
    }
}

void CanvasGridView::delayArrage(int ms)
{
    static QTimer *arrangeTimer = nullptr;
    if (arrangeTimer != nullptr){
        arrangeTimer->stop();
        delete arrangeTimer;
        arrangeTimer = nullptr;
        qDebug() << "reset timer" << m_screenNum;
    }
    if (ms < 1){
        d->resortCount++;
        qDebug() << "beging sort " << arrangeTimer << m_screenNum
                 << "resortCount" << d->resortCount;
        model()->setEnabledSort(true);
        model()->sort();
        return;
    }

    arrangeTimer = new QTimer;
    connect(arrangeTimer,&QTimer::timeout,[=](){
        arrangeTimer->stop();
        d->resortCount++;
        qDebug() << "beging sort " << arrangeTimer << m_screenNum
                 << "resortCount" << d->resortCount;
        model()->setEnabledSort(true);
        model()->sort();
    });
    arrangeTimer->start(ms);
}

DUrl CanvasGridView::currentCursorFile() const
{
    DUrl ret;
    DAbstractFileInfoPointer fp = model()->fileInfo(d->currentCursorIndex);
    if (fp){
        ret = fp->fileUrl();
    }
    return ret;
}

void CanvasGridView::syncIconLevel(int level)
{
    if (itemDelegate()->iconSizeLevel() == level) {
        return;
    }

    itemDelegate()->setIconSizeByIconSizeLevel(level);
    updateCanvas();
}

WId CanvasGridView::winId() const
{
    if (isTopLevel()) {
        return QAbstractItemView::winId();
    } else {
        return topLevelWidget()->winId();
    }
}

bool CanvasGridView::autoMerge() const
{
    return GridManager::instance()->autoMerge();
}

void CanvasGridView::setAutoMerge(bool enabled)
{
    GridManager::instance()->setAutoMerge(enabled);
    if (enabled) {
        //this->setRootUrl(DUrl(DFMMD_ROOT MERGEDDESKTOP_FOLDER));
        //刷新虚拟路径时是先查看是否已有展开状态
        DUrl virtualExpandUrl = GridManager::instance()->getCurrentVirtualExpandUrl();
        if(virtualExpandUrl.isEmpty()){
            this->setRootUrl(DUrl(DFMMD_ROOT MERGEDDESKTOP_FOLDER));
        }else {
            this->setRootUrl(virtualExpandUrl);
        }
    } else {
        // sa
        QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
        DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

        if (!QDir(desktopPath).exists()) {
            QDir::home().mkpath(desktopPath);
        }

        //复位成未展开的状态
        DUrl clearUrl;
        GridManager::instance()->setCurrentVirtualExpandUrl(DUrl(QUrl()));
        this->setRootUrl(desktopUrl);
    }
}

void CanvasGridView::toggleAutoMerge(bool enabled)
{
    if (enabled == GridManager::instance()->autoMerge()) return;

    setAutoMerge(enabled);
}

// please make sure the passed \a url argument is a valid virtual entry url.
void CanvasGridView::toggleEntryExpandedState(const DUrl &url)
{
    // just some simple check
    if (!url.isValid() || url.scheme() != DFMMD_SCHEME) {
        return;
    }

    clearSelection();

    // toggle expand state
    DMD_TYPES toggleType = MergedDesktopController::entryTypeByName(url.fileName());
    virtualEntryExpandState[toggleType] = !virtualEntryExpandState[toggleType];
    bool isExpand = virtualEntryExpandState[toggleType];

    // construct fragment which indicated the expanded entries
    QStringList expandedEntries;
    int possibleChildCount = 0;
    bool onlyExpandShowClickedEntry = false;
    for (unsigned int i = DMD_FIRST_TYPE; i <= DMD_ALL_TYPE; i++) {
        DMD_TYPES oneType = static_cast<DMD_TYPES>(i);
        if (oneType != DMD_FOLDER && virtualEntryExpandState[oneType]) {
            expandedEntries.append(MergedDesktopController::entryNameByEnum(oneType));
        }

        // check if icon cound is greater than desktop grid count
        possibleChildCount += 1; // 1: the virtual entry icon
        if (isExpand && virtualEntryExpandState[oneType]) {
            DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, MergedDesktopController::getVirtualEntryPath(oneType));
            if (info) {
                possibleChildCount += info->filesCount();
            }
        }
        if (possibleChildCount > GridManager::instance()->gridCount()) {
            onlyExpandShowClickedEntry = true;
            break;
        }
    }

    // prepare root url
    DUrl targetUrl(DFMMD_ROOT MERGEDDESKTOP_FOLDER);

    if (!expandedEntries.isEmpty()) {
        targetUrl.setFragment(onlyExpandShowClickedEntry ? MergedDesktopController::entryNameByEnum(toggleType) : expandedEntries.join(','));
    }

    // set root url (which will update the view)
    this->setRootUrl(targetUrl);
    //更新其他canvas的model自动整理数据。 todo:考虑优化没有扩展到的不用刷新
    QMap<QString, DUrl> mergeUpInfo;
    mergeUpInfo.insert(m_screenName, targetUrl);
    emit GridManager::instance()->sigSyncOperation(GridManager::soAutoMergeUpdate,QVariant::fromValue(mergeUpInfo));

}

void CanvasGridView::updateEntryExpandedState(const DUrl &url)
{
    this->setRootUrl(url);
}

QModelIndex CanvasGridView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    // Do not allow move when hold ctrl
    if (modifiers == Qt::ControlModifier) {
        return d->currentCursorIndex;
    }

    auto current = d->currentCursorIndex;
    if (!current.isValid()) {
        current = firstIndex();
        d->currentCursorIndex = current;
        return current;
    }

    if (rectForIndex(current).isEmpty()) {
        qCritical() << "current never empty" << current;
        d->currentCursorIndex = firstIndex();
        return d->currentCursorIndex;
    }

    QModelIndex index = moveCursorGrid(cursorAction, modifiers);

    if (index.isValid()) {
        d->currentCursorIndex = index;
        return index;
    }

    d->currentCursorIndex = current;
    return current;
}

int CanvasGridView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int CanvasGridView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

bool CanvasGridView::isIndexHidden(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return false;
}

void CanvasGridView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    setSelection(rect, command, false);
}

QRegion CanvasGridView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion region;
    auto selectedList = selection.indexes();
    for (auto &index : selectedList) {
        region = region.united(QRegion(visualRect(index)));
    }
    return region;
}

void CanvasGridView::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    QAbstractItemView::mouseMoveEvent(event);

    auto curPos = event->pos();
    QRect selectRect;

    if (d->showSelectRect) {
        selectRect.setLeft(qMin(curPos.x(), d->lastPos.x()));
        selectRect.setTop(qMin(curPos.y(), d->lastPos.y()));
        selectRect.setRight(qMax(curPos.x(), d->lastPos.x()));
        selectRect.setBottom(qMax(curPos.y(), d->lastPos.y()));
        d->selectRect = selectRect.normalized();
    }

    if (d->showSelectRect) {
        update();
        setState(DragSelectingState);
        auto command = QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect;
        setSelection(selectRect, command, true);
    }
}

void CanvasGridView::mousePressEvent(QMouseEvent *event)
{
    auto index = indexAt(event->pos());

    d->mousePressed = true;

    bool leftButtonPressed = event->button() == Qt::LeftButton;
    bool showSelectFrame = leftButtonPressed;
    showSelectFrame &= !index.isValid();
    d->showSelectRect = showSelectFrame;
    d->lastPos = event->pos();

    bool isEmptyArea = !index.isValid();

    itemDelegate()->commitDataAndCloseActiveEditor();

    if (isEmptyArea) {
        if (!DFMGlobal::keyCtrlIsPressed() && !DFMGlobal::keyShiftIsPressed()) {
            itemDelegate()->hideNotEditingIndexWidget();
            QAbstractItemView::setCurrentIndex(QModelIndex());
            clearSelection();
        }
    }

    d->beforeMoveSelection = selectionModel()->selection();

    bool isselected = isSelected(index);
    QAbstractItemView::mousePressEvent(event);

    if (leftButtonPressed && isselected && event->modifiers() == Qt::ControlModifier) {
        setProperty("lastPressedIndex", index);
        selectionModel()->select(QItemSelection (index, index), QItemSelectionModel::Select);
    }

    if (leftButtonPressed) {
        d->currentCursorIndex = index;

        if (!isEmptyArea) {
            const DUrl &url = model()->getUrlByIndex(index);
            DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
            if (info && info->isVirtualEntry()) {
                toggleEntryExpandedState(url);
            }
        }
    }
    update();
}

void CanvasGridView::mouseReleaseEvent(QMouseEvent *event)
{
    QAbstractItemView::mouseReleaseEvent(event);
    d->mousePressed = false;
    if (d->showSelectRect && d->selectRect.isValid()) {
        d->showSelectRect = false;
        d->selectRect = QRect();
//        update(d->selectRect);
    }

    QModelIndex index = property("lastPressedIndex").toModelIndex();
    if (index.isValid() && DFMGlobal::keyCtrlIsPressed() && index == indexAt(event->pos()) && isSelected(index)) {
        selectionModel()->select(QItemSelection (index, index), QItemSelectionModel::Deselect);
        setProperty("lastPressedIndex", QModelIndex());
    }

    update();
}

void CanvasGridView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    QPersistentModelIndex persistent = index;
    emit doubleClicked(persistent);
    if ((event->button() == Qt::LeftButton) && !edit(persistent, DoubleClicked, event)
            && !style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, nullptr, this)) {
        emit activated(persistent);
    }
}

void CanvasGridView::wheelEvent(QWheelEvent *event)
{
    if (DFMGlobal::keyCtrlIsPressed()) {
        if (event->angleDelta().y() > 0) {
            increaseIcon();
        } else {
            decreaseIcon();
        }
        QThread::msleep(200);
        event->accept();
    }
}

void CanvasGridView::keyPressEvent(QKeyEvent *event)
{
    if (Q_UNLIKELY(DFMApplication::appObtuselySetting()->value("ApplicationAttribute", "DisableDesktopShortcuts", false).toBool())) {
        bool specialShortcut = false;
        if (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::KeypadModifier) {
            switch (event->key()) {
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                specialShortcut = true;
                break;
            default:
                return;
            }
        }

        if (!specialShortcut) {
            return;
        }
    }

    QMap<QString, DUrl> selectUrlsMap;
    auto rootUrl = model()->rootUrl();
    bool canDeleted = true;
    for (const QModelIndex &index : selectionModel()->selectedIndexes()) {
        auto url = model()->getUrlByIndex(index);
        if (url.isEmpty()) {
            canDeleted = false;
            continue;
        }
        const DAbstractFileInfoPointer fileInfo = model()->fileInfo(index);
        if (fileInfo && !fileInfo->isVirtualEntry()) {
            selectUrlsMap.insert(url.toString(), url);
        }
    }
    selectUrlsMap.remove(rootUrl.toString());

    const DUrlList &selectUrls = selectUrlsMap.values();

    switch (event->modifiers()) {
    case Qt::NoModifier:
        switch (event->key()) {
        case Qt::Key_F1: {
            class PublicApplication : public DApplication
            {
            public:
                using  DApplication::handleHelpAction;
            };

            QString app_name = qApp->applicationName();
            qApp->setApplicationName("dde");
            reinterpret_cast<PublicApplication *>(DApplication::instance())->handleHelpAction();
            qApp->setApplicationName(app_name);
            break;
        }
        case Qt::Key_Tab: {
            this->selectionModel()->clear();
            QKeyEvent downKey(QEvent::KeyPress, Qt::Key_Down,  Qt::NoModifier);
            QCoreApplication::sendEvent(this, &downKey);
            break;
        }
        default:
            break;
        }
    // fall through
    case Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (!itemDelegate()->editingIndex().isValid()) {
                for (const DUrl &url : selectUrlsMap) {
                    openUrl(url);
                }
                return;
            }
            break;
        case Qt::Key_F5:
            model()->refresh();
            return;
        case Qt::Key_Delete:
            if (canDeleted && !selectUrlsMap.contains(rootUrl.toString()) && !selectUrls.isEmpty()) {
                DFileService::instance()->moveToTrash(this, selectUrls);
            }
            break;
        case Qt::Key_Space: {
            QStringList urls = GridManager::instance()->itemIds(m_screenNum);
            DUrlList entryUrls;
            foreach (QString url, urls) {
                entryUrls << DUrl(url);
            }
            DFMGlobal::showFilePreviewDialog(selectUrls, entryUrls);
        }
        break;
        default:
            break;
        }
        break;

    case Qt::ShiftModifier:
        if (event->key() == Qt::Key_Delete) {
            if (!canDeleted || selectUrls.isEmpty()) {
                return;
            }

            DFileService::instance()->deleteFiles(this, selectUrls);

            return;
        } else if (event->key() == Qt::Key_T) {
// Open           appController->actionOpenInTerminal(fmevent);
            return;
        }
        break;

    case Qt::ControlModifier:
        switch (event->key()) {
        case Qt::Key_Minus:
            decreaseIcon();
            return;
        case Qt::Key_Equal:
            increaseIcon();
            return;
        case Qt::Key_H: {

            ///###: record whether show files which were starting with ".";
            bool whetherShowHiddenFiles{ GridManager::instance()->getWhetherShowHiddenFiles() };
            GridManager::instance()->setWhetherShowHiddenFiles(!whetherShowHiddenFiles);

            itemDelegate()->hideAllIIndexWidget();
            clearSelection();
            model()->toggleHiddenFiles(rootUrl);
            updateHiddenItems();

            if (GridManager::instance()->getWhetherShowHiddenFiles()) {
                update();
            }

            return;
        }
        case Qt::Key_I:
            DFMGlobal::showPropertyDialog(nullptr, selectUrls);
            return;
        case Qt::Key_M:{
            if (Q_UNLIKELY(DFMApplication::appObtuselySetting()->value("ApplicationAttribute", "DisableDesktopContextMenu", false).toBool())) {
                return;
            }

            QModelIndexList indexList = selectionModel()->selectedIndexes();
            bool isEmptyArea = indexList.empty();
            Qt::ItemFlags flags;

            if (isEmptyArea) {
                flags = model()->flags(rootIndex());

                if (!flags.testFlag(Qt::ItemIsEnabled)) {
                    return;
                }
            } else {
                const QModelIndex &index = indexList.first();
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
                const QModelIndex &index = indexList.first();
                showNormalMenu(index, flags);
            }
            return;
        }
        default:
            break;
        }
        break;

    case Qt::ControlModifier | Qt::ShiftModifier:

        if (event->key() == Qt::Key_N) {
            if (itemDelegate()->editingIndex().isValid()) {
                return;
            }
//            clearSelection();
//            appController->actionNewFolder(fmevent);
            return;
        }

        if (event->key() == Qt::Key_Question) {
//            appController->actionShowHotkeyHelp(fmevent);
            return;
        }
        if (event->key() == Qt::Key_Plus) {
            increaseIcon();
            return;
        }
        break;

    default:
        break;
    }

    QAbstractItemView::keyPressEvent(event);

//    DUtil::TimerSingleShot(10, [this]() {
//        auto index = d->currentCursorIndex;
//        auto marginWidth = d->cellHeight;
//        auto rect = visualRect(index).marginsAdded(QMargins(marginWidth, marginWidth, marginWidth, marginWidth));
//        repaint(rect);
//    });
    update();
}

void CanvasGridView::dragEnterEvent(QDragEnterEvent *event)
{
    if (DFileDragClient::checkMimeData(event->mimeData())) {
        event->acceptProposedAction();
        DFileDragClient::setTargetUrl(event->mimeData(), currentUrl());
        return;
    }

    if (event->source()) {
        //drag进入后，源是view的话设置this view的选择为drag数据，用于处理多个view之间的drag
        CanvasGridView *sourceView = dynamic_cast<CanvasGridView *>(event->source());
        if(sourceView){
            QList<DUrl> urls;
            if (event->mimeData()){
                for (const QUrl &qurl : event->mimeData()->urls())
                    urls << qurl;
            }
            this->select(urls);
        }

        if (!GridManager::instance()->shouldArrange()) {
            d->startDodge = true;
        }
        itemDelegate()->hideNotEditingIndexWidget();
    }

    d->fileViewHelper->preproccessDropEvent(event);

    if (event->mimeData()->hasFormat("XdndDirectSave0")) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return;
    }

    QAbstractItemView::dragEnterEvent(event);
}

void CanvasGridView::dragMoveEvent(QDragMoveEvent *event)
{
    d->dodgeDelayTimer.stop();
    d->dragTargetGrid = QPoint(-1, -1);

    auto pos = event->pos();
    auto hoverIndex = indexAt(event->pos());

    auto startDodgeAnimation = [ = ]() {
        d->dragTargetGrid.setX((pos.x() - d->viewMargins.left()) / d->cellWidth);
        d->dragTargetGrid.setY((pos.y() - d->viewMargins.top()) / d->cellHeight);

        // FIXME: out of border???
        auto localeFile = GridManager::instance()->itemId(m_screenNum, d->dragTargetGrid);
        if (!localeFile.isEmpty() && !d->dodgeAnimationing) {
            d->dodgeDelayTimer.start();
        }

        d->fileViewHelper->preproccessDropEvent(event);
        if (!hoverIndex.isValid()) {
            if (DFileDragClient::checkMimeData(event->mimeData())) {
                event->acceptProposedAction();
                DFileDragClient::setTargetUrl(event->mimeData(), currentUrl());
            } else {
                event->accept();
            }
        }
    };

    if (hoverIndex.isValid()) {
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(hoverIndex);
        CanvasGridView *view = dynamic_cast<CanvasGridView*>(event->source());
        if (fileInfo) {
            //old
            //if (event->source() == this && !DFMGlobal::keyCtrlIsPressed()) {
            //new
            if (view && !DFMGlobal::keyCtrlIsPressed()) {
                event->setDropAction(Qt::MoveAction);
            }
            //end

            bool canDrop = fileInfo->canDrop();
            canDrop = fileInfo->isDir() && !fileInfo->isWritable();
            canDrop = fileInfo->supportedDropActions().testFlag(event->dropAction());
            if (!fileInfo->canDrop() || (fileInfo->isDir() && !fileInfo->isWritable()) ||
                    !fileInfo->supportedDropActions().testFlag(event->dropAction())) {
                // not support drag
                event->ignore();
            } else {
                if (DFileDragClient::checkMimeData(event->mimeData())) {
                    event->acceptProposedAction();
                    DFileDragClient::setTargetUrl(event->mimeData(), fileInfo->fileUrl());
                } else {
                    event->accept();
                }

                return;
            }
        }
    }

    if (!GridManager::instance()->shouldArrange()) {   //自定义
        startDodgeAnimation();
    }
    else if (GridManager::instance()->autoArrange()){ //自动排列的drag处理
        d->fileViewHelper->preproccessDropEvent(event);
        if (!hoverIndex.isValid()) {
            if (DFileDragClient::checkMimeData(event->mimeData())) {
                event->acceptProposedAction();
                DFileDragClient::setTargetUrl(event->mimeData(), currentUrl());
            } else {
                event->accept();
            }
        }
    }
    update();
}

void CanvasGridView::dragLeaveEvent(QDragLeaveEvent *event)
{
    CanvasGridView::m_flag.store(false, std::memory_order_release);
    //drag离开后，设置this view的选择为空，用于处理多个view之间的drag
    select({});

    d->dodgeDelayTimer.stop();
    d->startDodge = false;
    d->dragTargetGrid = QPoint(-1, -1);
    QAbstractItemView::dragLeaveEvent(event);
    update();
}

void CanvasGridView::dropEvent(QDropEvent *event)
{
    CanvasGridView::m_flag.store(false, std::memory_order_release);

    d->dodgeDelayTimer.stop();
    d->startDodge = false;
    d->dragTargetGrid = QPoint(-1, -1);

    QModelIndex targetIndex = indexAt(event->pos());

    QSet<QString> selectLocalFiles;
    auto selects = selectionModel()->selectedIndexes();
    bool dropOnSelf = false;
    for (auto index : selects) {
        auto info = model()->fileInfo(index);

        if (!info) {
            continue;
        }

        if (targetIndex == index) {
            dropOnSelf = true;
        }

        selectLocalFiles << info->fileUrl().toString();
    }

    DAbstractFileInfoPointer targetInfo = model()->fileInfo(indexAt(event->pos()));
    if (!targetInfo || dropOnSelf) {
        targetInfo = model()->fileInfo(rootIndex());
    }

    CanvasGridView *sourceView = dynamic_cast<CanvasGridView*>(event->source());
    if (sourceView && !DFMGlobal::keyCtrlIsPressed()) {
        event->setDropAction(Qt::MoveAction);
    } else {
        d->fileViewHelper->preproccessDropEvent(event);
    }

    if (event->mimeData()->property("IsDirectSaveMode").toBool()) {
        event->setDropAction(Qt::CopyAction);

        const QModelIndex &index = indexAt(event->pos());
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(index.isValid() ? index : rootIndex());

        if (fileInfo && fileInfo->fileUrl().isLocalFile()) {
            if (fileInfo->isDir()) {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->fileUrl());
            } else {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->parentUrl());
            }
        }

        event->accept(); // yeah! we've done with XDS so stop Qt from further event propagation.
    } else {
        if (event->dropAction() == Qt::MoveAction) {
            QModelIndex dropIndex = indexAt(gridRectAt(event->pos()).center());
            if (sourceView && (!dropIndex.isValid() || dropOnSelf)) {
                //自动排列和自动整理不允许屏幕间拖动
                if (GridManager::instance()->shouldArrange()) {
                    return;
                }

                auto point = event->pos();
                auto row = (point.x() - d->viewMargins.left()) / d->cellWidth;
                auto col = (point.y() - d->viewMargins.top()) / d->cellHeight;

                QList<QUrl> urls = event->mimeData()->urls();
                for (auto url : urls){
                    selectLocalFiles << url.toString();
                }

                //同屏移动
                if (sourceView == this){
                    //获取焦点
                    QString current = currentCursorFile().toString();
                    qDebug() << "move " << m_screenNum << "focus" << current << "count" << selectLocalFiles.size();
                    GridManager::instance()->move(m_screenNum, selectLocalFiles.toList(), current, row, col);
                }
                else {  //夸屏移动
                    //获取源屏幕的焦点
                    QString current = sourceView->currentCursorFile().toString();
                    qDebug() << "move form" << sourceView->screenNum() << "to" << m_screenNum
                             << "focus" << current << selectLocalFiles.size();
                    GridManager::instance()->move(sourceView->screenNum(),m_screenNum, selectLocalFiles.toList(), current, row, col);

                    //清除源view的选中状态
                    sourceView->select({});
                }

                setState(NoState);
                itemDelegate()->hideNotEditingIndexWidget();
                return;
            }

#if 0   //!错误的实现方法
            if (sourceView == nullptr && (!dropIndex.isValid() || dropOnSelf) ){
                //非桌面间拖入，只处理自定义
                if (!GridManager::instance()->shouldArrange()) {
                    auto point = event->pos();
                    auto row = (point.x() - d->viewMargins.left()) / d->cellWidth;
                    auto col = (point.y() - d->viewMargins.top()) / d->cellHeight;
                    QPoint pos(row,col);
                    QString item = GridManager::instance()->itemId(m_screenNum, pos);

                    QList<QUrl> urls = event->mimeData()->urls();
                    qDebug() << "drop" << urls << "pos" << pos
                             << "item:" << item;
                    if (item.isEmpty())
                    {
                        //todo 设置坐标
                        qDebug() << "todo user mode add files";
                    }else { //后面的需要 //!不是在这处理，后续调研 todo
                        //目标文件item是否可打开urls
                        //不能打开则交换位子
                    }
                }
            }
#endif
        }

        if (!targetIndex.isValid()) {
            targetIndex = rootIndex();
        }

        if (model()->supportedDropActions() & event->dropAction() && model()->flags(targetIndex) & Qt::ItemIsDropEnabled) {
            const Qt::DropAction action = event->dropAction();
            if (model()->dropMimeData(event->mimeData(), action, targetIndex.row(), targetIndex.column(), targetIndex)) {
                if (action != event->dropAction()) {
                    event->setDropAction(action);
                    event->accept();
                } else {
                    event->acceptProposedAction();
                }
            }
        }

        setState(NoState);
        viewport()->update();
    }

    if (DFileDragClient::checkMimeData(event->mimeData())) {
        event->acceptProposedAction();
        DFileDragClient::setTargetUrl(event->mimeData(), model()->getUrlByIndex(targetIndex));

        // DFileDragClient deletelater() will be called after connection destroyed
        DFileDragClient *c = new DFileDragClient(event->mimeData());
        DUrlList urlList = DUrl::fromQUrlList(event->mimeData()->urls());
        connect(c, &DFileDragClient::stateChanged, this, [this, urlList](DFileDragState state) {
            if (state == Finished) {
                select(urlList);
            }
        });
    }
}

void CanvasGridView::paintEvent(QPaintEvent *event)
{
//    if (d->_debug_profiler) {
//        qDebug() << "start repaint" << event->rect()  << event->region().rectCount();
//        auto currentTime = QTime::currentTime().msecsSinceStartOfDay();
//        auto deta = currentTime - d->lastRepaintTime;
//        if (deta < 500) {
//        return;
//        }
//        d->lastRepaintTime = currentTime;
//    }

    QPainter painter(viewport());
    auto repaintRect = event->rect();
    painter.setRenderHints(QPainter::HighQualityAntialiasing);

    auto option = viewOptions();
    option.textElideMode = Qt::ElideMiddle;

    const QModelIndex current = d->currentCursorIndex;
    const QAbstractItemModel *itemModel = this->model();
    const DFileSelectionModel *selections = this->selectionModel();
    const bool focusEnabled = false;
    const bool focus = (hasFocus() || viewport()->hasFocus()) && current.isValid() && focusEnabled;
    const QStyle::State state = option.state;
    const QAbstractItemView::State viewState = this->state();
    const bool enabled = (state & QStyle::State_Enabled) != 0;

    painter.setBrush(QColor(255, 0, 0, 0));

    if (d->_debug_show_grid) {
        painter.save();
        if (model()) {
            for (int i = 0; i < d->colCount * d->rowCount; ++i) {
                auto  pos = d->indexCoordinate(i).position();
                auto x = pos.x() * d->cellWidth + d->viewMargins.left();
                auto y = pos.y() * d->cellHeight + d->viewMargins.top();

                auto rect =  QRect(x, y, d->cellWidth, d->cellHeight);

                int rowMode = pos.x() % 2;
                int colMode = pos.y() % 2;
                auto color = (colMode == rowMode) ? QColor(0, 0, 255, 32) : QColor(255, 0, 0, 32);
                painter.fillRect(rect, color);

                if (pos == d->dragTargetGrid) {
                    painter.fillRect(rect, Qt::green);
                }
                painter.setPen(QPen(Qt::red, 2));
                painter.drawText(rect, QString("%1-%2").arg(pos.x()).arg(pos.y()));
            }
        }
        painter.restore();
    }

    DUrlList selecteds;
    if (d->dodgeAnimationing || d->startDodge) {
        selecteds = selectedUrls();
    }

//    qDebug() << d->dragIn << d->dodgeAnimationing;
    if (d->startDodge) {
        auto currentMousePos = mapFromGlobal(QCursor::pos());
        auto hoverIndex = indexAt(currentMousePos);
        auto url = model()->getUrlByIndex(hoverIndex);

        if (selecteds.contains(url)
                || (d->dodgeAnimationing && d->dodgeItems.contains(url.toString()))) {

        } else {
            if (hoverIndex.isValid() && hoverIndex != d->currentCursorIndex) {
                QPainterPath path;
                auto lastRect = visualRect(hoverIndex);
                path.addRoundRect(lastRect, 4, 4);
                painter.fillPath(path, QColor(43, 167, 248, 255 * 3 / 10));
                painter.strokePath(path, QColor(30, 126, 255, 255 * 2 / 10));
            }
        }
    }

    QStringList repaintLocalFiles;
    for (int x = 0; x < d->colCount; ++x) {
        for (int y = 0; y < d->rowCount; ++y) {
            auto localFile = GridManager::instance()->itemId(m_screenNum, x, y);
            if (!localFile.isEmpty()) {
                repaintLocalFiles << localFile;
            }
        }
    }

    auto overlayItems = GridManager::instance()->overlapItems(m_screenNum);
    for (int i = 0; i < 10 && i < overlayItems.length(); ++i) {
        auto localFile = overlayItems.value(i);
        if (!localFile.isEmpty()) {
            repaintLocalFiles << localFile;
        }
    }

//    int drawCount = 0;
    for (auto &localFile : repaintLocalFiles) {
        auto url = DUrl(localFile);
        // hide selected if draw animation
        if ((d->dodgeAnimationing || d->startDodge) && selecteds.contains(url)) {
//            qDebug() << "skip drag select" << url;
            continue;
        }


        if (d->dodgeAnimationing && d->dodgeItems.contains(localFile)) {
//            qDebug() << "skip  dragMoveItems" << localFile;
            continue;
        }

        auto index = model()->index(url);
        if (!index.isValid()) {
//            qDebug() << "skip index.isValid";
            continue;
        }
        option.rect = visualRect(index);

        bool needflash = false;
        for (auto &rr : event->region().rects())
            if (rr.intersects(option.rect)) {
                needflash = true;
            }

        if (!needflash) {
//            qDebug() << "skip !needflash";
            continue;
        }


        if (!repaintRect.intersects(option.rect)) {
//            qDebug() << "skip !repaintRect.intersects(option.rect)";
            continue;
        }

        option.rect = option.rect.marginsRemoved(d->cellMargins);
        option.state = state;
        if (selections && selections->isSelected(index)) {
            option.state |= QStyle::State_Selected;
        }
        if (enabled) {
            QPalette::ColorGroup cg;
            if ((itemModel->flags(index) & Qt::ItemIsEnabled) == 0) {
                option.state &= ~QStyle::State_Enabled;
                cg = QPalette::Disabled;
            } else {
                cg = QPalette::Normal;
            }
            option.palette.setCurrentColorGroup(cg);
        }
        if (focus && current == index) {
            option.state |= QStyle::State_HasFocus;
            if (viewState == EditingState) {
                option.state |= QStyle::State_Editing;
            }
        }
        option.state &= ~QStyle::State_MouseOver;

        painter.save();
        if (d->_debug_show_grid) {
            for (auto rect : itemPaintGeomertys(index)) {
                painter.setPen(Qt::red);
                painter.drawRect(rect);
            }
        }

        this->itemDelegate()->paint(&painter, option, index);
        DAbstractFileInfoPointer info = model()->fileInfo(index);
        if (info && info->scheme() == DFMMD_SCHEME && info->isVirtualEntry()) {
            DMD_TYPES oneType = MergedDesktopController::entryTypeByName(info->fileName());
            if (virtualEntryExpandState[oneType]) {
                // do draw mask here
                static QIcon expandMaskIcon = QIcon::fromTheme("folder-stack-mask");
                const QRect itemRect = itemIconGeomerty(index);
                QPixmap pixmap = DFMStyledItemDelegate::getIconPixmap(expandMaskIcon, itemRect.size(), devicePixelRatioF());
                painter.drawPixmap(itemRect.topLeft(), pixmap);
            }
        }
        painter.restore();
    }

    // draw select rect copy from QListView::paintEvent
    if (d->showSelectRect && d->selectRect.isValid()) {
        QStyleOptionRubberBand opt;
        opt.initFrom(this);
        opt.shape = QRubberBand::Rectangle;
        opt.opaque = false;
        opt.rect = d->selectRect;
        painter.save();
        style()->drawControl(QStyle::CE_RubberBand, &opt, &painter);
        painter.restore();
    }


// draw dragMove animation
    if (d->dodgeAnimationing) {
        for (auto animatingItem : d->dodgeItems) {
            auto localFile = animatingItem;
            auto index = model()->index(DUrl(localFile));
            if (index.isValid()) {
                option.rect = visualRect(index).marginsRemoved(d->cellMargins);
            }

            auto gridPos = d->dodgeTargetGrid->pos(m_screenNum, localFile);
            auto x = gridPos.x() * d->cellWidth + d->viewMargins.left();
            auto y = gridPos.y() * d->cellHeight + d->viewMargins.top();

            QRect end = QRect(x, y, d->cellWidth, d->cellHeight).marginsRemoved(d->cellMargins);

            auto current = dodgeDuration();
            auto nx = option.rect.x() + (end.x() - option.rect.x()) * current;
            auto ny = option.rect.y() + (end.y() - option.rect.y()) * current;
            option.rect.setX(static_cast<int>(nx));
            option.rect.setY(static_cast<int>(ny));
            option.rect.setSize(end.size());
            painter.save();
            itemDelegate()->paint(&painter, option, index);
            painter.restore();
        }
    }
}

//old
//void CanvasGridView::resizeEvent(QResizeEvent *event)
//{
//    updateCanvas();
//    // todo restore

//    return QAbstractItemView::resizeEvent(event);
//}

void CanvasGridView::focusInEvent(QFocusEvent *event)
{
    QAbstractItemView::focusInEvent(event);
    itemDelegate()->commitDataAndCloseActiveEditor();

    /// set menu actions filter
    DFileMenuManager::setActionWhitelist(QSet<MenuAction>());
    DFileMenuManager::setActionBlacklist(QSet<MenuAction>());
}

void CanvasGridView::focusOutEvent(QFocusEvent *event)
{
    QAbstractItemView::focusOutEvent(event);
    d->startDodge = false;
}

void CanvasGridView::contextMenuEvent(QContextMenuEvent *event)
{
    if (Q_UNLIKELY(DFMApplication::appObtuselySetting()->value("ApplicationAttribute", "DisableDesktopContextMenu", false).toBool())) {
        return;
    }

    const QModelIndex &index = indexAt(event->pos());
    bool indexIsSelected = selectionModel()->isSelected(index);
    bool isEmptyArea = d->fileViewHelper->isEmptyArea(event->pos()) && !indexIsSelected;
    Qt::ItemFlags flags;

    if (isEmptyArea) {
        flags = model()->flags(rootIndex());

        if (!flags.testFlag(Qt::ItemIsEnabled)) {
            return;
        }
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
        d->lastMenuPos = event->pos();
        showEmptyAreaMenu(flags);
    } else {
        if (!selectionModel()->isSelected(index)) {
            setCurrentIndex(index);
        }

        showNormalMenu(index, flags);
    }
}

bool CanvasGridView::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        updateCanvas();
    }

    return QAbstractItemView::event(event);
}

void CanvasGridView::rowsInserted(const QModelIndex &parent, int first, int last)
{
    QAbstractItemView::rowsInserted(parent, first, last);

    for (int index = first; index <= last; ++index) {
        const QModelIndex &child = parent.child(index, 0);
        DAbstractFileInfoPointer info = model()->fileInfo(child);
        if (info) {
            info->makeToActive();
        } else {
            qCritical() << "CanvasGridView::rowsInserted(): Create file info failed!!!!!!!!!!!!!" << index;
        }
    }

    update();
}

void CanvasGridView::keyboardSearch(const QString &search)
{
    if (search.isEmpty())
        return;

    d->fileViewHelper->keyboardSearch(search.toLocal8Bit().at(0));
}

void CanvasGridView::fakeDropEvent() noexcept
{
    d->dodgeDelayTimer.stop();
    d->startDodge = false;
    d->dragTargetGrid = QPoint(-1, -1);
}

QString CanvasGridView::canvansScreenName()
{
    return m_screenName;
}


QRect CanvasGridView::rectForIndex(const QModelIndex &index) const
{
    return visualRect(index);
}

DUrl CanvasGridView::currentUrl() const
{
    return model()->getUrlByIndex(rootIndex());
}

DFileSystemModel *CanvasGridView::model() const
{
    return qobject_cast<DFileSystemModel *>(QAbstractItemView::model());
}

DFileSelectionModel *CanvasGridView::selectionModel() const
{
    return static_cast<DFileSelectionModel *>(QAbstractItemView::selectionModel());
}

DesktopItemDelegate *CanvasGridView::itemDelegate() const
{
    return qobject_cast<DesktopItemDelegate *>(QAbstractItemView::itemDelegate());
}

void CanvasGridView::setItemDelegate(DesktopItemDelegate *delegate)
{
    QAbstractItemDelegate *dg = QAbstractItemView::itemDelegate();

    if (dg) {
        dg->deleteLater();
    }

    QAbstractItemView::setItemDelegate(delegate);
}

double CanvasGridView::dodgeDuration() const
{
    return d->dodgeDuration;
}

QMargins CanvasGridView::cellMargins() const
{
    return d->cellMargins;
}

QSize CanvasGridView::cellSize() const
{
    return QSize(d->cellWidth, d->cellHeight);
}

void CanvasGridView::openUrl(const DUrl &url)
{
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, url);
    if (!info || info->isVirtualEntry()) {
        // we do expand the virtual entry on single click, so no longer need to do that here.
        // toggleEntryExpandedState(url);
        return;
    }

    DFileService::instance()->openFile(this, url);
}

bool CanvasGridView::setCurrentUrl(const DUrl &url)
{
#if 0
    DUrl fileUrl = url;
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, fileUrl);
    if (!info) {
        qDebug() << "This scheme isn't support";
        return false;
    }

    const DUrl &checkUrl = currentUrl();

    if (checkUrl == fileUrl) {
        return false;
    }

    QModelIndex index = model()->setRootUrl(fileUrl);
    setRootIndex(index);

    if (!model()->canFetchMore(index)) {
        // TODO: updateContentLabel
        qDebug() << "TODO: updateContentLabel()";
    }

    if (focusWidget() && focusWidget()->window() == window() && fileUrl.isLocalFile()) {
        QDir::setCurrent(fileUrl.toLocalFile());
    }

    QAbstractItemView::setCurrentIndex(QModelIndex());

    model()->setFilters(model()->filters());

    if (d->filesystemWatcher) {
        d->filesystemWatcher->deleteLater();
    }

    QList<DAbstractFileInfoPointer> infoList = DFileService::instance()->getChildren(this, fileUrl,
                                                                                     QStringList(), model()->filters());
    if (autoMerge()) {
        GridManager::instance()->initWithoutProfile(infoList);
    }
    else if (GridManager::instance()->autoArrange()){
        GridManager::instance()->initArrage(infoList);
    }
    else {
        GridManager::instance()->initWithoutProfile(infoList);
    }

//    if(m_screenName != ScreenMrg->primaryScreen()->name())
//        return true;

    d->filesystemWatcher = model()->fileWatcher();

    connect(d->filesystemWatcher, &DAbstractFileWatcher::subfileCreated,
    this, [ = ](const DUrl & url) {
        Q_EMIT itemCreated(url);
        update();
    });

    connect(d->filesystemWatcher, &DAbstractFileWatcher::fileDeleted,
    this, [ = ](const DUrl & url) {
        Q_EMIT itemDeleted(url);
        update();
    });

    connect(d->filesystemWatcher, &DAbstractFileWatcher::fileMoved,
    this, [ = ](const DUrl & oriUrl, const DUrl & dstUrl) {

        bool findOldPos = false;
        QPoint oldPos = QPoint(-1, -1);

        if (GridManager::instance()->contains(m_screenNum, oriUrl.toString()) && !oriUrl.fileName().isEmpty()) {
            oldPos = GridManager::instance()->position(m_screenNum, oriUrl.toString());
            findOldPos = true;

#ifdef QT_DEBUG
            // TODO: switch to qCDebug()
            qDebug() << "find oldPos" << oldPos << oriUrl;
#endif // QT_DEBUG
        }

        bool findNewPos = false;
        if (dstUrl.parentUrl() == oriUrl.parentUrl() && !dstUrl.fileName().isEmpty()) {
            findNewPos = true;
        }

        findNewPos &= !GridManager::instance()->contains(m_screenNum, dstUrl.toString());

        if (!findNewPos) {
            Q_EMIT itemDeleted(oriUrl);
        } else {
            if (findOldPos) {
                GridManager::instance()->remove(m_screenNum, oriUrl.toString());
                GridManager::instance()->add(m_screenNum, oldPos, dstUrl.toString());
            } else {
                Q_EMIT itemCreated(dstUrl);
            }
        }

        update();
    });
    return true;
#endif
    if(GridManager::instance()->doneInit() && GridManager::instance()->autoMerge())
        GridManager::instance()->setCurrentVirtualExpandUrl(url);
    DUrl fileUrl = url;
    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, fileUrl);
    if (!info) {
        qDebug() << "This scheme isn't support";
        return false;
    }

    const DUrl &checkUrl = currentUrl();

    if (checkUrl == fileUrl) {
        return false;
    }

    QModelIndex index = model()->setRootUrl(fileUrl);
    setRootIndex(index);

    if (!model()->canFetchMore(index)) {
        // TODO: updateContentLabel
        qDebug() << "TODO: updateContentLabel()";
    }

    if (focusWidget() && focusWidget()->window() == window() && fileUrl.isLocalFile()) {
        QDir::setCurrent(fileUrl.toLocalFile());
    }

    QAbstractItemView::setCurrentIndex(QModelIndex());

    model()->setFilters(model()->filters());

    if (d->filesystemWatcher) {
        d->filesystemWatcher->deleteLater();
    }

    if(m_screenName == ScreenMrg->primaryScreen()->name() && GridManager::instance()->doneInit())
    {
        QList<DAbstractFileInfoPointer> infoList = DFileService::instance()->getChildren(this, fileUrl,
                                                                                         QStringList(), model()->filters());
        GridManager::instance()->initAutoMerge(infoList);
        GridManager::instance()->setCurrentAllItems(infoList);
    }
//    QList<DAbstractFileInfoPointer> infoList = DFileService::instance()->getChildren(this, fileUrl,
//                                                                                     QStringList(), model()->filters());
//    if (autoMerge()) {
//        GridManager::instance()->initWithoutProfile(infoList);
//    } else {
//        GridManager::instance()->initWithoutProfile(infoList);
//    }

//    if(m_screenName != ScreenMrg->primaryScreen()->name())
//        return true;

    d->filesystemWatcher = model()->fileWatcher();

    connect(d->filesystemWatcher, &DAbstractFileWatcher::subfileCreated,
    this, [ = ](const DUrl & url) {
        Q_EMIT itemCreated(url);
        update();
    });

    connect(d->filesystemWatcher, &DAbstractFileWatcher::fileDeleted,
    this, [ = ](const DUrl & url) {
        Q_EMIT itemDeleted(url);
        update();
    });

    connect(d->filesystemWatcher, &DAbstractFileWatcher::fileMoved,
    this, [ = ](const DUrl & oriUrl, const DUrl & dstUrl) {

        bool findOldPos = false;
        QPoint oldPos = QPoint(-1, -1);

        if (GridManager::instance()->contains(m_screenNum, oriUrl.toString()) && !oriUrl.fileName().isEmpty()) {
            oldPos = GridManager::instance()->position(m_screenNum, oriUrl.toString());
            findOldPos = true;

#ifdef QT_DEBUG
            // TODO: switch to qCDebug()
            qDebug() << "find oldPos" << oldPos << oriUrl;
#endif // QT_DEBUG
        }

        bool findNewPos = false;
        if (dstUrl.parentUrl() == oriUrl.parentUrl() && !dstUrl.fileName().isEmpty()) {
            findNewPos = true;
        }

        findNewPos &= !GridManager::instance()->contains(m_screenNum, dstUrl.toString());

        if (!findNewPos) {
            Q_EMIT itemDeleted(oriUrl);
        } else {
            if (findOldPos) {
                GridManager::instance()->remove(m_screenNum, oriUrl.toString());
                GridManager::instance()->add(m_screenNum, oldPos, dstUrl.toString());
            } else {
                //Q_EMIT itemCreated(dstUrl);
            }
        }

        update();
    });
    return true;
}

void CanvasGridView::initRootUrl()
{
    setAutoMerge(GridManager::instance()->autoMerge());
}

bool CanvasGridView::setRootUrl(const DUrl &url)
{
    if (url.isEmpty()) {
        return false;
    }

    if (url.scheme() == DFMMD_SCHEME) {
        for (unsigned int oneType = DMD_PICTURE; oneType <= DMD_OTHER; oneType++) {
            virtualEntryExpandState[static_cast<DMD_TYPES>(oneType)] = false;
        }

        QString frag = url.fragment();
        if (!frag.isEmpty()) {
            QStringList entryNameList = frag.split(',', QString::SkipEmptyParts);
            for (const QString &oneEntry : entryNameList) {
                virtualEntryExpandState[MergedDesktopController::entryTypeByName(oneEntry)] = true;
            }
        }
        //虚拟路径展开后立即保存
        if(GridManager::instance()->doneInit())
            GridManager::instance()->setCurrentVirtualExpandUrl(url);
    }

    itemDelegate()->hideAllIIndexWidget();

    clearSelection();

    return setCurrentUrl(url);
}

const DUrlList CanvasGridView::selectedUrls() const
{
    auto selects = selectionModel()->selectedIndexes();
    DUrlList urls;
    for (auto index : selects) {
        auto info = model()->fileInfo(index);
        if (info && !info->isVirtualEntry()) {
            urls << info->fileUrl();
        }
    }
    return urls;
}

void CanvasGridView::setScreenNum(int num)
{
    m_screenNum = num;
}

bool CanvasGridView::isSelected(const QModelIndex &index) const
{
    return static_cast<DFileSelectionModel *>(selectionModel())->isSelected(index);
}

void CanvasGridView::select(const QList<DUrl> &list)
{
    QModelIndex lastIndex;
    QItemSelection selection;

    clearSelection();

    for (auto &url : list) {
        auto index = model()->index(url);
        QItemSelectionRange selectionRange(index);
        if (!selection.contains(index)) {
            selection.push_back(selectionRange);
        }
        auto selectModel = static_cast<DFileSelectionModel *>(selectionModel());
        selectModel->select(selection, QItemSelectionModel::Select);
        lastIndex = index;
    }
    if (lastIndex.isValid()) {
        selectionModel()->setCurrentIndex(lastIndex, QItemSelectionModel::Select);
    }
}

int CanvasGridView::selectedIndexCount() const
{
    return static_cast<const DFileSelectionModel *>(selectionModel())->selectedCount();
}

bool CanvasGridView::edit(const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event)
{
    DUrl fileUrl = model()->getUrlByIndex(index);

    if (fileUrl.isEmpty() || selectedIndexCount() > 1 || (trigger == SelectedClicked && DFMGlobal::keyShiftIsPressed())) {
        return false;
    }

    if (trigger == SelectedClicked) {
        QStyleOptionViewItem option = viewOptions();

        option.rect = visualRect(index).marginsRemoved(d->cellMargins);

        const QRect &file_name_rect = itemDelegate()->fileNameRect(option, index);

        if (!file_name_rect.contains(static_cast<QMouseEvent *>(event)->pos())) {
            return false;
        }
    }

    if (QWidget *w = indexWidget(index)) {
        Qt::ItemFlags flags = model()->flags(index);
        if (((flags & Qt::ItemIsEditable) == 0) || ((flags & Qt::ItemIsEnabled) == 0)) {
            return false;
        }
        if (state() == QAbstractItemView::EditingState) {
            return false;
        }
        if (trigger == QAbstractItemView::AllEditTriggers) { // force editing
            d->fileViewHelper->triggerEdit(index);
            return true;
        }
        if ((trigger & editTriggers()) == QAbstractItemView::SelectedClicked
                && !selectionModel()->isSelected(index)) {
            return false;
        }

        if (trigger & editTriggers()) {
            w->setFocus();
            d->fileViewHelper->triggerEdit(index);
            return true;
        }
    }
    bool tmp = QAbstractItemView::edit(index, trigger, event);
    if (tmp) {
        d->fileViewHelper->triggerEdit(index);
    }

    return tmp;
}

void CanvasGridView::setDodgeDuration(double dodgeDuration)
{
    if (qFuzzyCompare(d->dodgeDuration, dodgeDuration)) {
        return;
    }

    d->dodgeDuration = dodgeDuration;
    emit dodgeDurationChanged(d->dodgeDuration);
}

void CanvasGridView::EnableUIDebug(bool enable)
{
    d->_debug_log = enable;
    d->_debug_show_grid = enable;
}

QString CanvasGridView::Size()
{
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream debug(&buffer);

    debug << GridManager::instance()->gridSize(m_screenNum);

    return QString::fromUtf8(buffer.buffer());
}

QString CanvasGridView::Dump()
{
    GridManager::instance()->dump();
    return "";
}

#include <QJsonObject>

QString CanvasGridView::DumpPos(qint32 x, qint32 y)
{
    QJsonObject debug;

    auto px = x * d->cellWidth + d->cellWidth / 2 + d->viewMargins.left();
    auto py = y * d->cellHeight + d->cellHeight / 2 + d->viewMargins.top();

    QModelIndex index = indexAt(QPoint(px, py));

    debug.insert("checkPoint:", QJsonValue::fromVariant(QList<QVariant>({px, py})));
    debug.insert("index", QJsonValue::fromVariant(QList<QVariant>({index.row(), index.column()})));
    debug.insert("url", model()->getUrlByIndex(index).toString());
    debug.insert("grid content", GridManager::instance()->itemId(m_screenNum, x, y));

    return QJsonDocument(debug).toJson();
}

void CanvasGridView::Refresh()
{
    model()->update();
}

static inline QRect fix_available_geometry()
{
    // virtualGeometry is same on all screen, so just get first one

    //if X11
    //auto virtualGeometry = qApp->screens().value(0)->virtualGeometry();
    //else

    QRect virtualGeometry;

    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") ||
            WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
//        virtualGeometry = Display::instance()->primaryScreen()->virtualGeometry();
        qDebug()<<"Display::instance()->primaryScreen()->virtualGeometry():" << Display::instance()->primaryScreen()->virtualGeometry();
        qDebug()<<"Display::instance()->primaryRect():" << Display::instance()->primaryRect();
        virtualGeometry = Display::instance()->primaryRect();

        int dockHideMode = DockIns::instance()->hideMode();
        if ( 1 == dockHideMode || 3 == dockHideMode) //隐藏与智能隐藏
            return virtualGeometry;

        //fix xcb的dock区域获取数据不对
        DockRect dockrect = DockIns::instance()->frontendWindowRect();
        qreal t_devicePixelRatio = Display::instance()->primaryScreen()->devicePixelRatio();
        if (!QHighDpiScaling::m_active) {
            t_devicePixelRatio = 1;
        }

        dockrect.width = dockrect.width / t_devicePixelRatio;
        dockrect.height = dockrect.height / t_devicePixelRatio;
        QRect primaryGeometry = Display::instance()->primaryRect();
        QRect ret = primaryGeometry;
        switch (DockIns::instance()->position()) {
        case 0: //上
            ret.setY(dockrect.height);
            break;
        case 1: //右
            ret.setWidth(ret.width() - dockrect.width);
            break;
        case 2: //下
            ret.setHeight(ret.height() - dockrect.height);
            break;
        case 3: //左
            ret.setX(dockrect.width);
            break;
        default:
            qCritical() << "dock postion error!";
            break;
        }
        qDebug() << "\n"
                 << "dump dock info begin ---------------------------" << "\n"
                 << "dockGeometry:" << dockrect << "\n"
                 << "virtualGeometry:" << virtualGeometry << "\n"
                 << "primarygeometry:" << primaryGeometry << "\n"
                 << "availableRect:" << ret << "\n"
                 << "dump dock info end ---------------------------" << "\n";
        return ret;
        //end
    }
    else {
        virtualGeometry = qApp->screens().value(0)->virtualGeometry();
    }


    xcb_ewmh_wm_strut_partial_t dock_xcb_ewmh_wm_strut_partial_t;
    memset(&dock_xcb_ewmh_wm_strut_partial_t, 0, sizeof(xcb_ewmh_wm_strut_partial_t));
    auto structParialInfoList = Xcb::XcbMisc::instance().find_dock_window();
    for (auto info : structParialInfoList) {
        if (info.rc.isValid()) {
            dock_xcb_ewmh_wm_strut_partial_t = Xcb::XcbMisc::instance().get_strut_partial(info.winId);
            break;
        }
    }

    QRegion virtualRegion = QRegion(virtualGeometry);

    //if X11
    //auto primaryGeometry = qApp->primaryScreen()->geometry();
    //else
    QRect primaryGeometry;

    if (XDG_SESSION_TYPE == QLatin1String("wayland") ||
            WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
//        primaryGeometry = Display::instance()->primaryScreen()->geometry();
        primaryGeometry = Display::instance()->primaryRect();
    } else {
        primaryGeometry = qApp->primaryScreen()->geometry();
    }

    QRect availableRect = primaryGeometry;
    // primary screen rect - dock rect;
    if (dock_xcb_ewmh_wm_strut_partial_t.top > 0) {
        availableRect.setY(dock_xcb_ewmh_wm_strut_partial_t.top);
    } else if (dock_xcb_ewmh_wm_strut_partial_t.right > 0) {
        availableRect.setWidth(2 * availableRect.width() - dock_xcb_ewmh_wm_strut_partial_t.right);
    } else if (dock_xcb_ewmh_wm_strut_partial_t.bottom > 0) {
        availableRect.setHeight(availableRect.height() - dock_xcb_ewmh_wm_strut_partial_t.bottom);
    } else if (dock_xcb_ewmh_wm_strut_partial_t.left > 0) {
        availableRect.setX(dock_xcb_ewmh_wm_strut_partial_t.left);
    }

    qDebug() << "\n"
             << "dump dock info begin ---------------------------" << "\n"
             << "virtualGeometry:" << virtualGeometry << "\n"
             << "primarygeometry:" << primaryGeometry << "\n"
             << "availableRect:" << availableRect << "\n"
             << "dump dock info end ---------------------------" << "\n";
    return availableRect;
}

static inline QRect getValidNewGeometry(const QRect &geometry, const QRect &oldGeometry)
{
    auto newGeometry = geometry;

//    if (qApp->screens().length() >= 2) {
    newGeometry = fix_available_geometry();
//    }
    bool geometryValid = (newGeometry.width() > 0) && (newGeometry.height() > 0);
    if (geometryValid) {
        return newGeometry;
    }

    newGeometry = Display::instance()->primaryRect();;
    geometryValid = (newGeometry.width() > 0) && (newGeometry.height() > 0);
    if (geometryValid) {
        return newGeometry;
    }

    qCritical() << "new valid geometry";
    return oldGeometry;
}

void CanvasGridView::initUI()
{
#ifdef QT_DEBUG
    EnableUIDebug(true);
#endif
    //d->dbusDock = DockIns::instance();//new DBusDock(this);

    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAutoFillBackground(false);
    setFrameShape(QFrame::NoFrame); // TODO: using QWidget instead of QFrame?
#if 0 //old
    setGeometry(Display::instance()->primaryRect());
    auto newGeometry =  getValidNewGeometry(Display::instance()->primaryRect(), this->geometry());
    d->canvasRect = newGeometry;
#endif
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    setDefaultDropAction(Qt::CopyAction);

    d->fileViewHelper = new CanvasViewHelper(this);
    d->fileViewHelper->setProperty("isCanvasViewHelper", true);

    setModel(new DFileSystemModel(d->fileViewHelper));
    model()->setEnabledSort(false);

    setSelectionModel(new DFileSelectionModel(model(), this));
    auto delegate = new DesktopItemDelegate(d->fileViewHelper);
    delegate->setEnabledTextShadow(true);
    delegate->setFocusTextBackgroundBorderColor(Qt::white);
    setItemDelegate(delegate);

    auto settings = Config::instance()->settings();
    settings->beginGroup(Config::groupGeneral);
    if (settings->contains(Config::keyIconLevel)) {
        auto iconSizeLevel = settings->value(Config::keyIconLevel).toInt();
        itemDelegate()->setIconSizeByIconSizeLevel(iconSizeLevel);
        qDebug() << "current icon size level" << itemDelegate()->iconSizeLevel();
    } else {
        itemDelegate()->setIconSizeByIconSizeLevel(1);
    }
    settings->endGroup();

    DFMSocketInterface::instance();

    d->waterMaskFrame = new WaterMaskFrame("/usr/share/deepin/dde-desktop-watermask.json", this);
    d->waterMaskFrame->lower();
    d->waterMaskFrame->updatePosition();
}

void CanvasGridView::updateGeometry(const QRect &geometry)
{
    auto newGeometry =  getValidNewGeometry(geometry, this->geometry());

    //if X11
    //setGeometry(qApp->primaryScreen()->geometry());
    //d->canvasRect = newGeometry;
    //qDebug() << "set newGeometry" << newGeometry << qApp->primaryScreen()->geometry();
    //else

    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") ||
            WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        setGeometry(Display::instance()->primaryRect());
        d->canvasRect = newGeometry;
        qDebug() << "set newGeometry" << newGeometry << Display::instance()->primaryScreen()->geometry();
    }

    else {
        setGeometry(qApp->primaryScreen()->geometry());
        d->canvasRect = newGeometry;
        qDebug() << "set newGeometry" << newGeometry << qApp->primaryScreen()->geometry();
    }


    d->waterMaskFrame->updatePosition();

    /*
     * For some reason, BackgroundHelper fails to resize the wallpaper when switching
     * a single active monitor among a multi-monitor setup, while CanvasGridView handles
     * this just fine. So we trigger an extra background resize manually here.
     */
    //BackgroundHelper::getDesktopInstance()->updateBackground((QLabel *)this->parent());

    updateCanvas();
    repaint();
}

void CanvasGridView::initConnection()
{
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
    [this](const QItemSelection & selected, const QItemSelection & deselected) {
        Q_UNUSED(selected);
        QModelIndex index = property("lastPressedIndex").toModelIndex();
        if (index.isValid() && deselected.contains(index)) {
            setProperty("lastPressedIndex", QModelIndex());
        }
    });
    connect(&d->dodgeDelayTimer, &QTimer::timeout,
    this, [ = ]() {
//        qDebug() << "start animation";
        d->dodgeAnimationing = true;
        auto animation = new QPropertyAnimation(this, "dodgeDuration");
        animation->setDuration(300);
        animation->setEasingCurve(QEasingCurve::InOutCubic);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);

        connect(animation, &QPropertyAnimation::valueChanged,
        this, [ = ]() {
            update();
        });

        connect(animation, &QPropertyAnimation::finished,
        this, [ = ]() {
            DUrlList selecteds = selectedUrls();
            DUrlList removed; //记录移除的
            for (auto relocateItem : selecteds) {
                auto localFile = relocateItem.toString();
                //处理跨屏，如果是跨屏的remove会失败，后面就无需再add
                if (GridManager::instance()->remove(m_screenNum, localFile))
                    removed << relocateItem;
            }

            // commit dodgeTargetGrid
            for (auto relocateItem : d->dodgeItems) {
                GridManager::instance()->remove(m_screenNum, relocateItem);
                auto pos = d->dodgeTargetGrid->pos(m_screenNum, relocateItem);
                GridManager::instance()->add(m_screenNum, pos, relocateItem);
            }

            //只有被remove的才add。用于处理跨屏拖动
            for (auto relocateItem : removed) {
                auto localFile = relocateItem.toString();
                GridManager::instance()->add(m_screenNum, d->dodgeTargetGrid->pos(m_screenNum, localFile), localFile);
            }
            d->dodgeAnimationing = false;
            delete d->dodgeTargetGrid;
            d->dodgeTargetGrid = nullptr;

            update();
        });
        animation->start();
        d->dodgeDelayTimer.stop();

        d->dodgeTargetGrid = GridManager::instance()->core();
        auto grid = d->dodgeTargetGrid;

        auto selURLs = selectedUrls();
        QStringList selLocalFiles;

        auto emptyBefore = 0;
        auto emptyAfter = 0;
        auto targetIndex = grid->toIndex(m_screenNum, d->dragTargetGrid);

        for (auto sel : selURLs) {
            auto localFile = sel.toString();
            selLocalFiles << localFile;
            auto pos = grid->pos(m_screenNum, localFile);
            auto index = grid->toIndex(m_screenNum, pos);
            grid->removeItem(m_screenNum, localFile);

            if (index < targetIndex) {
                ++emptyBefore;
            } else {
                ++emptyAfter;
            }
        }
        if (0 == emptyAfter) {
            ++targetIndex;
        }

        d->dodgeItems = grid->reloacle(m_screenNum, targetIndex, emptyBefore, emptyAfter);
        for (auto i = 0; i < selLocalFiles.length(); ++i) {
            grid->addItem(m_screenNum, targetIndex - emptyBefore + i, selLocalFiles.value(i));
        }
    });

//    d->syncTimer = new QTimer(this);
//    connect(d->syncTimer, &QTimer::timeout, this, [ = ]() {
//        update();
//        auto interval = d->syncTimer->interval() + 800;
//        if (interval > 10000) {
//            interval = 10000;
//        }
//        d->syncTimer->setInterval(interval);
//    });
//    d->syncTimer->start();
#if 0 //old
    auto connectScreenGeometryChanged = [this](QScreen * screen) {
        connect(screen, &QScreen::availableGeometryChanged,
        this, [ = ](const QRect & /*geometry*/) {
            QTimer::singleShot(400, this, [ = ]() {
                auto geometry = Display::instance()->primaryRect();
                qDebug() << "primaryScreen availableGeometryChanged changed to:" << geometry;
                qDebug() << "primaryScreen:" << qApp->primaryScreen() << qApp->primaryScreen()->geometry();
                updateGeometry(geometry);
            });
        });
    };

    connectScreenGeometryChanged(Display::instance()->primaryScreen());

    connect(Display::instance(), &Display::primaryScreenChanged,
    this, [ = ](QScreen * screen) {
        qDebug() << "primaryScreenChanged to:" << screen;

        //if X11
        //qDebug() << "currend primaryScreen" << qApp->primaryScreen()
        //       << qApp->primaryScreen()->availableGeometry();
        //else

        qDebug() << "currend primaryScreen" << screen
                 << screen->availableGeometry();

        screen = Display::instance()->primaryScreen();
        if (!screen) {
            return;
        }

        qDebug() << "screen availableGeometry:" << screen->availableGeometry();
        for (auto screen : qApp->screens()) {
            disconnect(screen, &QScreen::availableGeometryChanged, this, Q_NULLPTR);
        }
        connectScreenGeometryChanged(screen);

//        updateGeometry(Display::instance()->primaryRect());
        QTimer::singleShot(400, this, [ = ]() {
            auto geometry = Display::instance()->primaryRect();
            updateGeometry(geometry);
        });
    });

    connect(Display::instance(),&Display::primaryChanged,this, [ = ](){
        QTimer::singleShot(400, this, [ = ]() {
            auto geometry = Display::instance()->primaryRect();
            updateGeometry(geometry);
        });
    });
#endif
    connect(this->model(), &DFileSystemModel::newFileByInternal,
    this, [ = ](const DUrl & fileUrl) {
        if (GridManager::instance()->shouldArrange()){
            //交由viewmanager设置自动排序后的item的编辑框
            QTimer::singleShot(200,[fileUrl](){
                //需等待排列完成在打开editor
                emit GridManager::instance()->sigSyncOperation(GridManager::soRename,fileUrl.toString());
            });
            return ;
        }

        QString localFile = fileUrl.toString();
        int itemScreen;
        QPair<int, QPoint> orgPos;
        if (GridManager::instance()->find(localFile,orgPos)){
            itemScreen = orgPos.first;
        }
        else{
            qCritical() << "cannot find item" << localFile << "screen" <<m_screenNum;
            return ;
        }

        qDebug() << "newFileByInternal item" << localFile << "screen" << itemScreen
                 << "pos"<< orgPos.second << "to screen" << m_screenNum;

        QPair<int, QPoint> gridPos;
        gridPos.first = m_screenNum;
        gridPos.second = gridAt(d->lastMenuPos);
        if (d->lastMenuNewFilepath == localFile) {
            if (gridPos.second == GridManager::instance()->position(m_screenNum, localFile)) {
                //开启编辑框
                goto openEditor;
            }
        }

        gridPos = GridManager::instance()->forwardFindEmpty(m_screenNum, gridPos.second);

        //无需move
        if (gridPos == orgPos){
            goto openEditor;
        }

        //目标位置在当前屏才move
        if (gridPos.first == m_screenNum){
            //同屏
            if (m_screenNum == itemScreen){
                GridManager::instance()->move(m_screenNum, QStringList() << localFile,
                                              localFile, gridPos.second.x(), gridPos.second.y());
            }
            else{ //跨屏
                GridManager::instance()->move(itemScreen,m_screenNum, QStringList() << localFile,
                                              localFile, gridPos.second.x(), gridPos.second.y());
            }
        }
        //else{} //目标位置无法在m_screenNum放下，就不处理

openEditor:
        emit GridManager::instance()->sigSyncOperation(GridManager::soRename,fileUrl.toString());
        return ;
    });

    connect(this, &CanvasGridView::itemCreated, [ = ](const DUrl & url) {
        qDebug() << "CanvasGridView::itemCreated" << url << m_screenNum;
        d->lastMenuNewFilepath = url.toString();
        bool ret = GridManager::instance()->add(m_screenNum, d->lastMenuNewFilepath);

        /***************************************************************/
        //创建或者粘贴时保持之前的状态
        if(autoMerge()){
            DMD_TYPES toggleType = MergedDesktopController::checkUrlArrangedType(url);
            bool isExpand = virtualEntryExpandState[toggleType];
            if(isExpand){
                DUrl autoMergeUrl(DFMMD_ROOT MERGEDDESKTOP_FOLDER);
                autoMergeUrl.setFragment(QString());
                this->setRootUrl(autoMergeUrl);
                autoMergeUrl.setFragment(MergedDesktopController::entryNameByEnum(toggleType));
                this->setRootUrl(autoMergeUrl);
            }
            else {
                model()->refresh();
            }
        }
        else if (GridManager::instance()->autoArrange()){ //重新排列
            this->delayArrage();
        }

        /***************************************************************/
    });

    connect(this, &CanvasGridView::itemDeleted, [ = ](const DUrl & url) {

        if (!GridManager::instance()->remove(m_screenNum, url.toString()))
            return;

        auto index = model()->index(url);
        if (d->currentCursorIndex == index) {
            d->currentCursorIndex  = QModelIndex();
            selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Clear);
            setCurrentIndex(QModelIndex());
        }

        //重新排列
        if (GridManager::instance()->autoArrange()){
            this->delayArrage();
            return;
        }

        //自动整理
//        if (GridManager::instance()->autoMerge()) {
//            GridManager::instance()->reArrange();
//        }
    });

    connect(this->model(), &DFileSystemModel::requestSelectFiles,
            d->fileViewHelper, &CanvasViewHelper::onRequestSelectFiles);

    connect(this->model(), &QAbstractItemModel::dataChanged,
    this, [ = ](const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> &roles) {
        qDebug() << "dataChanged" << roles;
        if (d->resortCount > 0) {
            qDebug() << "dataChanged" << topLeft << bottomRight << roles;
            qDebug() << "resort desktop icons";
            model()->setEnabledSort(false);
            d->resortCount--;
            QStringList list;
            for (int i = 0; i < model()->rowCount(); ++i) {
                auto index = model()->index(i, 0);
                auto localFile = model()->getUrlByIndex(index).toString();
                list << localFile;
            }

            //自动排列
            if (GridManager::instance()->autoArrange()){
                GridManager::instance()->initArrage(list);
                return ;
            }

            GridManager::instance()->clear();
            //自动整理 todo
            //自定义
            for (auto lf : list) {
                GridManager::instance()->add(m_screenNum, lf);
            }
        }
    });

    connect(this, &CanvasGridView::doubleClicked,
    this, [this](const QModelIndex & index) {
        DUrl url = model()->getUrlByIndex(index);
        openUrl(url);
    }, Qt::QueuedConnection);


    connect(this, &CanvasGridView::autoAlignToggled,[this](){
        Presenter::instance()->onAutoAlignToggled();
        if (GridManager::instance()->autoArrange()){
            this->delayArrage();
        }
    });
    connect(this, &CanvasGridView::autoMergeToggled,
            Presenter::instance(), &Presenter::onAutoMergeToggled);
    connect(this, &CanvasGridView::sortRoleChanged,
            Presenter::instance(), &Presenter::onSortRoleChanged);
    connect(this, &CanvasGridView::changeIconLevel,
            Presenter::instance(), &Presenter::OnIconLevelChanged);

    connect(this, &CanvasGridView::changeIconLevel,
            [](int level){
        emit GridManager::instance()->sigSyncOperation(GridManager::soIconSize,level);
    });

    connect(this, &CanvasGridView::sortRoleChanged,
            [](int role, Qt::SortOrder order){
            QPoint sort(role,order);
            emit GridManager::instance()->sigSyncOperation(GridManager::soSort,sort);
    });

#if 0 //old
    connect(d->dbusDock, &DBusDock::HideModeChanged,
    this, [ = ]() {
          updateGeometry(Display::instance()->primaryRect());
    });
    connect(d->dbusDock, &DBusDock::PositionChanged,
    this, [ = ]() {
         updateGeometry(Display::instance()->primaryRect());
    });

    connect(d->dbusDock, &DBusDock::FrontendWindowRectChanged,
    this, [ = ]() {
         updateGeometry(Display::instance()->primaryRect());
    });

    connect(d->dbusDock, &DBusDock::IconSizeChanged,
    this, [ = ]() {
        updateGeometry(Display::instance()->primaryRect());
        this->updateCanvas();
    });
#endif

    connect(DFMApplication::instance(), &DFMApplication::showedHiddenFilesChanged, [ = ](bool isShowedHiddenFile) {
        QDir::Filters filters;
        if (isShowedHiddenFile) {
            filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
        } else {
            filters = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System;
        }
        model()->setFilters(filters);
        qDebug() << "current filters" << filters;
    });

    connect(DFMApplication::instance(), &DFMApplication::previewAttributeChanged, this->model(), &DFileSystemModel::update);
}

void CanvasGridView::updateCanvas()
{
#if 0 //old
    //if X11
    //auto outRect = qApp->primaryScreen()->geometry();
    //else
    QRect outRect;

    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") ||
            WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        outRect = Display::instance()->primaryRect();
    } else {
        outRect = qApp->primaryScreen()->geometry();
    }


    auto inRect = d->canvasRect;

    itemDelegate()->updateItemSizeHint();
    auto itemSize = itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex());

    QMargins geometryMargins;
    geometryMargins.setLeft(inRect.left() - outRect.left());
    geometryMargins.setRight(outRect.right() - inRect.right());
    geometryMargins.setTop(inRect.top() - outRect.top());
    geometryMargins.setBottom(outRect.bottom() - inRect.bottom());

    if (3 == d->dbusDock->hideMode()) {
        auto margin = 80;
        auto iconSize = d->dbusDock->iconSize();
        if (iconSize <= 30) {
            margin = 50;
        } else if (iconSize <= 36) {
            margin = 60;
        }

        QMargins dockMargin = QMargins(0, 0, 0, 0);
        auto position = d->dbusDock->position();
        switch (position) {
        case 0:
            dockMargin.setTop(margin);
            break;
        case 1:
            dockMargin.setRight(margin);
            break;
        case 2:
            dockMargin.setBottom(margin);
            break;
        case 3:
            dockMargin.setLeft(margin);
            break;
        }
        d->canvasRect = outRect.marginsRemoved(dockMargin);
        geometryMargins = dockMargin;
    } else {
        d->canvasRect = inRect;
    }
    d->updateCanvasSize(outRect.size(), d->canvasRect.size(), geometryMargins, itemSize);
    GridManager::instance()->updateGridSize(d->colCount, d->rowCount);

    updateEditorGeometries();

    auto expandedWidget = reinterpret_cast<QWidget *>(itemDelegate()->expandedIndexWidget());
    if (expandedWidget) {
        int offset = -1 * ((d->cellWidth - itemSize.width()) % 2);
        QMargins margins(offset, d->cellMargins.top(), 0, 0);
        expandedWidget ->setContentsMargins(margins);
    }
#else //todo计算margin
    itemDelegate()->updateItemSizeHint();
    auto itemSize = itemDelegate()->sizeHint(QStyleOptionViewItem(), QModelIndex());
    QMargins geometryMargins = QMargins(0, 0, 0, 0);
    d->updateCanvasSize(this->geometry().size(), this->geometry().size(), geometryMargins, itemSize);

    GridManager::instance()->updateGridSize(m_screenNum, d->colCount, d->rowCount);

    updateEditorGeometries();

    auto expandedWidget = reinterpret_cast<QWidget *>(itemDelegate()->expandedIndexWidget());
    if (expandedWidget) {
        int offset = -1 * ((d->cellWidth - itemSize.width()) % 2);
        QMargins margins(offset, d->cellMargins.top(), 0, 0);
        expandedWidget ->setContentsMargins(margins);
    }
#endif
    update();
}

void CanvasGridView::setIconByLevel(int level)
{
    if (itemDelegate()->iconSizeLevel() == level) {
        return;
    }
    itemDelegate()->setIconSizeByIconSizeLevel(level);
    emit this->changeIconLevel(itemDelegate()->iconSizeLevel());
    updateCanvas();
}

void CanvasGridView::increaseIcon()
{
    // TODO: 3 is 128*128, 0,1,2,3
    if (itemDelegate()->iconSizeLevel() >= 4) {
        return;
    }
    itemDelegate()->increaseIcon();
    emit this->changeIconLevel(itemDelegate()->iconSizeLevel());
    updateCanvas();
}

void CanvasGridView::decreaseIcon()
{
    itemDelegate()->decreaseIcon();
    emit this->changeIconLevel(itemDelegate()->iconSizeLevel());
    updateCanvas();
}

inline QPoint CanvasGridView::gridAt(const QPoint &pos) const
{
    auto row = (pos.x() - d->viewMargins.left()) / d->cellWidth;
    auto col = (pos.y() - d->viewMargins.top()) / d->cellHeight;
    return QPoint(row, col);
}

inline QRect CanvasGridView::gridRectAt(const QPoint &pos) const
{
    auto row = (pos.x() - d->viewMargins.left()) / d->cellWidth;
    auto col = (pos.y() - d->viewMargins.top()) / d->cellHeight;

    auto x = row * d->cellWidth + d->viewMargins.left();
    auto y = col * d->cellHeight + d->viewMargins.top();
    return QRect(x, y, d->cellWidth, d->cellHeight).marginsRemoved(d->cellMargins);
}

inline QList<QRect> CanvasGridView::itemPaintGeomertys(const QModelIndex &index) const
{
    QStyleOptionViewItem option = viewOptions();
    option.rect = visualRect(index).marginsRemoved(d->cellMargins);
    return itemDelegate()->paintGeomertys(option, index);
}

inline QRect CanvasGridView::itemIconGeomerty(const QModelIndex &index) const
{
    QStyleOptionViewItem option = viewOptions();
    option.rect = visualRect(index).marginsRemoved(d->cellMargins);
    auto rects = itemDelegate()->paintGeomertys(option, index);
    if (rects.isEmpty()) {
        return option.rect;
    }

    return rects.value(0);
}


inline QModelIndex CanvasGridView::firstIndex()
{
    auto localFile = GridManager::instance()->firstItemId(m_screenNum);
//    if(localFile.isEmpty())
//        return QModelIndex();
    return model()->index(DUrl(localFile));
}

inline QModelIndex CanvasGridView::lastIndex()
{
    auto localFile = GridManager::instance()->lastItemId(m_screenNum);
//    if(localFile.isEmpty())
//        return QModelIndex();
    return model()->index(DUrl(localFile));
}

void CanvasGridView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command, bool byIconRect)
{
    Q_UNUSED(byIconRect)

    auto selectRect = rect.normalized();
    auto topLeftGridPos = gridAt(selectRect.topLeft());
    auto bottomRightGridPos = gridAt(selectRect.bottomRight());

    QItemSelection oldSelection;
    //  keep old selection if mouse press
    bool ctrlShiftPress = DFMGlobal::keyShiftIsPressed() || DFMGlobal::keyCtrlIsPressed();
    if (ctrlShiftPress) {
        oldSelection = selectionModel()->selection();
        if (0 == selectionModel()->selection().indexes().size() && DFMGlobal::keyShiftIsPressed())
            d->beginPos = topLeftGridPos;
    } else {
        d->beginPos = QPoint(-1, -1);
    }

    // select by  key board, so mouse not pressed
    if (!d->mousePressed && d->currentCursorIndex.isValid()) {
        QItemSelectionRange selectionRange(d->currentCursorIndex);
        if (!oldSelection.contains(d->currentCursorIndex)) {
            oldSelection.push_back(selectionRange);
        }
        QAbstractItemView::selectionModel()->select(oldSelection, command);
        return;
    }

    //get current Point
    QPoint currentPoint(-1, -1);
    if (d->mousePressed) {
        auto clickIndex = indexAt(d->lastPos);
        if (clickIndex.isValid()) {
            QPoint tempClickIndex = visualRect(clickIndex).center();
            QPoint tempLastPoint = visualRect(d->currentCursorIndex).center();
            if (!d->currentCursorIndex.isValid())
                tempLastPoint = tempClickIndex + QPoint(1, 1);
            selectRect = QRect(tempClickIndex, tempLastPoint);

            topLeftGridPos = gridAt(selectRect.topLeft());
            bottomRightGridPos = gridAt(selectRect.bottomRight());
            currentPoint = topLeftGridPos;
            if (QPoint(-1, -1) == currentPoint)
                return;
        } else {
            // TODO: what?
            if (!d->showSelectRect) {
                return;
            }
            oldSelection = d->beforeMoveSelection;
            topLeftGridPos = gridAt(d->selectRect.topLeft());
            bottomRightGridPos = gridAt(d->selectRect.bottomRight());

            QItemSelection rectSelection;
            QItemSelection toRemoveSelection;
            for (auto x = topLeftGridPos.x(); x <= bottomRightGridPos.x(); ++x) {
                for (auto y = topLeftGridPos.y(); y <= bottomRightGridPos.y(); ++y) {
                    auto localFile = GridManager::instance()->itemId(m_screenNum, x, y);
                    if (localFile.isEmpty()) {
                        continue;
                    }
                    auto index = model()->index(DUrl(localFile));
                    auto list = QList<QRect>() << itemPaintGeomertys(index);
                    for (const QRect &r : list) {
                        if (selectRect.intersects(r)) {
                            QItemSelectionRange selectionRange(index);
                            if (!rectSelection.contains(index)) {
                                rectSelection.push_back(selectionRange);
                            }
                            break;
                        }
                        if (byIconRect) {
                            break;
                        }
                    }
                }
            }
            if (command != QItemSelectionModel::Deselect) {
                // Remove dump select
                for (auto &sel : rectSelection) {
                    for (auto &index : sel.indexes())
                        if (!oldSelection.contains(index)) {
                            oldSelection += rectSelection;
                        }
                }
                for (auto toRemove : toRemoveSelection) {
                    oldSelection.removeAll(toRemove);
                }
                QAbstractItemView::selectionModel()->select(oldSelection, command);
            } else {
                QAbstractItemView::selectionModel()->select(rectSelection, command);
            }
            return;
        }
    }

    if (DFMGlobal::keyShiftIsPressed()) {
        QItemSelection rectSelection;
        QItemSelection toRemoveSelection;
        //just keyShift
        bool beginSmall = d->beginPos.x() < currentPoint.x() || d->beginPos.y() < currentPoint.y();
        if (beginSmall) {
            topLeftGridPos = d->beginPos;
            bottomRightGridPos = currentPoint;
        } else {
            topLeftGridPos = currentPoint;
            bottomRightGridPos = d->beginPos;
        }
        qDebug() << "topLeftGridPos"  << topLeftGridPos << "bottomRightGridPos " << bottomRightGridPos;
        for (int x = 0; x < d->colCount; ++x) {
            for (int y = topLeftGridPos.y(); y <= bottomRightGridPos.y(); ++y) {
                if (x < topLeftGridPos.x() && y == topLeftGridPos.y())
                    continue;
                if (x > bottomRightGridPos.x() && y == bottomRightGridPos.y())
                    break;
                auto localFile = GridManager::instance()->itemId(m_screenNum, x, y);
                if (localFile.isEmpty()) {
                    continue;
                }
                auto index = model()->index(DUrl(localFile));
                auto list = QList<QRect>() << itemPaintGeomertys(index);
                QItemSelectionRange selectionRange(index);
                if (!rectSelection.contains(index)) {
                    rectSelection.push_back(selectionRange);
                }
            }
        }
        QAbstractItemView::selectionModel()->clear();
        QAbstractItemView::selectionModel()->select(rectSelection, command);
    } else if (DFMGlobal::keyCtrlIsPressed()) {
        //just Ctrl
        auto localFile = GridManager::instance()->itemId(m_screenNum, topLeftGridPos.x(), topLeftGridPos.y());
        if (localFile.isEmpty()) {
            return;
        }
        auto index = model()->index(DUrl(localFile));
        QItemSelectionRange selectionRange(index);
        if (!oldSelection.contains(index)) {
            oldSelection.push_back(selectionRange);
        } else {
            oldSelection.removeOne(selectionRange);
        }
        QAbstractItemView::selectionModel()->select(oldSelection, command);
        d->beginPos = topLeftGridPos;
    } else {
        //just click or mouseleft select
        auto localFile = GridManager::instance()->itemId(m_screenNum, topLeftGridPos.x(), topLeftGridPos.y());
        if (localFile.isEmpty()) {
            return;
        }
        auto index = model()->index(DUrl(localFile));
        QItemSelectionRange selectionRange(index);
        QItemSelection rectSelection;
        rectSelection.push_back(selectionRange);
        QAbstractItemView::selectionModel()->select(rectSelection, command);
        d->beginPos = topLeftGridPos;
    }
}

void CanvasGridView::handleContextMenuAction(int action)
{
    bool changeSort  = false;

    switch (action) {
    case DisplaySettings: {
        QStringList args;
        args << "--print-reply" << "--dest=com.deepin.dde.ControlCenter"
             << "/com/deepin/dde/ControlCenter" << "com.deepin.dde.ControlCenter.ShowModule"
             << "string:display";
        startProcessDetached("dbus-send", args);
        break;
    }
    case CornerSettings:
        Desktop::instance()->showZoneSettings();
        break;
    case WallpaperSettings:
        Desktop::instance()->ShowWallpaperChooser(m_screenName);
        break;
    case MenuAction::SelectAll:
        this->selectAll();
        break;
//    case FileManagerProperty: {
//        QStringList localFiles;
//        localFiles << currentUrl().toLocalFile();
//        qDebug() << localFiles;
//        DFMSocketInterface::instance()->showProperty(localFiles);
//        break;
//    }
    case AutoMerge:
        this->toggleAutoMerge(!autoMerge());
        emit autoMergeToggled();
        break;
    case AutoSort:
        emit autoAlignToggled();
        break;
    case IconSize0:
    case IconSize1:
    case IconSize2:
    case IconSize3:
    case IconSize4:
        setIconByLevel(action - IconSize0); //todo 多屏同步
        break;

    case MenuAction::Name:
    case MenuAction::Size:
    case MenuAction::Type:
    case MenuAction::LastModifiedDate:
        changeSort = true; //todo 多屏同步
        break;

    default:
        qDebug() << action;
    }

    if (changeSort) {
        model()->setEnabledSort(true);
        d->resortCount++;
//        QMap<int, int> sortActions;
//        sortActions.insert(MenuAction::Name, DFileSystemModel::FileDisplayNameRole);
//        sortActions.insert(MenuAction::Size, DFileSystemModel::FileSizeRole);
//        sortActions.insert(MenuAction::Type, DFileSystemModel::FileMimeTypeRole);
//        sortActions.insert(MenuAction::LastModifiedDate, DFileSystemModel::FileLastModifiedRole);

        int sortRole = kSortActions.value(action);
        Qt::SortOrder sortOrder = model()->sortOrder();
        //若排序模式一样，则改变升降
        if (sortRole == model()->sortRole())
                sortOrder = sortOrder == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;

        model()->setSortRole(sortRole, sortOrder);
        model()->sort();
        Q_EMIT sortRoleChanged(sortRole, sortOrder);
    }
}

void CanvasGridView::showEmptyAreaMenu(const Qt::ItemFlags &/*indexFlags*/)
{
#if 0 //older code
    const QModelIndex &index = rootIndex();
    const DAbstractFileInfoPointer &info = model()->fileInfo(index);
    QVector<MenuAction> actions;
    if (!autoMerge()) {
        actions << MenuAction::NewFolder << MenuAction::NewDocument
                << MenuAction::SortBy;
        actions << MenuAction::Paste;
    }
    actions << MenuAction::SelectAll << MenuAction::OpenInTerminal
            << MenuAction::Property << MenuAction::Separator;

    if (actions.isEmpty()) {
        return;
    }

    const QMap<MenuAction, QVector<MenuAction> > &subActions = info->subMenuActionList();

    QSet<MenuAction> disableList = DFileMenuManager::getDisableActionList(model()->getUrlByIndex(index));

    if (model()->state() != DFileSystemModel::Idle) {
        disableList << MenuAction::SortBy;
    }

//    if (!indexFlags.testFlag(Qt::ItemIsEditable)) {
//        disableList << MenuAction::NewDocument << MenuAction::NewFolder << MenuAction::Paste;
//    }

    if (!model()->rowCount()) {
        disableList << MenuAction::SelectAll;
    }

    DFileMenu *menu = DFileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);
    if (!menu) {
        return;
    }

    auto *pasteAction = menu->actionAt(DFileMenuManager::getActionString(MenuAction::Paste));

    QMenu iconSizeMenu;

    for (int i = itemDelegate()->minimumIconSizeLevel(); i <= itemDelegate()->maximumIconSizeLevel(); ++i) {
        auto iconSize = new QAction(&iconSizeMenu);
        iconSize->setText(itemDelegate()->iconSizeLevelDescription(i));
        iconSize->setData(IconSize + i);
        iconSize->setCheckable(true);
        iconSize->setChecked(i == itemDelegate()->iconSizeLevel());
        iconSizeMenu.addAction(iconSize);
    }

    QAction iconSizeAction(menu);
    iconSizeAction.setText(tr("Icon size"));
    iconSizeAction.setData(IconSize);
    iconSizeAction.setMenu(&iconSizeMenu);
    menu->insertAction(pasteAction, &iconSizeAction);

    QAction menuAutoMerge(menu);
    menuAutoMerge.setText(tr("Auto merge"));
    menuAutoMerge.setData(AutoMerge);
    menuAutoMerge.setCheckable(true);
    menuAutoMerge.setChecked(autoMerge());

//#if !defined(DISABLE_AUTOMERGE) || defined(QT_DEBUG)
//    menu->insertAction(pasteAction, &menuAutoMerge);
//#endif // DISABLE_AUTOMERGE

    DGioSettings settings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
    if (settings.value("auto-merge").toBool()) {
        menu->insertAction(pasteAction, &menuAutoMerge);
    }

    QAction autoSort(menu);
    autoSort.setText(tr("Auto arrange"));
    autoSort.setData(AutoSort);
    autoSort.setCheckable(true);
    autoSort.setChecked(GridManager::instance()->autoArrange());
    if (!autoMerge()) {
        menu->insertAction(pasteAction, &autoSort);
    }

    auto *propertyAction = menu->actionAt(DFileMenuManager::getActionString(MenuAction::Property));
//    QAction property(menu);
//    property.setText(tr("Properties"));
//    property.setData(FileManagerProperty);
//    menu->insertAction(propertyAction, &property);

//    QAction *sortByAction = menu->actionAt(DFileMenuManager::getActionString(MenuAction::SortBy));
//    DFileMenu *sortBySubMenu = static_cast<DFileMenu *>(sortByAction ? sortByAction->menu() : Q_NULLPTR);
//    if (sortBySubMenu) {
//        QMap<int, MenuAction> sortActions;
//        sortActions.insert(DFileSystemModel::FileDisplayNameRole, MenuAction::Name);
//        sortActions.insert(DFileSystemModel::FileSizeRole, MenuAction::Size);
//        sortActions.insert(DFileSystemModel::FileMimeTypeRole, MenuAction::Type);
//        sortActions.insert(DFileSystemModel::FileLastModifiedRole, MenuAction::LastModifiedDate);

//        auto sortRole = model()->sortRole();
//        auto sortMenuAction = sortActions.value(sortRole);

//        QAction *sortRoleAction = sortBySubMenu->actionAt(DFileMenuManager::getActionString(sortMenuAction));

//        sortRoleAction->setChecked(d->autoSort);
//}

    QList<QAction *> pluginActions = DFileMenuManager::loadEmptyAreaPluginMenu(menu, model()->rootUrl(), true);

    if (pluginActions.count() > 0) {
        QAction *separator = new QAction(menu);
        separator->setSeparator(true);
        menu->insertAction(pluginActions.at(0), separator);
    }

    QAction display(menu);
    display.setText(tr("Display Settings"));
    display.setData(DisplaySettings);
    menu->addAction(&display);

    QAction corner(menu);
    // QGSettings(const QByteArray &schema_id, const QByteArray &path = QByteArray(), QObject *parent = NULL);
    //DGioSettings(const QString& schemaId, const QString& path, QObject* parent = nullptr);
    DGioSettings gsetting("com.deepin.dde.desktop", "/com/deepin/dde/desktop/");
    if (gsetting.keys().contains("enable-hotzone-settings") && gsetting.value("enable-hotzone-settings").toBool()) {
        corner.setText(tr("Corner Settings"));
        corner.setData(CornerSettings);
        menu->addAction(&corner);
    }

    QAction wallpaper(menu);
#ifdef DISABLE_SCREENSAVER
    wallpaper.setText(tr("Set Wallpaper"));
#else
    wallpaper.setText(tr("Wallpaper and Screensaver"));
#endif
    wallpaper.setData(WallpaperSettings);
    menu->addAction(&wallpaper);

    menu->removeAction(propertyAction);
    menu->setEventData(model()->rootUrl(), selectedUrls(), winId(), this);

    connect(menu, &DFileMenu::triggered, this, [ = ](QAction * action) {
        qDebug() << "trigger action" << action->data();
        if (!action->data().isValid()) {
            return;
        }
        handleContextMenuAction(action->data().toInt());
    });

    d->fileViewHelper->handleMenu(menu);

    if (DesktopInfo().waylandDectected()) {

        QPoint t_tmpPoint = QCursor::pos();
        if (t_tmpPoint.x() + menu->sizeHint().width() > width())
            t_tmpPoint.setX(t_tmpPoint.x() - menu->sizeHint().width());

        if (t_tmpPoint.y() + menu->sizeHint().height() > height())
            t_tmpPoint.setY(t_tmpPoint.y() - menu->sizeHint().height());
        menu->exec(t_tmpPoint);
        menu->deleteLater();
        return;
    }
#endif

    const QModelIndex &index = rootIndex();
    const DAbstractFileInfoPointer &info = model()->fileInfo(index);
    QVector<MenuAction> actions;
    actions << MenuAction::NewFolder << MenuAction::NewDocument;
    if(!autoMerge()){
        actions << MenuAction::SortBy;
    }
    actions << MenuAction::Paste
            << MenuAction::SelectAll << MenuAction::OpenInTerminal
            << MenuAction::Property << MenuAction::Separator;
    if (actions.isEmpty()) {
        return;
    }

    const QMap<MenuAction, QVector<MenuAction> > &subActions = info->subMenuActionList();

    QSet<MenuAction> disableList = DFileMenuManager::getDisableActionList(model()->getUrlByIndex(index));

    if (model()->state() != DFileSystemModel::Idle) {
        disableList << MenuAction::SortBy;
    }
    if (!model()->rowCount()) {
        disableList << MenuAction::SelectAll;
    }

    DFileMenu *menu = DFileMenuManager::genereteMenuByKeys(actions, disableList, true, subActions);
    if (!menu) {
        return;
    }

    auto *pasteAction = menu->actionAt(DFileMenuManager::getActionString(MenuAction::Paste));

    QMenu iconSizeMenu;

    for (int i = itemDelegate()->minimumIconSizeLevel(); i <= itemDelegate()->maximumIconSizeLevel(); ++i) {
        auto iconSize = new QAction(&iconSizeMenu);
        iconSize->setText(itemDelegate()->iconSizeLevelDescription(i));
        iconSize->setData(IconSize + i);
        iconSize->setCheckable(true);
        iconSize->setChecked(i == itemDelegate()->iconSizeLevel());
        iconSizeMenu.addAction(iconSize);
    }
    //图标大小排列
    QAction iconSizeAction(menu);
    iconSizeAction.setText(tr("Icon size"));
    iconSizeAction.setData(IconSize);
    iconSizeAction.setMenu(&iconSizeMenu);
    menu->insertAction(pasteAction, &iconSizeAction);
    //自动整理
    QAction menuAutoMerge(menu);
    menuAutoMerge.setText(tr("Auto merge"));
    menuAutoMerge.setData(AutoMerge);
    menuAutoMerge.setCheckable(true);
    menuAutoMerge.setChecked(autoMerge());
    DGioSettings settings("com.deepin.dde.filemanager.desktop", "/com/deepin/dde/filemanager/desktop/");
    if (settings.value("auto-merge").toBool()) {
        menu->insertAction(pasteAction, &menuAutoMerge);
    }
    //自动排序
    QAction autoSort(menu);
    autoSort.setText(tr("Auto arrange"));
    autoSort.setData(AutoSort);
    autoSort.setCheckable(true);
    autoSort.setChecked(GridManager::instance()->autoArrange());
    if (!autoMerge()) {
        menu->insertAction(pasteAction, &autoSort);

        //勾选当前使用的排序
        QAction *sortAction = menu->actionAt(DFileMenuManager::getActionString(MenuAction::SortBy));
        if (sortAction != nullptr && sortAction->menu() != nullptr){
            QMenu *roleMenu = sortAction->menu();
            int datetype = kSortActions.key(model()->sortRole());
            qDebug() << model()->sortRole() << datetype << model()->sortOrder();
            for (QAction *action : roleMenu->actions()){
                if (action->data().toInt() == datetype && autoSort.isChecked()){
                    action->setCheckable(true);
                    action->setChecked(true);
                }
                else {
                    action->setCheckable(false);
                    action->setChecked(false);
                }
            }
        }
    }

    auto *propertyAction = menu->actionAt(DFileMenuManager::getActionString(MenuAction::Property));
    QList<QAction *> pluginActions = DFileMenuManager::loadEmptyAreaPluginMenu(menu, model()->rootUrl(), true);

    if (pluginActions.count() > 0) {
        QAction *separator = new QAction(menu);
        separator->setSeparator(true);
        menu->insertAction(pluginActions.at(0), separator);
    }

    //显示设置
    QAction display(menu);
    display.setText(tr("Display Settings"));
    display.setData(DisplaySettings);
    menu->addAction(&display);

    //热区设置，新版无热区设置需求
    //    QAction corner(menu);
    //    DGioSettings gsetting("com.deepin.dde.desktop", "/com/deepin/dde/desktop/");
    //    if (gsetting.keys().contains("enable-hotzone-settings") && gsetting.value("enable-hotzone-settings").toBool()) {
    //        corner.setText(tr("Corner Settings"));
    //        corner.setData(CornerSettings);
    //        menu->addAction(&corner);
    //    }

    //壁纸和屏保设置
    QAction wallpaper(menu);
#ifdef DISABLE_SCREENSAVER
    wallpaper.setText(tr("Set Wallpaper"));
#else
    wallpaper.setText(tr("Wallpaper and Screensaver"));
#endif
    wallpaper.setData(WallpaperSettings);
    menu->addAction(&wallpaper);

    menu->removeAction(propertyAction);
    menu->setEventData(model()->rootUrl(), selectedUrls(), winId(), this);

    connect(menu, &DFileMenu::triggered, this, [ = ](QAction * action) {
        qDebug() << "trigger action" << action->data();
        if (!action->data().isValid()) {
            return;
        }
        handleContextMenuAction(action->data().toInt());
    });

    d->fileViewHelper->handleMenu(menu);

    //todo 处理缩放
    if (DesktopInfo().waylandDectected()) {

        QPoint t_tmpPoint = QCursor::pos();
        if (t_tmpPoint.x() + menu->sizeHint().width() > width())
            t_tmpPoint.setX(t_tmpPoint.x() - menu->sizeHint().width());

        if (t_tmpPoint.y() + menu->sizeHint().height() > height())
            t_tmpPoint.setY(t_tmpPoint.y() - menu->sizeHint().height());
        menu->exec(t_tmpPoint);
        menu->deleteLater();
        return;
    }

    menu->exec();
    menu->deleteLater();
}

void CanvasGridView::showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags)
{
    if (!index.isValid()) {
        return;
    }

    bool showProperty = true;
    DUrlList list = selectedUrls();

    const DAbstractFileInfoPointer &info = model()->fileInfo(index);
    if (!info || info->isVirtualEntry()) {
        return;
    }

    QSet<MenuAction> disableList;
    QSet<MenuAction> unusedList;

    // blumia: when touching this part, do the same change in dfileview.cpp
    if (list.size() == 1) {
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

    if (list.length() == 1) {
        unusedList << MenuAction::OpenInNewWindow
                   << MenuAction::OpenInNewTab
                   << MenuAction::SendToDesktop
                   << MenuAction::AddToBookMark;
    } else {
        unusedList << MenuAction::SendToDesktop;
    }

    //totally use dde file manager libs for menu actions
    auto *menu = DFileMenuManager::createNormalMenu(info->fileUrl(), list, disableList, unusedList, winId(), true);

    QSet<MenuAction> ignoreActions;
    ignoreActions  << MenuAction::Open;
    menu->setIgnoreMenuActions(ignoreActions);

    if (!menu) {
        return;
    }

    auto *propertyAction = menu->actionAt(DFileMenuManager::getActionString(MenuAction::Property));
    if (propertyAction) {
        menu->removeAction(propertyAction);
    }

    QAction property(menu);
    if (showProperty) {
        property.setText(tr("Properties"));
        property.setData(FileManagerProperty);
        menu->addAction(&property);
    }

    menu->setEventData(model()->rootUrl(), selectedUrls(), winId(), this);

    connect(menu, &DFileMenu::triggered, this, [ = ](QAction * action) {
        if (!action->data().isValid()) {
            return;
        }

        switch (action->data().toInt()) {
        case MenuAction::Open: {
            // TODO: Workaround todo 修改为openfiles
            for (auto &url : list) {
                openUrl(url);
            }
        }
        break;
        case FileManagerProperty: {

            DFMGlobal::showPropertyDialog(this, this->selectedUrls());
            break;
        }
        case MenuAction::Rename: {
            if (list.size() == 1) {
                QAbstractItemView::edit(index); //###: select one file on desktop.

            } else { //###: select more than one files.

                QList<DUrl> selectedUrls{ this->selectedUrls() };
                DFMGlobal::showMultiFilesRenameDialog(selectedUrls);
            }
            break;
        }
        default:
            break;
        }
    });

    d->fileViewHelper->handleMenu(menu);

    //todo 处理缩放
    if (DesktopInfo().waylandDectected()) {

        QPoint t_tmpPoint = QCursor::pos();
        if (t_tmpPoint.x() + menu->sizeHint().width() > width())
            t_tmpPoint.setX(t_tmpPoint.x() - menu->sizeHint().width());

        if (t_tmpPoint.y() + menu->sizeHint().height() > height())
            t_tmpPoint.setY(t_tmpPoint.y() - menu->sizeHint().height());
        menu->exec(t_tmpPoint);
        menu->deleteLater();
        return;
    }

    menu->exec();
    menu->deleteLater();
}
