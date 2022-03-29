/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "dragdropoper.h"
#include "model/canvasmodel.h"
#include "utils/keyutil.h"
#include "grid/canvasgrid.h"
#include "model/canvasselectionmodel.h"
#include "view/canvasview_p.h"
#include "canvasmanager.h"
#include "displayconfig.h"

#include <base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

#include <DFileDragClient>

#include <QDebug>
#include <QStandardPaths>
#include <QDragEnterEvent>
#include <QMimeData>

DGUI_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DDP_CANVAS_USE_NAMESPACE

DragDropOper::DragDropOper(CanvasView *parent)
    : QObject(parent), view(parent)
{
}

bool DragDropOper::enter(QDragEnterEvent *event)
{
    //todo using model
    // set target dir as desktop.
    m_target = view->model()->rootUrl();
    if (DFileDragClient::checkMimeData(event->mimeData())) {
        event->acceptProposedAction();
        // set target dir to source app.
        // after setting, app will copy it's items to target.
        // and this action is start before dropEvent instead in or after dropEvent.
        DFileDragClient::setTargetUrl(event->mimeData(), m_target);
        event->setDropAction(Qt::CopyAction);
        return false;
    }

    updatePrepareDodgeValue(event);

    if (checkXdndDirectSave(event))
        return false;

    preproccessDropEvent(event, event->mimeData()->urls(), view->model()->rootUrl());
    return true;
}

void DragDropOper::leave(QDragLeaveEvent *event)
{
    m_target.clear();
    stopDelayDodge();
    updatePrepareDodgeValue(event);
}

bool DragDropOper::move(QDragMoveEvent *event)
{
    stopDelayDodge();
    auto pos = event->pos();
    auto hoverIndex = view->indexAt(pos);
    QUrl curUrl = hoverIndex.isValid() ? view->model()->url(hoverIndex) : view->model()->rootUrl();
    if (hoverIndex.isValid()) {
        if (auto fileInfo = view->model()->fileInfo(hoverIndex)) {
            bool canDrop = !fileInfo->canDrop() || (fileInfo->isDir() && !fileInfo->isWritable()) || !fileInfo->supportedDropActions().testFlag(event->dropAction());
            if (!canDrop) {
                handleMoveMimeData(event, curUrl);

                // append compress
                //                if (fileInfo->canDrop() && fileInfo->canDragCompress()) {
                //                    // Set when dragging and dropping gvfs files, do not support additional compression
                //                    auto urls = event->mimeData()->urls();
                //                    if (!urls.isEmpty()) {
                //                        event->setDropAction(Qt::CopyAction);
                //                        auto itemInfo = DFMBASE_NAMESPACE::InfoFactory::create<DFMBASE_NAMESPACE::LocalFileInfo>(urls.first());
                //                        // todo isGvfsMountFile
                //                        if (itemInfo && itemInfo->isGvfsMountFile()) {
                //                            event->setDropAction(Qt::MoveAction);
                //                        }
                //                    }
                //                }
                return false;
            } else {
                // not support drop
                event->ignore();
            }
        }
    }

    tryDodge(event);

    // on blank space
    preproccessDropEvent(event, event->mimeData()->urls(), view->model()->rootUrl());
    if (!hoverIndex.isValid())
        handleMoveMimeData(event, curUrl);

    return false;
}

bool DragDropOper::drop(QDropEvent *event)
{
    stopDelayDodge();
    updatePrepareDodgeValue(event);

    // some special case.
    if (dropFilter(event))
        return false;

    // copy file by other app
    if (dropClientDownload(event))
        return false;

    // DirectSaveMode
    if (dropDirectSaveMode(event))
        return false;

    // move file on view grid.
    if (dropBetweenView(event))
        return false;

    if (dropMimeData(event))
        return false;

    event->ignore();
    return false;
}

void DragDropOper::preproccessDropEvent(QDropEvent *event, const QList<QUrl> &urls, const QUrl &targetFileUrl) const
{
    // todo sameuser.

    CanvasView *fromView = qobject_cast<CanvasView *>(event->source());
    if (fromView) {
        auto action = isCtrlPressed() ? Qt::CopyAction : Qt::MoveAction;
        event->setDropAction(action);
    } else if (urls.isEmpty()) {
        return;
    } else {
        auto itemInfo = FileCreator->createFileInfo(targetFileUrl);
        if (Q_UNLIKELY(!itemInfo))
            return;

        Qt::DropAction defaultAction = Qt::CopyAction;
        const QUrl from = urls.first();

        // using MoveAction if alt key is pressed.
        // CopyAction if ctrl key is pressed.
        if (isAltPressed()) {
            defaultAction = Qt::MoveAction;
        } else if (!isCtrlPressed()) {
            // todo(zy) //using DStorageInfo::inSameDevice
            if (targetFileUrl.scheme() == from.scheme()) {
                defaultAction = Qt::MoveAction;
            }
        }

        // is from or to trash or is to trash
        {
            bool isFromTrash = from.url().contains(".local/share/Trash/");
            bool isToTrash = false;   //to.isTrashFile(); // todo(zy)

            if (isFromTrash && isToTrash) {
                event->setDropAction(Qt::IgnoreAction);
                return;
            } else if (isFromTrash || isToTrash) {
                defaultAction = Qt::MoveAction;
            }
        }

        // todo is from vault

        // is from recent file
        // todo

        event->setDropAction(defaultAction);
    }
}

void DragDropOper::updateTarget(const QMimeData *data, const QUrl &url)
{
    if (url == m_target)
        return;
    m_target = url;
    DFileDragClient::setTargetUrl(data, m_target);
}

bool DragDropOper::checkXdndDirectSave(QDragEnterEvent *event) const
{
    if (event->mimeData()->hasFormat("XdndDirectSave0")) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return true;
    }

    return false;
}

void DragDropOper::selectItems(const QList<QUrl> &fileUrl) const
{
    //fileUrl is file:///xxxx
    QItemSelection selection;
    QMap<int, QPersistentModelIndex> viewCurrentIndex;
    for (const QUrl &url : fileUrl) {
        auto desktopUrl = url.toString();
        auto index = view->model()->index(desktopUrl);
        QItemSelectionRange selectionRange(index);
        if (!selection.contains(index)) {
            selection.push_back(selectionRange);
        }

        // the last selected index in every view.
        QPair<int, QPoint> pos;
        if (GridIns->point(desktopUrl, pos)) {
            viewCurrentIndex.insert(pos.first, index);
        }
    }

    if (!selection.isEmpty()) {
        view->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);

        // update new focus index.
        auto allView = CanvasIns->views();

        // clear focus
        for (const QSharedPointer<CanvasView> &v : allView) {
            v->d->operState().setCurrent(QModelIndex());
            v->d->operState().setContBegin(QModelIndex());
        }

        // set new focus.
        for (auto itor = viewCurrentIndex.begin(); itor != viewCurrentIndex.end(); ++itor) {
            for (const QSharedPointer<CanvasView> &v : allView) {
                if (v->screenNum() == itor.key()) {
                    v->d->operState().setCurrent(itor.value());
                    v->d->operState().setContBegin(itor.value());
                    break;
                }
            }
        }
    }
}

bool DragDropOper::dropFilter(QDropEvent *event)
{
    //Prevent the desktop's computer/recycle bin/home directory from being dragged and copied to other directories
    {
        QModelIndex index = view->indexAt(event->pos());
        if (index.isValid()) {
            QUrl targetItem = view->model()->url(index);
            auto itemInfo = FileCreator->createFileInfo(targetItem);
            if (itemInfo && (itemInfo->isDir() || itemInfo->url() == DesktopAppUrl::homeDesktopFileUrl())) {
                auto sourceUrls = event->mimeData()->urls();
                for (const QUrl &url : sourceUrls) {
                    if ((DesktopAppUrl::computerDesktopFileUrl() == url) || (DesktopAppUrl::trashDesktopFileUrl() == url) || (DesktopAppUrl::homeDesktopFileUrl() == url)) {
                        event->setDropAction(Qt::IgnoreAction);
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool DragDropOper::dropClientDownload(QDropEvent *event) const
{
    auto data = event->mimeData();
    if (DFileDragClient::checkMimeData(data)) {
        event->acceptProposedAction();
        qWarning() << "drop on" << m_target;

        QList<QUrl> urlList = data->urls();
        if (!urlList.isEmpty()) {
            // todo 排查哪些情况会进这里
            //Q_ASSERT(false);
            DFileDragClient *client = new DFileDragClient(data, const_cast<DragDropOper *>(this));
            qDebug() << "dragClientDownload" << client << data << urlList;
            connect(client, &DFileDragClient::stateChanged, this, [this, urlList](DFileDragState state) {
                if (state == Finished)
                    selectItems(urlList);
                qDebug() << "stateChanged" << state << urlList;
            });

            connect(client, &DFileDragClient::serverDestroyed, client, &DFileDragClient::deleteLater);
            connect(client, &DFileDragClient::destroyed, []() {
                qDebug() << "drag client deleted";
            });
        }

        return true;
    }

    return false;
}

bool DragDropOper::dropBetweenView(QDropEvent *event) const
{
    CanvasView *fromView = qobject_cast<CanvasView *>(event->source());
    if (!fromView || isCtrlPressed())
        return false;

    auto dropGridPos = view->d->gridAt(event->pos());
    auto dropRect = view->d->itemRect(dropGridPos);
    auto dropIndex = view->indexAt(dropRect.center());

    auto targetIndex = view->indexAt(event->pos());
    bool dropOnSelf = targetIndex.isValid() ? view->selectionModel()->selectedIndexes().contains(targetIndex) : false;

    // process this case in other drop function(e.g. move) if targetGridPos is used and it is not drop-needed.
    if (dropIndex.isValid() && !dropOnSelf) {
        if (!targetIndex.isValid()) {
            qInfo() << "drop on invaild target, skip. drop:" << dropGridPos.x() << dropGridPos.y();
            return true;
        }
        return false;
    }

    auto sourceUrls = event->mimeData()->urls();
    QSet<int> itemfrom;
    QPair<int, QPoint> tmpPos;
    QMap<QString, QPair<int, QPoint>> itemPos;
    for (const QUrl &url : sourceUrls) {
        auto item = url.toString();
        if (Q_LIKELY(GridIns->point(item, tmpPos))) {
            itemfrom.insert(tmpPos.first);
            itemPos.insert(item, tmpPos);
        }
    }
    auto resetFocus = [this](const QPoint &target) -> QModelIndex {
        auto droped = GridIns->item(view->screenNum(), target);
        auto newFocus = view->model()->index(droped);
        view->d->operState().setContBegin(newFocus);
        view->d->operState().setCurrent(newFocus);
        return newFocus;
    };

    if (itemfrom.size() > 1) {
        // items are from different view, using append.
        // remove all item.
        for (auto iter = itemPos.begin(); iter != itemPos.end(); ++iter)
            GridIns->remove(iter.value().first, iter.key());
        // then try drop item on target.
        GridIns->tryAppendAfter(itemPos.keys(), view->screenNum(), dropGridPos);

        // reset the focus for key move
        auto newFocus = resetFocus(dropGridPos);
        qDebug() << "reset focus "
                 << "to" << newFocus;
    } else if (itemfrom.size() == 1) {
        // items are from one view, using move.
        // normally, item should from the view that is event->source().
        auto focus = fromView->d->operState().current();
        auto focusItem = fromView->model()->url(focus).toString();
        if (!focusItem.isEmpty()) {
            if (GridIns->move(view->screenNum(), dropGridPos, focusItem, itemPos.keys())) {
                // reset the focus for key move
                auto newFocus = resetFocus(dropGridPos);
                qDebug() << "reset focus from" << focus << "to" << newFocus;
            }
        } else {
            qWarning() << "can not find fcous." << focus << fromView->screenNum();
        }
    } else {
        qWarning() << "can not find drop item.";
    }

    if (DispalyIns->autoAlign())
        GridIns->arrange();

    event->setDropAction(Qt::MoveAction);
    event->accept();
    CanvasIns->update();
    return true;
}

bool DragDropOper::dropDirectSaveMode(QDropEvent *event) const
{
    if (event->mimeData()->property("IsDirectSaveMode").toBool()) {
        event->setDropAction(Qt::CopyAction);
        const QModelIndex &index = view->indexAt(event->pos());
        auto fileInfo = view->model()->fileInfo(index.isValid() ? index : view->rootIndex());

        if (fileInfo && fileInfo->url().isLocalFile()) {
            if (fileInfo->isDir())
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->url());
            else
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->parentUrl());
        }

        event->accept();   // yeah! we've done with XDS so stop Qt from further event propagation.
        return true;
    }

    return false;
}

bool DragDropOper::dropMimeData(QDropEvent *event) const
{
    auto model = view->model();
    auto targetIndex = view->indexAt(event->pos());
    bool enableDrop = targetIndex.isValid() ? model->flags(targetIndex) & Qt::ItemIsDropEnabled : model->flags(model->rootIndex()) & Qt::ItemIsDropEnabled;
    if (model->supportedDropActions() & event->dropAction() && enableDrop) {
        preproccessDropEvent(event, event->mimeData()->urls(), targetIndex.isValid() ? model->url(targetIndex) : model->rootUrl());
        const Qt::DropAction action = event->dropAction();
        if (model->dropMimeData(event->mimeData(), action, targetIndex.row(), targetIndex.column(), targetIndex)) {
            if (action != event->dropAction()) {
                event->setDropAction(action);
                event->accept();
            } else {
                event->acceptProposedAction();
            }
            // todo(zy) FileSignalManager::requestSelectFile to selected the droped item.
        }
        return true;
    }
    return false;
}

void DragDropOper::handleMoveMimeData(QDropEvent *event, const QUrl &url)
{
    if (DFileDragClient::checkMimeData(event->mimeData())) {
        event->acceptProposedAction();
        // update target url if mouse focus is on file which can drop.
        updateTarget(event->mimeData(), url);
        qWarning() << "drop by app " << m_target;
    } else {
        event->accept();
    }
}

void DragDropOper::updatePrepareDodgeValue(QEvent *event)
{
    view->d->dodgeOper->updatePrepareDodgeValue(event);
}

void DragDropOper::tryDodge(QDragMoveEvent *event)
{
    view->d->dodgeOper->tryDodge(event);
}

void DragDropOper::stopDelayDodge()
{
    view->d->dodgeOper->stopDelayDodge();
}
