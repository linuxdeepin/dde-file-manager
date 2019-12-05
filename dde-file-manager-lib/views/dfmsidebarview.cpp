/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmsidebarview.h"
#include "dfmsidebaritem.h"
#include "dfileservices.h"
#include "app/define.h"

#include <QDebug>
#include <dstorageinfo.h>
#include <qmimedata.h>
#include <QtConcurrent>
#include <models/dfmsidebarmodel.h>

DFM_BEGIN_NAMESPACE

DFMSideBarView::DFMSideBarView(QWidget *parent)
    : DListView (parent)
{
    setVerticalScrollMode(ScrollPerPixel);
    setIconSize(QSize(16, 16));
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//    setMouseTracking(true);

    setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropOverwriteMode(false);
}

void DFMSideBarView::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);
#if QT_CONFIG(draganddrop)
    if (state() == DraggingState) {
        startDrag(model()->supportedDragActions());
        setState(NoState); // the startDrag will return when the dnd operation is done
        stopAutoScroll();
        QPoint pt = mapFromGlobal(QCursor::pos());
        QRect rc = geometry();
        if (!rc.contains(pt)) {
            emit requestRemoveItem(); //model()->removeRow(currentIndex().row());
        }
    }
#endif // QT_CONFIG(draganddrop)
}

void DFMSideBarView::dragEnterEvent(QDragEnterEvent *event)
{
    if (isAccepteDragEvent(event)) {
        return;
    }

    DListView::dragEnterEvent(event);

    if (event->source() != this) {
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
    }
}

void DFMSideBarView::dragMoveEvent(QDragMoveEvent *event)
{
    if (isAccepteDragEvent(event)) {
        return;
    }

    DListView::dragMoveEvent(event);

    if (event->source() != this) {
        event->ignore();
    }
}

void DFMSideBarView::dropEvent(QDropEvent *event)
{
    if (isAccepteDragEvent(event)) {

    }
    DFMSideBarItem *item = itemAt(event->pos());
    if (!item) {
        return DListView::dropEvent(event);
    }

    qDebug() << "source: " << event->mimeData()->urls();
    qDebug() << "target item: " << item->groupName() << "|" << item->text() <<  "|" << item->url();

    DUrlList urls;
    for (const QUrl &url : event->mimeData()->urls()) {
        if (DUrl(url).parentUrl() == item->url()) {
            qDebug() << "skip the same dir file..." << url;
        } else {
            urls << url;
        }
    }

    Qt::DropAction action = canDropMimeData(item, event->mimeData(), event->proposedAction());
    if (action == Qt::IgnoreAction) {
        action = canDropMimeData(item, event->mimeData(), event->possibleActions());
    }

    if (urls.size() > 0 && onDropData(urls, item->url(), action)) {
        event->setDropAction(action);
        event->accept();
        return;
    }

    DListView::dropEvent(event);
}

bool DFMSideBarView::onDropData(DUrlList srcUrls, DUrl dstUrl, Qt::DropAction action) const
{
    const DAbstractFileInfoPointer &dstInfo = fileService->createFileInfo(this, dstUrl);
    if (!dstInfo) {
        return false;
    }

    for (DUrl &u : srcUrls) {
        // we only do redirection for burn:// urls for the fear of screwing everything up again
        if (u.scheme() == BURN_SCHEME) {
            DAbstractFileInfoPointer fi = fileService->createFileInfo(nullptr, u);
            while (fi && fi->canRedirectionFileUrl()) {
                u = fi->redirectedFileUrl();
                fi = fileService->createFileInfo(nullptr, u);
            }
        }
    }

    // convert destnation url to real path if it's a symbol link.
    if (dstInfo->isSymLink()) {
        dstUrl = dstInfo->rootSymLinkTarget();
    }

    switch (action) {
    case Qt::CopyAction:
        // blumia: should run in another thread or user won't do another DnD opreation unless the copy action done.
        QtConcurrent::run([=](){
            fileService->pasteFile(this, DFMGlobal::CopyAction, dstUrl, srcUrls);
        });
        break;
    case Qt::LinkAction:
        break;
    case Qt::MoveAction:
        fileService->pasteFile(this, DFMGlobal::CutAction, dstUrl, srcUrls);
        break;
    default:
        return false;
    }

    return true;
}

DFMSideBarItem *DFMSideBarView::itemAt(const QPoint &pt)
{
    DFMSideBarItem *item = nullptr;
    QModelIndex index = indexAt(pt);
    if (!index.isValid()) {
        return item;
    }

    DFMSideBarModel *mod = dynamic_cast<DFMSideBarModel *>(model());
    Q_ASSERT(mod);
    item = mod->itemFromIndex(index);
    Q_ASSERT(item);

    return item;
}

Qt::DropAction DFMSideBarView::canDropMimeData(DFMSideBarItem *item, const QMimeData *data, Qt::DropActions actions) const
{
    // Got a copy of urls so whatever data was changed, it won't affact the following code.
    QList<QUrl> urls = data->urls();

    if (urls.empty()) {
        return Qt::IgnoreAction;
    }

    for (const QUrl &url : urls) {
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(this, url);
        if (!fileInfo || !fileInfo->isReadable()) {
            return Qt::IgnoreAction;
        }
    }

    const DAbstractFileInfoPointer &info = fileService->createFileInfo(this, item->url());

    if (!info || !info->canDrop()) {
        return Qt::IgnoreAction;
    }

    const Qt::DropActions support_actions = info->supportedDropActions() & actions;

    if (DStorageInfo::inSameDevice(urls.first(), item->url())) {
        if (support_actions.testFlag(Qt::MoveAction)) {
            return Qt::MoveAction;
        }
    }

    if (support_actions.testFlag(Qt::CopyAction)) {
        return Qt::CopyAction;
    }

    if (support_actions.testFlag(Qt::MoveAction)) {
        return Qt::MoveAction;
    }

    if (support_actions.testFlag(Qt::LinkAction)) {
        return Qt::LinkAction;
    }

    return Qt::IgnoreAction;
}

bool DFMSideBarView::isAccepteDragEvent(DFMDragEvent *event)
{
    DFMSideBarItem *item = itemAt(event->pos());
    if (!item) {
        return false;
    }

    bool accept = false;
    Qt::DropAction action = canDropMimeData(item, event->mimeData(), event->proposedAction());
    if (action == Qt::IgnoreAction) {
        action = canDropMimeData(item, event->mimeData(), event->possibleActions());
    }

    if (action != Qt::IgnoreAction) {
       event->setDropAction(action);
       event->accept();
       accept = true;
    }

    return accept;
}

DFM_END_NAMESPACE
