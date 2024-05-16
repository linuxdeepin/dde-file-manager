// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dragdrophelper.h"
#include "views/fileview.h"
#include "models/fileviewmodel.h"
#include "events/workspaceeventsequence.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/mimedata/dfmmimedata.h>

#include <dfm-framework/event/event.h>

#include <DFileDragClient>
#include <QMimeData>

Q_DECLARE_METATYPE(Qt::DropAction *)
Q_DECLARE_METATYPE(QList<QUrl> *)

DFMBASE_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace dfmplugin_workspace;

DragDropHelper::DragDropHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
}

bool DragDropHelper::dragEnter(QDragEnterEvent *event)
{
    dfmmimeData.clear();
    currentHoverIndexUrl = QUrl();
    const QMimeData *data = event->mimeData();
    currentDragUrls = data->urls();

    if (data->formats().contains(DFMGLOBAL_NAMESPACE::Mime::kDFMTreeUrlsKey)) {
        currentDragUrls.clear();
        auto treeUrlsStr = QString(data->data(DFMGLOBAL_NAMESPACE::Mime::kDFMTreeUrlsKey));
        auto treeUrlss = treeUrlsStr.split("\n");
        for (const auto &url : treeUrlss) {
            if (url.isEmpty())
                continue;
            currentDragUrls.append(QUrl(url));
        }
    }

    if (data->hasFormat(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey))
        dfmmimeData = DFMMimeData::fromByteArray(data->data(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey));
    currentDragSourceUrls = dfmmimeData.isValid() ? dfmmimeData.urls() : currentDragUrls;

    if (!checkTargetEnable(view->rootUrl()))
        return true;

    // Filter the event that cannot be dragged
    if (checkProhibitPaths(event, currentDragUrls))
        return true;

    if (handleDFileDrag(data, view->rootUrl())) {
        event->acceptProposedAction();
        return true;
    }

    for (const QUrl &url : currentDragUrls) {
        auto info = InfoFactory::create<FileInfo>(url);
        if (!info || !info->canAttributes(CanableInfoType::kCanMoveOrCopy)) {
            event->ignore();
            return true;
        }
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
    FileInfoPointer hoverFileInfo = fileInfoAtPos(event->pos());
    if (!hoverFileInfo)
        return false;

    bool fall = true;
    handleDropEvent(event, &fall);

    if (!fall)
        return true;

    QUrl toUrl = hoverFileInfo->urlOf(UrlInfoType::kUrl);
    if (!checkTargetEnable(toUrl) || !hoverFileInfo->isAttributes(OptInfoType::kIsWritable)) {
        event->ignore();
        currentHoverIndexUrl = toUrl;
        return true;
    }

    dragFileFromCurrent = false;
    QList<QUrl> fromUrls = currentDragUrls;
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
        FileInfoPointer info = InfoFactory::create<FileInfo>(url);
        if (event->dropAction() == Qt::DropAction::MoveAction && !info->canAttributes(CanableInfoType::kCanRename) && !dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileCanMove", url)) {
            view->setViewSelectState(false);
            event->ignore();
            return true;
        }

        // target is not local device, origin is dir and can not write, prohibit drop
        const QUrl &targetUrl = hoverFileInfo->urlOf(UrlInfoType::kUrl);
        if (!hoverFileInfo->extendAttributes(ExtInfoType::kFileLocalDevice).toBool()) {
            if (!info->isAttributes(OptInfoType::kIsWritable)) {
                view->setViewSelectState(false);
                event->ignore();
                return true;
            }
        }

        if (UniversalUtils::urlEquals(targetUrl, url)) {
            view->setViewSelectState(false);
            event->ignore();
            return true;
        }

        if (UniversalUtils::isParentOnly(url, view->rootUrl()))
            dragFileFromCurrent = true;
    }

    if (!hoverFileInfo->canAttributes(CanableInfoType::kCanDrop)
        || !hoverFileInfo->supportedOfAttributes(SupportedType::kDrop).testFlag(event->dropAction())
        || (hoverFileInfo->isAttributes(OptInfoType::kIsDir) && !hoverFileInfo->isAttributes(OptInfoType::kIsWritable))) {
        // NOTE: if item can not drop, the drag item will drop to root dir.
        currentHoverIndexUrl = view->rootUrl();
        if (event->source() == view && dragFileFromCurrent) {
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
            if (dragFileFromCurrent) {
                view->setViewSelectState(false);
                event->ignore();
            } else {
                view->setViewSelectState(true);
                event->accept();
            }
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
        event->acceptProposedAction();
    }

    return true;
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
    // only handle ignore
    if (!fall && !event->isAccepted())
        return true;

    // NOTE: The following code sets the properties that will be used
    // in the project `linuxdeepin/qt5platform-plugins`.
    // The purpose is to support dragging a file from a archive to extract it to the dde-filemanager
    if (event->mimeData()->property("IsDirectSaveMode").toBool()) {
        event->setDropAction(Qt::CopyAction);

        FileInfoPointer info = fileInfoAtPos(event->pos());
        if (info && dfmbase::FileUtils::isLocalFile(info->urlOf(UrlInfoType::kUrl))) {
            if (info->isAttributes(OptInfoType::kIsDir)) {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", info->urlOf(UrlInfoType::kUrl));
            } else {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", info->urlOf(UrlInfoType::kParentUrl));
            }
        }

        event->accept();
    } else {
        QModelIndex hoverIndex = view->indexAt(event->pos());

        if (event->source() == view && dragFileFromCurrent &&
                (!hoverIndex.isValid() || view->isSelected(hoverIndex)) &&
                !WindowUtils::keyCtrlIsPressed())
            return true;

        if (!hoverIndex.isValid()) {
            hoverIndex = view->rootIndex();
        } else {
            FileInfoPointer fileInfo = view->model()->fileInfo(hoverIndex);
            if (fileInfo.isNull())
                hoverIndex.data(Global::ItemRoles::kItemCreateFileInfoRole);
            fileInfo = view->model()->fileInfo(hoverIndex);

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

        QUrl toUrl = view->model()->data(hoverIndex, DFMGLOBAL_NAMESPACE::ItemRoles::kItemUrlRole).toUrl();
        if (dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDrop", currentDragSourceUrls, toUrl))
            return true;

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
        } else if (WindowUtils::isWayLand()) {
            // Bug-209635，wayland下wine应用拖拽时，QDragEvent中的action为Qt::IgnoreActon
            // 当拖拽事件action无效时，判断文件来源，wine应用默认做CopyAction处理
            QList<QUrl> urls = event->mimeData()->urls();

            if (!urls.isEmpty()) {
                const QUrl from = QUrl(urls.first());
                if (from.path().contains("/.deepinwine/")
                        && view->model()->dropMimeData(event->mimeData(), Qt::CopyAction, hoverIndex.row(), hoverIndex.column(), hoverIndex.parent())) {
                    event->acceptProposedAction();
                }
            }
        }
    }

    return false;
}

bool DragDropHelper::isDragTarget(const QModelIndex &index) const
{
    auto info = view->model()->fileInfo(index);
    if (info)
        return UniversalUtils::urlEquals(info->urlOf(UrlInfoType::kUrl), currentHoverIndexUrl);

    return false;
}

bool DragDropHelper::handleDFileDrag(const QMimeData *data, const QUrl &url)
{
    // for archive manager
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
            default: {
                auto dropAction { checkAction(*action, sameUser) };
                event->setDropAction(dropAction);
                // set failed
                if (dropAction != event->dropAction()) {
                    event->accept();
                    return true;
                }
            }
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

        FileInfoPointer info = fileInfoAtPos(event->pos());
        if (!info)
            return;

        if (event->mimeData() && !event->mimeData()->hasFormat(DFMGLOBAL_NAMESPACE::Mime::kDFMAppTypeKey))
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

QSharedPointer<FileInfo> DragDropHelper::fileInfoAtPos(const QPoint &pos)
{
    QModelIndex index = view->indexAt(pos);
    if (!index.isValid())
        index = view->rootIndex();

    return view->model()->fileInfo(index);
}

bool DragDropHelper::checkProhibitPaths(QDragEnterEvent *event, const QList<QUrl> &urls) const
{
    // Filter the event that cannot be dragged
    if (!urls.isEmpty() && FileUtils::isContainProhibitPath(urls)) {
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
        return true;
    }

    return false;
}

bool DragDropHelper::checkTargetEnable(const QUrl &targetUrl) const
{
    if (!dfmmimeData.isValid())
        return true;

    if (FileUtils::isTrashFile(targetUrl) || FileUtils::isTrashDesktopFile(targetUrl))
        return dfmmimeData.canTrash() || dfmmimeData.canDelete();

    return true;
}

Qt::DropAction DragDropHelper::checkAction(Qt::DropAction srcAction, bool sameUser)
{
    if (!sameUser && srcAction == Qt::MoveAction)
        return Qt::IgnoreAction;

    return srcAction;
}
