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

#include <dthememanager.h>
#include <danchors.h>
#include <DUtil>
#include <DApplication>

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

#include <QGSettings>

#include "../model/dfileselectionmodel.h"
#include "../presenter/gridmanager.h"
#include "../presenter/apppresenter.h"
#include "../presenter/display.h"
#include "../presenter/dfmsocketinterface.h"
#include "../desktop.h"
#include "../dbus/dbusdock.h"
#include "../config/config.h"
#include "backgroundhelper.h"

#include "interfaces/private/mergeddesktop_common_p.h"
#include "util/xcb/xcb.h"
#include "private/canvasviewprivate.h"
#include "canvasviewhelper.h"
#include "watermaskframe.h"
#include "desktopitemdelegate.h"
#include "dfmevent.h"

#include "app/define.h"
#include "controllers/mergeddesktopcontroller.h"

std::atomic<bool> CanvasGridView::m_flag{ false };
QMap<DMD_TYPES, bool> CanvasGridView::virtualEntryExpandState;

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

CanvasGridView::CanvasGridView(QWidget *parent)
    : QAbstractItemView(parent), d(new CanvasViewPrivate)
{
    D_THEME_INIT_WIDGET(CanvasGridView);

    initUI();
    initConnection();
}

CanvasGridView::~CanvasGridView()
{

}

QRect CanvasGridView::visualRect(const QModelIndex &index) const
{
    auto url = model()->getUrlByIndex(index);
    auto gridPos = GridManager::instance()->position(url.toString());

    auto x = gridPos.x() * d->cellWidth + d->viewMargins.left();
    auto y = gridPos.y() * d->cellHeight + d->viewMargins.top();
    return QRect(x, y, d->cellWidth, d->cellHeight);
}

QModelIndex CanvasGridView::indexAt(const QPoint &point) const
{
    auto gridPos = gridAt(point);
    auto localFile =  GridManager::instance()->itemId(gridPos.x(), gridPos.y());
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
    auto pos = GridManager::instance()->position(url.toString());
    auto newCoord = Coordinate(pos);

    switch (cursorAction) {
    case MoveLeft:
        while (pos.x() >= 0) {
            newCoord = newCoord.moveLeft();
            pos = newCoord.position();
            if (!GridManager::instance()->isEmpty(pos.x(), pos.y())) {
                break;
            }
        }
        break;
    case MoveRight:
        while (pos.x() < d->colCount) {
            newCoord = newCoord.moveRight();
            pos = newCoord.position();
            if (!GridManager::instance()->isEmpty(pos.x(), pos.y())) {
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
            if (!GridManager::instance()->isEmpty(pos.x(), pos.y())) {
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
            if (!GridManager::instance()->isEmpty(pos.x(), pos.y())) {
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
                if (!GridManager::instance()->isEmpty(pos.x(), pos.y())) {
                    auto localFile = GridManager::instance()->itemId(pos.x(), pos.y());
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
                if (!GridManager::instance()->isEmpty(pos.x(), pos.y())) {
                    auto localFile = GridManager::instance()->itemId(pos.x(), pos.y());
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

    auto localFile =  GridManager::instance()->itemId(pos.x(), pos.y());
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

    auto existItems = GridManager::instance()->itemIds();

    QStringList items;
    for (const DAbstractFileInfoPointer &info : infoList) {
        QString hideItem = info->fileUrl().toString();
        existItems.removeAll(hideItem);
        if (!GridManager::instance()->contains(hideItem)) {
            GridManager::instance()->add(hideItem);
        }
    }

    for (auto nonexistItem : existItems) {
        GridManager::instance()->remove(nonexistItem);
    }

    if (GridManager::instance()->shouldArrange()) {
        GridManager::instance()->reArrange();
    }
}

void CanvasGridView::setGeometry(const QRect &rect)
{
    if (parentWidget()) {
        QAbstractItemView::setGeometry(QRect(0, 0, rect.width(), rect.height()));
    } else {
        QAbstractItemView::setGeometry(rect);
    }
}

WId CanvasGridView::winId() const
{
    if (isTopLevel()) {
        return QAbstractItemView::winId();
    }
    else {
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
        this->setRootUrl(DUrl(DFMMD_ROOT MERGEDDESKTOP_FOLDER));
    } else {
        // sa
        QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
        DUrl desktopUrl = DUrl::fromLocalFile(desktopPath);

        if (!QDir(desktopPath).exists()) {
            QDir::home().mkpath(desktopPath);
        }

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
        d->selectFrame->setGeometry(d->selectRect);
    }

    if (d->showSelectRect) {
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
    d->selectFrame->setVisible(d->showSelectRect);
    d->lastPos = event->pos();

    bool isEmptyArea = !index.isValid();

    if (isEmptyArea) {
        if (!DFMGlobal::keyCtrlIsPressed() && !DFMGlobal::keyShiftIsPressed()) {
            itemDelegate()->hideNotEditingIndexWidget();
            QAbstractItemView::setCurrentIndex(QModelIndex());
            clearSelection();
        }
    }

    d->beforeMoveSelection = selectionModel()->selection();

    QAbstractItemView::mousePressEvent(event);

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
    d->selectFrame->setGeometry(d->selectRect);
    d->selectFrame->setVisible(d->showSelectRect);
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
            public: using  DApplication::handleHelpAction;
            };

            QString app_name = qApp->applicationName();
            qApp->setApplicationName("dde");
            reinterpret_cast<PublicApplication *>(DApplication::instance())->handleHelpAction();
            qApp->setApplicationName(app_name);
            break;
        }
        default: break;
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
            QStringList urls = GridManager::instance()->itemIds();
            DUrlList entryUrls;
            foreach (QString url, urls) {
                entryUrls << DUrl(url);
            }
            DFMGlobal::showFilePreviewDialog(selectUrls, entryUrls);
        }
        break;
        default: break;
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

    default: break;
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
    if (event->source()) {
        if (!autoMerge()) {
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
        auto localeFile = GridManager::instance()->itemId(d->dragTargetGrid);
        if (!localeFile.isEmpty() && !d->dodgeAnimationing) {
            d->dodgeDelayTimer.start();
        }

        d->fileViewHelper->preproccessDropEvent(event);
        event->accept();
    };

    if (hoverIndex.isValid()) {
        const DAbstractFileInfoPointer &fileInfo = model()->fileInfo(hoverIndex);

        if (fileInfo) {
            if (event->source() == this && !DFMGlobal::keyCtrlIsPressed()) {
                event->setDropAction(Qt::MoveAction);
            }

            if (fileInfo->canDrop() && fileInfo->supportedDropActions().testFlag(event->dropAction())) {
                event->accept();
                return;
            }
        }
    }

    if (!autoMerge()) {
        startDodgeAnimation();
    }
    update();
}

void CanvasGridView::dragLeaveEvent(QDragLeaveEvent *event)
{
    CanvasGridView::m_flag.store(false, std::memory_order_release);

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

    QStringList selectLocalFiles;
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

    if (event->source() == this && !DFMGlobal::keyCtrlIsPressed()) {
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

            if (event->source() == this && (!dropIndex.isValid() || dropOnSelf)) {
                if (autoMerge()) {
                    return;
                }
                auto point = event->pos();
                auto row = (point.x() - d->viewMargins.left()) / d->cellWidth;
                auto col = (point.y() - d->viewMargins.top()) / d->cellHeight;
                auto current = model()->fileInfo(d->currentCursorIndex)->fileUrl().toString();
                GridManager::instance()->move(selectLocalFiles, current, row, col);
                setState(NoState);
                itemDelegate()->hideNotEditingIndexWidget();
                DUtil::TimerSingleShot(20, [this]() {
                    repaint();
                    update();
                });
                return;
            }
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
            auto localFile = GridManager::instance()->itemId(x, y);
            if (!localFile.isEmpty()) {
                repaintLocalFiles << localFile;
            }
        }
    }

    auto overlayItems = GridManager::instance()->overlapItems();
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
                QPixmap pixmap = DStyledItemDelegate::getIconPixmap(expandMaskIcon, itemRect.size(), devicePixelRatioF());
                painter.drawPixmap(itemRect.topLeft(), pixmap);
            }
        }
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

            auto gridPos = d->dodgeTargetGrid->pos(localFile);
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

void CanvasGridView::resizeEvent(QResizeEvent * event)
{
    updateCanvas();
    // todo restore

    return QAbstractItemView::resizeEvent(event);
}

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
    } else {
        GridManager::instance()->initProfile(infoList);
    }

    d->filesystemWatcher = model()->fileWatcher();

    connect(d->filesystemWatcher, &DAbstractFileWatcher::subfileCreated,
    this, [ = ](const DUrl &url) {
        Q_EMIT itemCreated(url);
        update();
    });

    connect(d->filesystemWatcher, &DAbstractFileWatcher::fileDeleted,
    this, [ = ](const DUrl &url) {
        Q_EMIT itemDeleted(url);
        update();
    });

    connect(d->filesystemWatcher, &DAbstractFileWatcher::fileMoved,
            this, [ = ](const DUrl &oriUrl, const DUrl &dstUrl) {

        bool findOldPos = false;
        QPoint oldPos = QPoint(-1, -1);

        if (GridManager::instance()->contains(oriUrl.toString()) && !oriUrl.fileName().isEmpty()) {
            oldPos = GridManager::instance()->position(oriUrl.toString());
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

        findNewPos &= !GridManager::instance()->contains(dstUrl.toString());

        if (!findNewPos) {
            Q_EMIT itemDeleted(oriUrl);
        } else {
            if (findOldPos) {
                GridManager::instance()->remove(oriUrl.toString());
                GridManager::instance()->add(oldPos, dstUrl.toString());
            } else {
                Q_EMIT itemCreated(dstUrl);
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
            for (const QString & oneEntry : entryNameList) {
                virtualEntryExpandState[MergedDesktopController::entryTypeByName(oneEntry)] = true;
            }
        }
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

bool CanvasGridView::isSelected(const QModelIndex &index) const
{
    return static_cast<DFileSelectionModel *>(selectionModel())->isSelected(index);
}

void CanvasGridView::select(const QList<DUrl> &list)
{
    QModelIndex lastIndex;
    QItemSelection selection;
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

void CanvasGridView::EnableUIDebug()
{
    d->_debug_log = true;
    d->_debug_show_grid = true;
}

QString CanvasGridView::Size()
{
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream debug(&buffer);

    debug << GridManager::instance()->gridSize();

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
    debug.insert("grid content", GridManager::instance()->itemId(x, y));

    return QJsonDocument(debug).toJson();
}

// TODO: should fix by qt;
bool find_wm_strut_partial_rect_list(QList<QRect> rects)
{
    bool find = false;
    for (auto &rect : rects) {
        find |= rect.isValid();
    }

    return find;
}

static inline QList<QRect> wm_strut_partial_rect_list(xcb_ewmh_wm_strut_partial_t st, QRect virtualGeometry)
{
    QList<QRect> strutRectList;

    auto w = virtualGeometry.width();
    auto h = virtualGeometry.height();

    // left
    auto left = static_cast<int>(st.left);
    auto left_start = static_cast<int>(st.left_start_y);
    auto left_end = static_cast<int>(st.left_end_y);
    QRect leftStruct = QRect(0, left_start, left, left_end - left_start);
    leftStruct.setY(0);
    leftStruct.setHeight(h);
    strutRectList << leftStruct;

    // right
    auto right = static_cast<int>(st.right);
    auto right_start = static_cast<int>(st.right_start_y);
    auto right_end = static_cast<int>(st.right_end_y);
    QRect rightStruct = QRect(w - right, right_start, right, right_end - right_start);
    rightStruct.setY(0);
    rightStruct.setHeight(h);
    strutRectList << rightStruct;

    // top
    auto top = static_cast<int>(st.top);
    auto top_start = static_cast<int>(st.top_start_x);
    auto top_end = static_cast<int>(st.top_end_x);
    QRect topStruct = QRect(top_start, 0, top_end - top_start, top);
    topStruct.setX(0);
    topStruct.setWidth(w);
    strutRectList << topStruct;

    // bottom
    auto bottom = static_cast<int>(st.bottom);
    auto bottom_start = static_cast<int>(st.bottom_start_x);
    auto bottom_end = static_cast<int>(st.bottom_end_x);
    QRect bottomStruct = QRect(bottom_start, h - bottom, bottom_end - bottom_start, bottom);
    bottomStruct.setX(0);
    bottomStruct.setWidth(w);
    strutRectList << bottomStruct;

    return strutRectList;
}

static inline QRect fix_available_geometry()
{
    QList<QRect> strutParialRectList;

    auto screens = qApp->screens();

    // virtualGeometry is same on all screen, so just get first one
    auto virtualGeometry = qApp->screens().value(0)->virtualGeometry();
    // try 5 time
    for (int i = 0; i < 5; ++i) {
        auto structParialInfoList = Xcb::XcbMisc::instance().find_dock_window();
        for (auto info : structParialInfoList) {
            xcb_ewmh_wm_strut_partial_t st = Xcb::XcbMisc::instance().get_strut_partial(info.winId);
            qDebug() << "\n"
                     << "dump xcb_ewmh_wm_strut_partial_t begin ---------------------------" << "\n"
                     << "st.left :" << st.left << "\n"
                     << "st.right :" << st.right << "\n"
                     << "st.top :" << st.top << "\n"
                     << "st.left_start_y :" << st.left_start_y << "\n"
                     << "st.left_end_y :" << st.left_end_y << "\n"
                     << "st.right_start_y :" << st.right_start_y << "\n"
                     << "st.right_end_y :" << st.right_end_y << "\n"
                     << "st.top_start_x :" << st.top_start_x << "\n"
                     << "st.top_end_x :" << st.top_end_x << "\n"
                     << "st.bottom_start_x :" << st.bottom_start_x << "\n"
                     << "st.bottom_end_x :" << st.bottom_end_x << "\n"
                     << "dump xcb_ewmh_wm_strut_partial_t end ---------------------------" << "\n";
            strutParialRectList << wm_strut_partial_rect_list(st, virtualGeometry);
        }
        if (find_wm_strut_partial_rect_list(strutParialRectList)) {
            qDebug() << "get fix availableRect" << strutParialRectList << virtualGeometry;
            break;
        }
        QThread::sleep(1);
    }
    if (!find_wm_strut_partial_rect_list(strutParialRectList)) {
        qCritical() << "can not get struct partial post !!!!!!!!!!!!!!!!!";
    }

    QRegion virtualRegion = QRegion(virtualGeometry);
    QRegion strutParialRegion;
    for (auto rect : strutParialRectList) {
        strutParialRegion = strutParialRegion.united(rect);
    }

    QRegion availableRegion = virtualRegion.subtracted(strutParialRegion);

    auto primaryGeometry = qApp->primaryScreen()->geometry();
    QRect availableRect = availableRegion.intersected(primaryGeometry).rects().value(0);

    qDebug() << "\n"
             << "dump dock info begin ---------------------------" << "\n"
             << "strutParialRegion:" << strutParialRegion << "\n"
             << "virtualGeometry:" << virtualGeometry << "\n"
             << "primarygeometry:" << primaryGeometry << "\n"
             << "availableRect:" << availableRect << "\n"
             << "dump dock info end ---------------------------" << "\n";
    return availableRect;
}

static inline QRect getValidNewGeometry(const QRect &geometry, const QRect &oldGeometry)
{
    auto newGeometry = geometry;

    if (qApp->screens().length() >= 2) {
        newGeometry = fix_available_geometry();
    }
    bool geometryValid = (newGeometry.width() > 0) && (newGeometry.height() > 0);
    if (geometryValid) {
        return newGeometry;
    }

    auto primaryScreen = Display::instance()->primaryScreen();
    newGeometry = primaryScreen->geometry();;
    geometryValid = (newGeometry.width() > 0) && (newGeometry.height() > 0);
    if (geometryValid) {
        return newGeometry;
    }

    qCritical() << "new valid geometry";
    return oldGeometry;
}

void CanvasGridView::initUI()
{
    d->dbusDock = new DBusDock(this);

    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);

    auto primaryScreen = Display::instance()->primaryScreen();
    setGeometry(primaryScreen->geometry());
    auto newGeometry =  getValidNewGeometry(primaryScreen->availableGeometry(), this->geometry());
    d->canvasRect = newGeometry;

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);
    setDefaultDropAction(Qt::CopyAction);

    d->selectFrame = new QFrame(this);
    d->selectFrame->setVisible(false);
    d->selectFrame->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    d->selectFrame->setObjectName("SelectRect");
    d->selectFrame->setGeometry(QRect(-1, -1, 0, 0));

    d->fileViewHelper = new CanvasViewHelper(this);

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
    setGeometry(qApp->primaryScreen()->geometry());
    d->canvasRect = newGeometry;
    qDebug() << "set newGeometry" << newGeometry << qApp->primaryScreen()->geometry();
    d->waterMaskFrame->updatePosition();

    /*
     * For some reason, BackgroundHelper fails to resize the wallpaper when switching
     * a single active monitor among a multi-monitor setup, while CanvasGridView handles
     * this just fine. So we trigger an extra background resize manually here.
     */
    BackgroundHelper::getDesktopInstance()->updateBackground((QLabel*)this->parent());

    updateCanvas();
    repaint();
}

void CanvasGridView::initConnection()
{
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
            auto selecteds = selectedUrls();
            for (auto relocateItem : selecteds) {
                auto localFile = relocateItem.toString();
                GridManager::instance()->remove(localFile);
            }

            // commit dodgeTargetGrid
            for (auto relocateItem : d->dodgeItems) {
                GridManager::instance()->remove(relocateItem);
                auto pos = d->dodgeTargetGrid->pos(relocateItem);
                GridManager::instance()->add(pos, relocateItem);
            }

            for (auto relocateItem : selecteds) {
                auto localFile = relocateItem.toString();
                GridManager::instance()->add(d->dodgeTargetGrid->pos(localFile), localFile);
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
        auto targetIndex = grid->toIndex(d->dragTargetGrid);

        for (auto sel : selURLs) {
            auto localFile = sel.toString();
            selLocalFiles << localFile;
            auto pos = grid->pos(localFile);
            auto index = grid->toIndex(pos);
            grid->removeItem(localFile);

            if (index < targetIndex) {
                ++emptyBefore;
            } else {
                ++emptyAfter;
            }
        }
        if (0 == emptyAfter) {
            ++targetIndex;
        }

        d->dodgeItems = grid->reloacle(targetIndex, emptyBefore, emptyAfter);
        for (auto i = 0; i < selLocalFiles.length(); ++i) {
            grid->addItem(targetIndex - emptyBefore + i, selLocalFiles.value(i));
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

    auto connectScreenGeometryChanged = [this](QScreen * screen) {
        connect(screen, &QScreen::availableGeometryChanged,
        this, [ = ](const QRect & /*geometry*/) {
            QTimer::singleShot(400, this, [ = ]() {
                auto geometry = Display::instance()->primaryScreen()->availableGeometry();
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
        qDebug() << "currend primaryScreen" << qApp->primaryScreen()
                 << qApp->primaryScreen()->availableGeometry();

        if (!screen) {
            return;
        }

        qDebug() << "screen availableGeometry:" << screen->availableGeometry();
        for (auto screen : qApp->screens()) {
            disconnect(screen, &QScreen::availableGeometryChanged, this, Q_NULLPTR);
        }
        connectScreenGeometryChanged(screen);

        updateGeometry(screen->availableGeometry());
    });

    connect(this->model(), &DFileSystemModel::newFileByInternal,
    this, [ = ](const DUrl & fileUrl) {
        auto localFile = fileUrl.toString();
        auto gridPos = gridAt(d->lastMenuPos);

        if (d->lastMenuNewFilepath == localFile) {
            if (gridPos == GridManager::instance()->position(localFile)) {
                return;
            }
        }

        gridPos = GridManager::instance()->forwardFindEmpty(gridPos);
        GridManager::instance()->move(QStringList() << localFile, localFile, gridPos.x(), gridPos.y());
    });

    connect(this, &CanvasGridView::itemCreated, [ = ](const DUrl & url) {
        d->lastMenuNewFilepath = url.toString();
        GridManager::instance()->add(d->lastMenuNewFilepath);
    });

    connect(this, &CanvasGridView::itemDeleted, [ = ](const DUrl & url) {
        GridManager::instance()->remove(url.toString());

        auto index = model()->index(url);
        if (d->currentCursorIndex == index) {
            d->currentCursorIndex  = QModelIndex();
            selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::Clear);
            setCurrentIndex(QModelIndex());
        }

        if (GridManager::instance()->shouldArrange()) {
            GridManager::instance()->reArrange();
        }
    });

    connect(this->model(), &DFileSystemModel::requestSelectFiles,
            d->fileViewHelper, &CanvasViewHelper::onRequestSelectFiles);

    connect(this->model(), &QAbstractItemModel::dataChanged,
    this, [ = ](const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> &roles) {
        qDebug() << "dataChanged";
        if (d->resortCount > 0) {
            qDebug() << "dataChanged" << topLeft << bottomRight << roles;
            qDebug() << "resort desktop icons";
            model()->setEnabledSort(false);
            d->resortCount--;
            GridManager::instance()->clear();
            QStringList list;
            for (int i = 0; i < model()->rowCount(); ++i) {
                auto index = model()->index(i, 0);
                auto localFile = model()->getUrlByIndex(index).toString();
                list << localFile;
            }
            for (auto lf : list) {
                GridManager::instance()->add(lf);
            }
            GridManager::instance()->reArrange();
        }
    });

    connect(this, &CanvasGridView::doubleClicked,
    this, [this](const QModelIndex & index) {
        DUrl url = model()->getUrlByIndex(index);
        openUrl(url);
    }, Qt::QueuedConnection);


    connect(this, &CanvasGridView::autoAlignToggled,
            Presenter::instance(), &Presenter::onAutoAlignToggled);
    connect(this, &CanvasGridView::autoMergeToggled,
            Presenter::instance(), &Presenter::onAutoMergeToggled);
    connect(this, &CanvasGridView::sortRoleChanged,
            Presenter::instance(), &Presenter::onSortRoleChanged);
    connect(this, &CanvasGridView::changeIconLevel,
            Presenter::instance(), &Presenter::OnIconLevelChanged);

    connect(d->dbusDock, &DBusDock::HideModeChanged,
    this, [ = ]() {
        if (3 == d->dbusDock->hideMode() || 1 == d->dbusDock->hideMode()) {
            updateGeometry(Display::instance()->primaryScreen()->availableGeometry());
        }
    });
    connect(d->dbusDock, &DBusDock::PositionChanged,
    this, [ = ]() {
        if (3 == d->dbusDock->hideMode()) {
            updateGeometry(Display::instance()->primaryScreen()->availableGeometry());
        }
    });
    connect(d->dbusDock, &DBusDock::IconSizeChanged,
    this, [ = ]() {
        if (3 == d->dbusDock->hideMode()) {
            this->updateCanvas();
        }
    });

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
    auto outRect = qApp->primaryScreen()->geometry();
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
    auto localFile = GridManager::instance()->firstItemId();
    return model()->index(DUrl(localFile));
}

inline QModelIndex CanvasGridView::lastIndex()
{
    auto localFile = GridManager::instance()->lastItemId();
    return model()->index(DUrl(localFile));
}

void CanvasGridView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command, bool byIconRect)
{
    auto selectRect = rect.normalized();
    auto topLeftGridPos = gridAt(selectRect.topLeft());
    auto bottomRightGridPos = gridAt(selectRect.bottomRight());

    QItemSelection oldSelection;
    //  keep old selection if mouse press
    bool ctrlShiftPress = DFMGlobal::keyShiftIsPressed() || DFMGlobal::keyCtrlIsPressed();
    if (ctrlShiftPress) {
        oldSelection = selectionModel()->selection();
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

    if (d->mousePressed && ctrlShiftPress) {
        auto clickIndex = indexAt(d->lastPos);
        if (clickIndex.isValid()) {
            auto clickedPoint = visualRect(clickIndex).center();
            auto lastPoint = visualRect(d->currentCursorIndex).center();
            // when ctrl, only deal the mouse clicked item
            if (!d->currentCursorIndex.isValid() || DFMGlobal::keyCtrlIsPressed()) {
                lastPoint = clickedPoint + QPoint(1, 1);
            }
            selectRect = QRect(clickedPoint, lastPoint).normalized();
            topLeftGridPos = gridAt(selectRect.topLeft());
            bottomRightGridPos = gridAt(selectRect.bottomRight());
        } else {
            // TODO: what?
            if (!d->showSelectRect) {
                return;
            }
            oldSelection = d->beforeMoveSelection;
            topLeftGridPos = gridAt(d->selectRect.topLeft());
            bottomRightGridPos = gridAt(d->selectRect.bottomRight());
        }
    }

    QItemSelection rectSelection;
    QItemSelection toRemoveSelection;
    for (auto x = topLeftGridPos.x(); x <= bottomRightGridPos.x(); ++x) {
        for (auto y = topLeftGridPos.y(); y <= bottomRightGridPos.y(); ++y) {
            auto localFile = GridManager::instance()->itemId(x, y);
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
                    if (DFMGlobal::keyCtrlIsPressed() && oldSelection.contains(index)) {
                        toRemoveSelection.push_back(selectionRange);
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
        Desktop::instance()->showWallpaperSettings();
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
        setIconByLevel(action - IconSize0);
        break;

    case MenuAction::Name:
    case MenuAction::Size:
    case MenuAction::Type:
    case MenuAction::LastModifiedDate:
        changeSort = true;
        break;

    default:
        qDebug() << action;
    }

    if (changeSort) {
        model()->setEnabledSort(true);
        d->resortCount++;
        QMap<int, int> sortActions;
        sortActions.insert(MenuAction::Name, DFileSystemModel::FileDisplayNameRole);
        sortActions.insert(MenuAction::Size, DFileSystemModel::FileSizeRole);
        sortActions.insert(MenuAction::Type, DFileSystemModel::FileMimeTypeRole);
        sortActions.insert(MenuAction::LastModifiedDate, DFileSystemModel::FileLastModifiedRole);

        int             sortRole    = sortActions.value(action);
        Qt::SortOrder   sortOrder   = model()->sortOrder() == Qt::AscendingOrder ?
                                      Qt::DescendingOrder : Qt::AscendingOrder;

        model()->setSortRole(sortRole, sortOrder);
        model()->sort();
        Q_EMIT sortRoleChanged(sortRole, sortOrder);
    }
}

void CanvasGridView::showEmptyAreaMenu(const Qt::ItemFlags &/*indexFlags*/)
{
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
#if !defined(DISABLE_AUTOMERGE) || defined(QT_DEBUG)
    menu->insertAction(pasteAction, &menuAutoMerge);
#endif // DISABLE_AUTOMERGE

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
    /*QList<QAction *> extensionActions = */DFileMenuManager::loadEmptyAreaExtensionMenu(menu, model()->rootUrl(), true);

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
    QGSettings gsetting("com.deepin.dde.desktop", "/com/deepin/dde/desktop/");
    if (gsetting.keys().contains("enableHotzoneSettings") && gsetting.get("enable-hotzone-settings").toBool()) {
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
    auto *menu = DFileMenuManager::createNormalMenu(info->fileUrl(), list, disableList, unusedList, winId());

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
            // TODO: Workaround
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

    menu->exec();
    menu->deleteLater();
}
