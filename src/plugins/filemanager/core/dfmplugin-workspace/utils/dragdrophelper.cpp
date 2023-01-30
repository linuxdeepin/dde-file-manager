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
#include "events/workspaceeventsequence.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/windowutils.h"
#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/utils/universalutils.h"
#include "dfm-base/utils/universalutils.h"

#include <dfm-framework/event/event.h>

#include <DFileDragClient>
#include <QMimeData>

Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QList<QUrl> *)

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

DragDropHelper::DragDropHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
}

bool DragDropHelper::dragEnter(QDragEnterEvent *event)
{
    currentHoverIndexUrl = QUrl();
    const QMimeData *data = event->mimeData();
    currentDragUrls = data->urls();

    // Filter the event that cannot be dragged
    if (checkProhibitPaths(event, currentDragUrls))
        return true;

    if (handleDFileDrag(data, view->rootUrl())) {
        event->acceptProposedAction();
        return true;
    }

    for (const QUrl &url : currentDragUrls) {
        auto info = InfoFactory::create<AbstractFileInfo>(url);
        if (!info || !info->canAttributes(CanableInfoType::kCanMoveOrCopy)) {
            event->ignore();
            return true;
        }
    }

    if (!currentDragUrls.isEmpty()) {
        QList<QUrl> urls {};
        bool ok = dpfHookSequence->run("dfmplugin_utils", "hook_UrlsTransform", currentDragUrls, &urls);

        if (ok && !urls.isEmpty())
            const_cast<QMimeData *>(event->mimeData())->setUrls(urls);
    }

    bool fall = true;
    handleDropEvent(event, &fall);

    if (!fall)
        return true;

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
        bool fall = true;
        handleDropEvent(event, &fall);

        if (!fall)
            return true;

        QUrl toUrl = hoverFileInfo->urlOf(UrlInfoType::kUrl);
        QList<QUrl> fromUrls = event->mimeData()->urls();
        Qt::DropAction dropAction = event->dropAction();
        if (dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDragMove", fromUrls, toUrl, &dropAction)) {
            event->setDropAction(dropAction);
            view->setViewSelectState(false);
            if (dropAction != Qt::IgnoreAction)
                event->accept();
            else
                event->ignore();
            currentHoverIndexUrl = toUrl;
            return true;
        }

        for (const QUrl &url : fromUrls) {
            AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
            if (event->dropAction() == Qt::DropAction::MoveAction && !info->canAttributes(CanableInfoType::kCanRename) && !dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileCanMove", url)) {
                view->setViewSelectState(false);
                event->ignore();
                return true;
            }

            // target is not local device, origin is dir and can not write, prohibit drop
            const QUrl &targetUrl = hoverFileInfo->urlOf(UrlInfoType::kUrl);
            if (!FileUtils::isLocalDevice(targetUrl)) {
                if (!info->isAttributes(OptInfoType::kIsWritable)) {
                    view->setViewSelectState(false);
                    event->ignore();
                    return true;
                }
            }
        }

        if (!hoverFileInfo->canAttributes(CanableInfoType::kCanDrop)
            || !hoverFileInfo->supportedOfAttributes(SupportedType::kDrop).testFlag(event->dropAction())
            || (hoverFileInfo->isAttributes(OptInfoType::kIsDir) && !hoverFileInfo->isAttributes(OptInfoType::kIsWritable))) {
            // NOTE: if item can not drop, the drag item will drop to root dir.
            currentHoverIndexUrl = view->rootUrl();
            if (event->source() == view) {
                view->setViewSelectState(false);
                event->ignore();
            } else {
                view->setViewSelectState(true);
                event->accept();
            }
            return true;
        }

        if (!handleDFileDrag(event->mimeData(), hoverFileInfo->urlOf(UrlInfoType::kUrl))) {
            currentHoverIndexUrl = toUrl;
            bool b = (event->source() == view);
            if (b && !WindowUtils::keyCtrlIsPressed() && UniversalUtils::urlEquals(toUrl, view->rootUrl())) {
                view->setViewSelectState(false);
                event->ignore();
            } else if (!b && !WindowUtils::keyCtrlIsPressed() && UniversalUtils::urlEquals(toUrl, view->rootUrl())) {
                view->setViewSelectState(true);
                event->accept();
            } else if (WindowUtils::keyCtrlIsPressed() && UniversalUtils::urlEquals(toUrl, view->rootUrl())) {
                view->setViewSelectState(true);
                event->accept();
            } else if (hoverFileInfo->isAttributes(OptInfoType::kIsDir) || FileUtils::isDesktopFile(hoverFileInfo->urlOf(UrlInfoType::kUrl))) {
                view->setViewSelectState(false);
                event->accept();
            }
        } else {
            view->setViewSelectState(false);
            event->ignore();
        }

        return true;
    }

    return false;
}

bool DragDropHelper::dragLeave(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
    currentHoverIndexUrl = QUrl();
    currentDragUrls.clear();
    return false;
}

bool DragDropHelper::drop(QDropEvent *event)
{
    currentHoverIndexUrl = QUrl();
    bool fall = true;
    handleDropEvent(event, &fall);

    if (!fall)
        return true;

    if (event->mimeData()->property("IsDirectSaveMode").toBool()) {
        event->setDropAction(Qt::CopyAction);

        AbstractFileInfoPointer info = fileInfoAtPos(event->pos());
        if (info && info->urlOf(UrlInfoType::kUrl).isLocalFile()) {
            if (info->isAttributes(OptInfoType::kIsDir)) {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", info->urlOf(UrlInfoType::kUrl));
            } else {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", info->urlOf(UrlInfoType::kParentUrl));
            }
        }

        event->accept();
    } else {
        QModelIndex hoverIndex = view->indexAt(event->pos());

        if (event->source() == view && (!hoverIndex.isValid() || view->isSelected(hoverIndex)) && !WindowUtils::keyCtrlIsPressed())
            return true;

        if (!hoverIndex.isValid()) {
            hoverIndex = view->rootIndex();
        } else {
            const AbstractFileInfoPointer &fileInfo = view->model()->itemFileInfo(hoverIndex);
            if (fileInfo) {
                bool isDrop = dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_IsDrop", fileInfo->urlOf(UrlInfoType::kUrl));
                // NOTE: if item can not drop, the drag item will drop to root dir.
                if (fileInfo->isAttributes(OptInfoType::kIsFile)
                    && !FileUtils::isDesktopFile(fileInfo->urlOf(UrlInfoType::kUrl))
                    && !isDrop)
                    hoverIndex = view->rootIndex();
            }
        }

        if (!hoverIndex.isValid())
            return true;

        QUrl toUrl = view->model()->getUrlByIndex(hoverIndex);
        QList<QUrl> fromUrls = event->mimeData()->urls();
        if (dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDrop", fromUrls, toUrl)) {
            return true;
        }

        bool supportDropAction = view->model()->supportedDropActions() & event->dropAction();
        bool dropEnabled = view->model()->flags(hoverIndex) & Qt::ItemIsDropEnabled;
        if (supportDropAction && dropEnabled) {
            const Qt::DropAction action = view->dragDropMode() == QAbstractItemView::InternalMove
                    ? Qt::MoveAction
                    : event->dropAction();
            bool isDropped = view->model()->dropMimeData(event->mimeData(), action, hoverIndex.row(), hoverIndex.column(), hoverIndex.parent());
            if (isDropped) {
                if (action != event->dropAction()) {
                    event->setDropAction(action);
                    event->accept();
                } else {
                    event->acceptProposedAction();
                }
            }

            view->selectionModel()->clear();
            if (event->isAccepted())
                return true;   // TODO (xust) this is a temp workaround.
        }
    }

    return false;
}

bool DragDropHelper::isDragTarget(const QModelIndex &index) const
{
    auto info = view->model()->itemFileInfo(index);
    if (info)
        return UniversalUtils::urlEquals(info->urlOf(UrlInfoType::kUrl), currentHoverIndexUrl);

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

void DragDropHelper::handleDropEvent(QDropEvent *event, bool *fall)
{
    const bool sameUser = SysInfoUtils::isSameUser(event->mimeData());

    auto checkEventCustom = [this, sameUser](QDropEvent *event, const QList<QUrl> &urls, const QUrl &urlTo, Qt::DropAction *action) {
        if (WorkspaceEventSequence::instance()->doCheckDragTarget(urls, urlTo, action)) {
            switch (*action) {
            case Qt::IgnoreAction: {
                event->ignore();
                return true;
            }
            default:
                event->setDropAction(checkAction(*action, sameUser));
            }
        }
        return false;
    };

    if (event->source() == view && !WindowUtils::keyCtrlIsPressed()) {
        Qt::DropAction defaultAction = Qt::MoveAction;
        if (checkEventCustom(event, currentDragUrls, view->rootUrl(), &defaultAction)) {
            *fall = false;
            return;
        } else {
            event->setDropAction(checkAction(defaultAction, sameUser));
        }
    } else {
        if (currentDragUrls.isEmpty())
            return;

        AbstractFileInfoPointer info = fileInfoAtPos(event->pos());
        if (!info)
            return;

        Qt::DropAction defaultAction = Qt::CopyAction;
        if (WindowUtils::keyAltIsPressed()) {
            defaultAction = Qt::MoveAction;
        } else if (!WindowUtils::keyCtrlIsPressed()
                   && FileUtils::isSameDevice(currentDragUrls.first(),
                                              info->canAttributes(CanableInfoType::kCanRedirectionFileUrl)
                                                      ? info->urlOf(UrlInfoType::kRedirectedFileUrl)
                                                      : info->urlOf(UrlInfoType::kUrl))) {
            defaultAction = Qt::MoveAction;
        }

        if (checkEventCustom(event, currentDragUrls, view->rootUrl(), &defaultAction)) {
            *fall = false;
            return;
        }

        if (event->possibleActions().testFlag(defaultAction))
            event->setDropAction(checkAction(defaultAction, sameUser));

        if (!info->supportedOfAttributes(SupportedType::kDrop).testFlag(event->dropAction())) {
            QList<Qt::DropAction> actions;
            actions.reserve(3);
            actions << Qt::CopyAction << Qt::MoveAction << Qt::LinkAction;

            for (Qt::DropAction action : actions) {
                if (event->possibleActions().testFlag(action) && info->supportedOfAttributes(SupportedType::kDrop).testFlag(action)) {
                    event->setDropAction(checkAction(action, sameUser));
                    break;
                }
            }
        }
    }
}

QSharedPointer<AbstractFileInfo> DragDropHelper::fileInfoAtPos(const QPoint &pos)
{
    QModelIndex index = view->indexAt(pos);
    if (!index.isValid())
        index = view->rootIndex();

    return view->model()->itemFileInfo(index);
}

bool DragDropHelper::checkProhibitPaths(QDragEnterEvent *event, const QList<QUrl> &urls) const
{
    // Filter the event that cannot be dragged
    if (urls.isEmpty() || FileUtils::isContainProhibitPath(urls)) {
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
        return true;
    }

    return false;
}

Qt::DropAction DragDropHelper::checkAction(Qt::DropAction srcAction, bool sameUser)
{
    if (!sameUser && srcAction == Qt::MoveAction)
        return Qt::IgnoreAction;

    return srcAction;
}
