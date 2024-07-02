// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dragdropoper.h"
#include "utils/keyutil.h"
#include "grid/canvasgrid.h"
#include "model/canvasselectionmodel.h"
#include "view/canvasview_p.h"
#include "canvasmanager.h"
#include "displayconfig.h"
#include "utils/fileutil.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/mimedata/dfmmimedata.h>

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
    updateDFMMimeData(event);
    updateDragHover(event->pos());

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

    updateDragHover(QPoint(-1, -1));
}

bool DragDropOper::move(QDragMoveEvent *event)
{
    stopDelayDodge();
    updateDragHover(event->pos());

    auto pos = event->pos();
    auto hoverIdx = view->baseIndexAt(pos);
    // extend
    if (hoverIdx.isValid() && view->d->hookIfs) {
        QUrl hoverUrl = view->model()->fileUrl(hoverIdx);
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
    QUrl curUrl = hoverIdx.isValid() ? view->model()->fileUrl(hoverIdx) : view->model()->rootUrl();
    if (!checkTargetEnable(curUrl)) {
        event->ignore();
    } else if (hoverIdx.isValid()) {
        if (auto fileInfo = view->model()->fileInfo(hoverIdx)) {
            bool canDrop = !fileInfo->canAttributes(CanableInfoType::kCanDrop) || (fileInfo->isAttributes(OptInfoType::kIsDir) && !fileInfo->isAttributes(OptInfoType::kIsWritable)) || !fileInfo->supportedOfAttributes(SupportedType::kDrop).testFlag(event->dropAction());
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
    if (!hoverIdx.isValid())
        handleMoveMimeData(event, curUrl);

    return true;
}

bool DragDropOper::drop(QDropEvent *event)
{
    stopDelayDodge();
    updatePrepareDodgeValue(event);

    // extend
    if (view->d->hookIfs) {
        preproccessDropEvent(event, event->mimeData()->urls(), view->model()->rootUrl());
        QVariantHash ext;
        ext.insert("QDropEvent", reinterpret_cast<qlonglong>(event));
        QUrl dropUrl;
        QModelIndex dropIndex = view->baseIndexAt(event->pos());
        if (dropIndex.isValid())
            dropUrl = view->model()->fileUrl(dropIndex);
        else
            dropUrl = view->model()->rootUrl();

        ext.insert("dropUrl", QVariant(dropUrl));
        if (view->d->hookIfs->dropData(view->screenNum(), event->mimeData(), event->pos(), &ext)) {
            fmInfo() << "data droped by extend";
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

        if (event->mimeData() && !event->mimeData()->hasFormat(DFMGLOBAL_NAMESPACE::Mime::kDFMAppTypeKey))
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

        // is from trash
        if (FileUtils::isTrashFile(from))
            defaultAction = Qt::MoveAction;

        const bool sameUser = SysInfoUtils::isSameUser(event->mimeData());
        if (event->possibleActions().testFlag(defaultAction))
            event->setDropAction((defaultAction == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : defaultAction);

        if (!itemInfo->supportedOfAttributes(SupportedType::kDrop).testFlag(event->dropAction())) {
            QList<Qt::DropAction> actions;

            actions.reserve(3);
            actions << Qt::CopyAction << Qt::MoveAction << Qt::LinkAction;

            for (Qt::DropAction action : actions) {
                if (event->possibleActions().testFlag(action) && itemInfo->supportedOfAttributes(SupportedType::kDrop).testFlag(action)) {
                    event->setDropAction((action == Qt::MoveAction && !sameUser) ? Qt::IgnoreAction : action);
                    break;
                }
            }
        }
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
    if (!urlsForDragEvent.isEmpty() && FileUtils::isContainProhibitPath(urlsForDragEvent)) {
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
            auto v = std::find_if(allView.begin(), allView.end(), [itor](const QSharedPointer<CanvasView> &v) {
                return v->screenNum() == itor.key();
            });

            if (v != allView.end()) {
                (*v)->d->operState().setCurrent(itor.value());
                (*v)->d->operState().setContBegin(itor.value());
            }
        }
    }
}

bool DragDropOper::dropFilter(QDropEvent *event)
{
    //Prevent the desktop's computer/recycle bin/home directory from being dragged and copied to other directories
    {
        QModelIndex index = view->baseIndexAt(event->pos());
        if (index.isValid()) {
            QUrl targetItem = view->model()->fileUrl(index);
            auto itemInfo = FileCreator->createFileInfo(targetItem);
            if (itemInfo && (itemInfo->isAttributes(OptInfoType::kIsDir) || itemInfo->urlOf(UrlInfoType::kUrl) == DesktopAppUrl::homeDesktopFileUrl())) {
                auto sourceUrls = event->mimeData()->urls();
                bool find = std::any_of(sourceUrls.begin(), sourceUrls.end(), [](const QUrl &url) {
                    return (DesktopAppUrl::computerDesktopFileUrl() == url)
                            || (DesktopAppUrl::trashDesktopFileUrl() == url)
                            || (DesktopAppUrl::homeDesktopFileUrl() == url);
                });

                if (find) {
                    event->setDropAction(Qt::IgnoreAction);
                    return true;
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
        fmWarning() << "drop on" << m_target;

        QList<QUrl> urlList = data->urls();
        if (!urlList.isEmpty()) {
            // todo 排查哪些情况会进这里
            //Q_ASSERT(false);
            DFileDragClient *client = new DFileDragClient(data, const_cast<DragDropOper *>(this));
            fmDebug() << "dragClientDownload" << client << data << urlList;
            connect(client, &DFileDragClient::stateChanged, this, [this, urlList](DFileDragState state) {
                if (state == Finished)
                    selectItems(urlList);
                fmDebug() << "stateChanged" << state << urlList;
            });

            connect(client, &DFileDragClient::serverDestroyed, client, &DFileDragClient::deleteLater);
            connect(client, &DFileDragClient::destroyed, []() {
                fmDebug() << "drag client deleted";
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
    auto dropIndex = view->baseIndexAt(dropRect.center());

    auto targetIndex = view->baseIndexAt(event->pos());
    bool dropOnSelf = targetIndex.isValid() ? view->selectionModel()->selectedIndexesCache().contains(targetIndex) : false;

    // process this case in other drop function(e.g. move) if targetGridPos is used and it is not drop-needed.
    if (dropIndex.isValid() && !dropOnSelf) {
        if (!targetIndex.isValid()) {
            fmInfo() << "drop on invaild target, skip. drop:" << dropGridPos.x() << dropGridPos.y();
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

    if (itemPos.isEmpty()) {
        fmWarning() << "can not drop invaild items" << sourceUrls;
        return false;
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
        resetFocus(dropGridPos);
        fmDebug() << "append items " << itemPos.first() << "begin" << view->screenNum() << dropGridPos << itemPos.size();
    } else if (itemfrom.size() == 1) {
        // items are from one view, using move.
        // normally, item should from the view that is event->source().
        auto focus = fromView->d->operState().current();
        auto focusItem = fromView->model()->fileUrl(focus).toString();
        if (!focusItem.isEmpty()) {
            if (GridIns->move(view->screenNum(), dropGridPos, focusItem, itemPos.keys())) {
                // reset the focus for key move
                resetFocus(dropGridPos);
                fmDebug() << "move items" << focusItem << itemPos.value(focusItem) << "to"
                          << view->screenNum() << dropGridPos << "count" << itemPos.size();
            }
        } else {
            fmWarning() << "can not find fcous." << focus << fromView->screenNum();
        }
    } else {
        fmWarning() << "can not find drop item.";
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
    // NOTE: The following code sets the properties that will be used
    // in the project `linuxdeepin/qt5platform-plugins`.
    // The purpose is to support dragging a file from a archive to extract it to the dde-filemanager
    if (event->mimeData()->property("IsDirectSaveMode").toBool()) {
        event->setDropAction(Qt::CopyAction);
        const QModelIndex &index = view->baseIndexAt(event->pos());
        auto fileInfo = view->model()->fileInfo(index.isValid() ? index : view->rootIndex());

        if (fileInfo && dfmbase::FileUtils::isLocalFile(fileInfo->urlOf(UrlInfoType::kUrl))) {
            if (fileInfo->isAttributes(OptInfoType::kIsDir))
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->urlOf(UrlInfoType::kUrl));
            else
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", fileInfo->urlOf(UrlInfoType::kParentUrl));
        }

        event->accept();   // yeah! we've done with XDS so stop Qt from further event propagation.
        return true;
    }

    return false;
}

bool DragDropOper::dropMimeData(QDropEvent *event) const
{
    auto model = view->model();
    auto targetIndex = view->baseIndexAt(event->pos());
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
    } else if (WindowUtils::isWayLand()) {
        // Bug-209635，wayland下wine应用拖拽时，QDragEvent中的action为Qt::IgnoreActon
        // 当拖拽事件action无效时，判断文件来源，wine应用默认做CopyAction处理
        QList<QUrl> urls = event->mimeData()->urls();

        if (!urls.isEmpty()) {
            const QUrl from = QUrl(urls.first());
            if (!from.path().contains("/.deepinwine/"))
                return false;
            if (model->dropMimeData(event->mimeData(), Qt::CopyAction, targetIndex.row(), targetIndex.column(), targetIndex))
                event->acceptProposedAction();
            return true;
        }
    }
    return false;
}

void DragDropOper::handleMoveMimeData(QDropEvent *event, const QUrl &url)
{
    if (DFileDragClient::checkMimeData(event->mimeData())) {
        event->acceptProposedAction();
        // update target url if mouse focus is on file which can drop.
        updateTarget(event->mimeData(), url);
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

void DragDropOper::updateDragHover(const QPoint &pos)
{
    //update the old one
    view->update(dragHoverIndex);

    dragHoverIndex = view->baseIndexAt(pos);
    // update the new one
    view->update(dragHoverIndex);
    return;
}

void DragDropOper::stopDelayDodge()
{
    view->d->dodgeOper->stopDelayDodge();
}

void DragDropOper::updateDFMMimeData(QDropEvent *event)
{
    dfmmimeData.clear();
    const QMimeData *data = event->mimeData();
    if (data && data->hasFormat(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey))
        dfmmimeData = DFMMimeData::fromByteArray(data->data(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey));
}

bool DragDropOper::checkTargetEnable(const QUrl &targetUrl)
{
    if (!dfmmimeData.isValid())
        return true;

    if (FileUtils::isTrashDesktopFile(targetUrl)) {
        if (dfmmimeData.isTrashFile())
            return false;
        return dfmmimeData.canTrash() || dfmmimeData.canDelete();
    }

    return true;
}
