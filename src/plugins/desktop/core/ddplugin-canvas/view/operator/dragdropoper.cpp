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
#include "utils/keyutil.h"
#include "grid/canvasgrid.h"
#include "model/canvasselectionmodel.h"
#include "view/canvasview_p.h"
#include "canvasmanager.h"
#include "displayconfig.h"
#include "utils/fileutil.h"

#include <base/schemefactory.h>
#include <base/standardpaths.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/sysinfoutils.h>

#include <DFileDragClient>

#include <QDebug>
#include <QStandardPaths>
#include <QDragEnterEvent>
#include <QMimeData>

DGUI_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

DragDropOper::DragDropOper(CanvasView *parent)
    : QObject(parent), view(parent)
{
}

bool DragDropOper::enter(QDragEnterEvent *event)
{
    // Filter the event that cannot be dragged
    if (checkProhibitPaths(event))
        return true;

    // set target dir as desktop.
    m_target = view->model()->rootUrl();
    if (DFileDragClient::checkMimeData(event->mimeData())) {
        event->acceptProposedAction();
        // set target dir to source app.
        // after setting, app will copy it's items to target.
        // and this action is start before dropEvent instead in or after dropEvent.
        DFileDragClient::setTargetUrl(event->mimeData(), m_target);
        event->setDropAction(Qt::CopyAction);
        return true;
    }

    updatePrepareDodgeValue(event);

    if (checkXdndDirectSave(event))
        return true;

    preproccessDropEvent(event, event->mimeData()->urls(), view->model()->rootUrl());
    return false;
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
    // extend
    if (hoverIndex.isValid() && view->d->hookIfs) {
        QUrl hoverUrl = view->model()->fileUrl(hoverIndex);
        Qt::DropAction dropAction = Qt::IgnoreAction;

        QVariantHash ext;
        ext.insert("hoverUrl", QVariant(hoverUrl));
        ext.insert("dropAction", qlonglong(&dropAction));

        if (view->d->hookIfs->dragMove(view->screenNum(), event->mimeData(), event->pos(), &ext)) {
            if (dropAction != Qt::IgnoreAction) {
                event->setDropAction(dropAction);
                event->accept();
                return true;
            }
        }
    }
    QUrl curUrl = hoverIndex.isValid() ? view->model()->fileUrl(hoverIndex) : view->model()->rootUrl();
    if (hoverIndex.isValid()) {
        if (auto fileInfo = view->model()->fileInfo(hoverIndex)) {
            bool canDrop = !fileInfo->canAttributes(AbstractFileInfo::FileCanType::kCanDrop) || (fileInfo->isAttributes(AbstractFileInfo::FileIsType::kIsDir) && !fileInfo->isAttributes(AbstractFileInfo::FileIsType::kIsWritable)) || !fileInfo->supportedAttributes(AbstractFileInfo::SupportType::kDrop).testFlag(event->dropAction());
            if (!canDrop) {
                handleMoveMimeData(event, curUrl);

                return true;
            } else {
                // not support drop
                event->ignore();
            }
        }
    }

    tryDodge(event);

    // hover
    preproccessDropEvent(event, event->mimeData()->urls(), curUrl);
    if (!hoverIndex.isValid())
        handleMoveMimeData(event, curUrl);

    return true;
}

bool DragDropOper::drop(QDropEvent *event)
{
    stopDelayDodge();
    updatePrepareDodgeValue(event);

    // extend
    if (view->d->hookIfs) {
        QVariantHash ext;
        ext.insert("QDropEvent", reinterpret_cast<qlonglong>(event));
        QUrl dropUrl;
        QModelIndex dropIndex = view->indexAt(event->pos());
        if (dropIndex.isValid()) {
            dropUrl = view->model()->fileUrl(dropIndex);
        }
        ext.insert("dropUrl", QVariant(dropUrl));

        if (view->d->hookIfs->dropData(view->screenNum(), event->mimeData(), event->pos(), &ext)) {
            qDebug() << "droped by extend";
            return true;
        }
    }

    // some special case.
    if (dropFilter(event))
        return true;

    // copy file by other app
    if (dropClientDownload(event))
        return true;

    // DirectSaveMode
    if (dropDirectSaveMode(event))
        return true;

    // move file on view grid.
    if (dropBetweenView(event))
        return true;

    if (dropMimeData(event))
        return true;

    event->ignore();
    return true;
}

void DragDropOper::preproccessDropEvent(QDropEvent *event, const QList<QUrl> &urls, const QUrl &targetFileUrl) const
{
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
            if (FileUtils::isSameDevice(targetFileUrl, from)) {
                defaultAction = Qt::MoveAction;
            }
        }

        // is from or to trash or is to trash
        {
            bool isFromTrash = FileUtils::isTrashFile(from);
            bool isToTrash = false;   // there is no  trash dir on desktop.

            if (isFromTrash && isToTrash) {
                event->setDropAction(Qt::IgnoreAction);
                return;
            } else if (isFromTrash || isToTrash) {
                defaultAction = Qt::MoveAction;
            }
        }

        const bool sameUser = isSameUser(event->mimeData());
        if (event->possibleActions().testFlag(defaultAction))
            event->setDropAction((defaultAction == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : defaultAction);

        // todo is from vault

        if (!itemInfo->supportedAttributes(AbstractFileInfo::SupportType::kDrop).testFlag(event->dropAction())) {
            QList<Qt::DropAction> actions;

            actions.reserve(3);
            actions << Qt::CopyAction << Qt::MoveAction << Qt::LinkAction;

            for (Qt::DropAction action : actions) {
                if (event->possibleActions().testFlag(action) && itemInfo->supportedAttributes(AbstractFileInfo::SupportType::kDrop).testFlag(action)) {
                    event->setDropAction((action == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : action);
                    break;
                }
            }
        }

        //        // is from recent file
        //        if (from.isRecentFile()) {
        //            defaultAction = isToTrash ? Qt::MoveAction : Qt::CopyAction;
        //            event->setDropAction(defaultAction);
        //        }

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

bool DragDropOper::checkProhibitPaths(QDragEnterEvent *event) const
{
    auto urlsForDragEvent = event->mimeData()->urls();

    // Filter the event that cannot be dragged
    if (urlsForDragEvent.isEmpty() || FileUtils::isContainProhibitPath(urlsForDragEvent)) {
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
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
            QUrl targetItem = view->model()->fileUrl(index);
            auto itemInfo = FileCreator->createFileInfo(targetItem);
            if (itemInfo && (itemInfo->isAttributes(AbstractFileInfo::FileIsType::kIsDir) || itemInfo->urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl) == DesktopAppUrl::homeDesktopFileUrl())) {
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
        auto focusItem = fromView->model()->fileUrl(focus).toString();
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

        if (fileInfo && fileInfo->urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl).isLocalFile()) {
            if (fileInfo->isAttributes(AbstractFileInfo::FileIsType::kIsDir))
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->urlInfo(AbstractFileInfo::FileUrlInfoType::kUrl));
            else
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->urlInfo(AbstractFileInfo::FileUrlInfoType::kParentUrl));
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
        preproccessDropEvent(event, event->mimeData()->urls(), targetIndex.isValid() ? model->fileUrl(targetIndex) : model->rootUrl());
        const Qt::DropAction action = event->dropAction();
        if (model->dropMimeData(event->mimeData(), action, targetIndex.row(), targetIndex.column(), targetIndex)) {
            if (action != event->dropAction()) {
                event->setDropAction(action);
                event->accept();
            } else {
                event->acceptProposedAction();
            }
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

bool DragDropOper::isSameUser(const QMimeData *data) const
{
    if (data->hasFormat(DFMGLOBAL_NAMESPACE::Mime::kMimeDataUserIDKey)) {
        QString userID = data->data(DFMGLOBAL_NAMESPACE::Mime::kMimeDataUserIDKey);
        return userID == QString::number(SysInfoUtils::getUserId());
    }

    return false;
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
