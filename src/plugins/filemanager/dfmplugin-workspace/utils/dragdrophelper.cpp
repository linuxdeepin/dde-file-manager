/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "dragdrophelper.h"
#include "views/fileview.h"
#include "models/fileviewmodel.h"
#include "models/filesortfilterproxymodel.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/windowutils.h"
#include "events/workspaceeventsequence.h"

#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/utils/fileutils.h"

#include <DFileDragClient>

#include <QMimeData>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

DragDropHelper::DragDropHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
}

bool DragDropHelper::dragEnter(QDragEnterEvent *event)
{
    const QMimeData *data = event->mimeData();
    if (handleDFileDrag(data, view->rootUrl())) {
        event->acceptProposedAction();
        return true;
    }

    currentDragUrls = data->urls();
    qInfo() << currentDragUrls;
    for (const QUrl &url : currentDragUrls) {
        auto info = InfoFactory::create<AbstractFileInfo>(url);
        if (!info || !info->canMoveOrCopy()) {
            event->ignore();
            return true;
        }
    }

    Qt::DropAction action = event->dropAction();
    if (WorkspaceEventSequence::instance()->doCheckDragTarget(currentDragUrls, view->rootUrl(), &action)) {
        switch (action) {
        case Qt::IgnoreAction: {
            event->ignore();
            return true;
        }
        default:
            event->setDropAction(action);
        }
    }

    handleDropEvent(event);
    qInfo() << event << event->dropAction();

    if (event->mimeData()->hasFormat("XdndDirectSave0")) {
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return true;
    }

    return false;
}

bool DragDropHelper::dragMove(QDragMoveEvent *event)
{
    AbstractFileInfoPointer hoverFileInfo = fileInfoAtPos(event->pos());

    if (hoverFileInfo) {
        handleDropEvent(event);

        if (!hoverFileInfo->canDrop()
            || !hoverFileInfo->supportedDropActions().testFlag(event->dropAction())
            || (hoverFileInfo->isDir() && !hoverFileInfo->isWritable())) {
            event->ignore();
            return true;
        }

        if (!handleDFileDrag(event->mimeData(), hoverFileInfo->url())) {
            event->accept();
        } else {
            event->ignore();
        }

        return true;
    }

    return false;
}

bool DragDropHelper::dragLeave(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    currentDragUrls.clear();
    return false;
}

bool DragDropHelper::drop(QDropEvent *event)
{
    handleDropEvent(event);

    if (event->mimeData()->property("IsDirectSaveMode").toBool()) {
        event->setDropAction(Qt::CopyAction);

        AbstractFileInfoPointer info = fileInfoAtPos(event->pos());
        if (info && info->url().isLocalFile()) {
            if (info->isDir()) {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", info->url());
            } else {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", info->parentUrl());
            }
        }

        event->accept();
    } else {
        QModelIndex hoverIndex = view->indexAt(event->pos());

        if (event->source() == view && (!hoverIndex.isValid() || view->isSelected(hoverIndex)))
            return true;

        bool isDropAtRootIndex = false;
        if (!hoverIndex.isValid()) {
            hoverIndex = view->model()->rootIndex();
            isDropAtRootIndex = true;
        }
        if (!hoverIndex.isValid())
            return true;

        bool supportDropAction = view->model()->supportedDropActions() & event->dropAction();
        bool dropEnabled = isDropAtRootIndex ? true : (view->model()->flags(hoverIndex) & Qt::ItemIsDropEnabled);
        if (supportDropAction && dropEnabled) {
            const Qt::DropAction action = view->dragDropMode() == QAbstractItemView::InternalMove
                    ? Qt::MoveAction
                    : event->dropAction();
            bool isDropped = view->model()->dropMimeData(event->mimeData(), action, hoverIndex.row(), hoverIndex.column(), hoverIndex);
            if (isDropped) {
                if (action != event->dropAction()) {
                    event->setDropAction(action);
                    event->accept();
                } else {
                    event->acceptProposedAction();
                }
            }

            view->selectionModel()->clear();
        }
    }

    return false;
}

bool DragDropHelper::handleDFileDrag(const QMimeData *data, const QUrl &url)
{
    if (DFileDragClient::checkMimeData(data)) {
        DFileDragClient::setTargetUrl(data, url);
        return true;
    }

    return false;
}

void DragDropHelper::handleDropEvent(QDropEvent *event)
{
    bool sameUser = isSameUser(event->mimeData());
    if (event->source() == view && !WindowUtils::keyCtrlIsPressed()) {
        event->setDropAction(checkAction(Qt::MoveAction, sameUser));
    } else {
        if (currentDragUrls.isEmpty())
            return;

        AbstractFileInfoPointer info = fileInfoAtPos(event->pos());
        if (!info)
            return;

        Qt::DropAction defaultAction = Qt::CopyAction;
        if (WindowUtils::keyAltIsPressed()) {
            defaultAction = Qt::MoveAction;
        } else if (!WindowUtils::keyCtrlIsPressed()) {
            if (FileUtils::isSameDevice(currentDragUrls.first(), info->url()))
                defaultAction = Qt::MoveAction;
        }

        if (event->possibleActions().testFlag(defaultAction))
            event->setDropAction(checkAction(defaultAction, sameUser));

        if (!info->supportedDropActions().testFlag(event->dropAction())) {
            QList<Qt::DropAction> actions;
            actions.reserve(3);
            actions << Qt::CopyAction << Qt::MoveAction << Qt::LinkAction;

            for (Qt::DropAction action : actions) {
                if (event->possibleActions().testFlag(action) && info->supportedDropActions().testFlag(action)) {
                    event->setDropAction(checkAction(action, sameUser));
                    break;
                }
            }
        }
    }
}

QSharedPointer<AbstractFileInfo> DragDropHelper::fileInfoAtPos(const QPoint &pos)
{
    const QModelIndex &index = view->indexAt(pos);
    if (index.isValid()) {
        const FileViewItem *item = view->model()->itemFromIndex(index);
        if (item)
            return view->model()->itemFromIndex(index)->fileInfo();
    } else {
        return view->model()->rootItem()->fileInfo();
    }
    return nullptr;
}

bool DragDropHelper::isSameUser(const QMimeData *data)
{
    if (data->hasFormat(DFMGLOBAL_NAMESPACE::kMimeDataUserIDKey)) {
        QString userID = data->data(DFMGLOBAL_NAMESPACE::kMimeDataUserIDKey);
        return userID == QString::number(SysInfoUtils::getUserId());
    }

    return false;
}

Qt::DropAction DragDropHelper::checkAction(Qt::DropAction srcAction, bool sameUser)
{
    if (!sameUser && srcAction == Qt::MoveAction)
        return Qt::IgnoreAction;

    return srcAction;
}
