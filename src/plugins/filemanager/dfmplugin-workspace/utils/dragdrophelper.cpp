// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
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
        fmDebug() << "Processing tree URLs from mime data";
        currentDragUrls.clear();
        auto treeUrlsStr = QString(data->data(DFMGLOBAL_NAMESPACE::Mime::kDFMTreeUrlsKey));
        auto treeUrlss = treeUrlsStr.split("\n");
        for (const auto &url : treeUrlss) {
            if (url.isEmpty())
                continue;
            currentDragUrls.append(QUrl(url));
        }
        fmDebug() << "Tree URLs processed - count:" << currentDragUrls.size();
    }

    if (data->hasFormat(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey)) {
        fmDebug() << "Processing DFM mime data";
        dfmmimeData = DFMMimeData::fromByteArray(data->data(DFMGLOBAL_NAMESPACE::Mime::kDFMMimeDataKey));
    }
    currentDragSourceUrls = dfmmimeData.isValid() ? dfmmimeData.urls() : currentDragUrls;

    auto targetUrl = view->rootUrl();
    fmDebug() << "Target URL:" << targetUrl.toString();
    if (!checkTargetEnable(targetUrl)) {
        fmDebug() << "Target not enabled for drag operations";
        return true;
    }

    // Filter the event that cannot be dragged
    if (checkProhibitPaths(event, currentDragUrls)) {
        fmDebug() << "Drag operation prohibited by path restrictions";
        return true;
    }

    if (handleDFileDrag(data, targetUrl)) {
        fmDebug() << "DFile drag handled successfully";
        event->acceptProposedAction();
        return true;
    }

    for (const QUrl &url : currentDragUrls) {
        if (checkDragEnable(url, targetUrl))
            continue;

        fmDebug() << "Drag operation not enabled for URL:" << url.toString();
        event->ignore();
        return true;
    }

    bool fall = true;
    handleDropEvent(event, &fall);
    if (!fall) {
        fmDebug() << "Drop event handled by custom logic";
        return true;
    }

    if (event->mimeData()->hasFormat("XdndDirectSave0")) {
        fmDebug() << "Direct save mode detected - setting copy action";
        event->setDropAction(Qt::CopyAction);
        event->acceptProposedAction();
        return true;
    }

    return false;
}

bool DragDropHelper::dragMove(QDragMoveEvent *event)
{
    FileInfoPointer hoverFileInfo = fileInfoAtPos(event->pos());
    if (!hoverFileInfo) {
        fmDebug() << "No file info at drag position";
        return false;
    }

    bool fall = true;
    handleDropEvent(event, &fall);

    if (!fall) {
        fmDebug() << "Drop event handled by custom logic in drag move";
        return true;
    }

    QUrl toUrl = hoverFileInfo->urlOf(UrlInfoType::kUrl);
    // NOTE: if drag file hover on a file item in list view, allow drop the file to root dir.
    // so, check kCanDrop attributes of dir file only
    if (!checkTargetEnable(toUrl)
        || (!hoverFileInfo->canAttributes(CanableInfoType::kCanDrop) && hoverFileInfo->isAttributes(OptInfoType::kIsDir))) {
        fmDebug() << "Target not suitable for drop - ignoring event";
        event->ignore();
        currentHoverIndexUrl = toUrl;
        return true;
    }

    dragFileFromCurrent = false;
    QList<QUrl> fromUrls = currentDragUrls;
    Qt::DropAction dropAction = event->dropAction();
    if (dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDragMove", fromUrls, toUrl, &dropAction)) {
        fmDebug() << "Drag move handled by hook - drop action:" << dropAction;
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
        if (event->dropAction() == Qt::DropAction::MoveAction) {
            if (!checkMoveEnable(url, toUrl)) {
                fmDebug() << "Move operation not enabled for URL:" << url.toString();
                view->setViewSelectState(false);
                event->ignore();
                return true;
            }
        }

        // copy action must origin file can copy
        const QUrl &targetUrl = hoverFileInfo->urlOf(UrlInfoType::kUrl);
        FileInfoPointer info = InfoFactory::create<FileInfo>(url);
        if (event->dropAction() == Qt::DropAction::CopyAction && info->exists() && !info->canAttributes(CanableInfoType::kCanMoveOrCopy)) {
            fmDebug() << "Copy operation not allowed for URL:" << url.toString();
            view->setViewSelectState(false);
            event->ignore();
            return true;
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
        } else if (hoverFileInfo->isAttributes(OptInfoType::kIsDir) || FileUtils::isDesktopFileSuffix(hoverFileInfo->urlOf(UrlInfoType::kUrl))) {
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
    fmDebug() << "Drag leave event - clearing state";
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
    if (!fall && !event->isAccepted()) {
        fmDebug() << "Drop event handled by custom logic";
        return true;
    }

    // NOTE: The following code sets the properties that will be used
    // in the project `linuxdeepin/qt5platform-plugins`.
    // The purpose is to support dragging a file from a archive to extract it to the dde-filemanager
    if (event->mimeData()->property("IsDirectSaveMode").toBool()) {
        fmDebug() << "Direct save mode detected - setting copy action";
        event->setDropAction(Qt::CopyAction);

        FileInfoPointer info = fileInfoAtPos(event->pos());
        if (info && info->urlOf(UrlInfoType::kUrl).isLocalFile()) {
            if (info->isAttributes(OptInfoType::kIsDir)) {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", info->urlOf(UrlInfoType::kUrl));
                fmDebug() << "Direct save URL set to directory:" << info->urlOf(UrlInfoType::kUrl).toString();
            } else {
                const_cast<QMimeData *>(event->mimeData())->setProperty("DirectSaveUrl", info->urlOf(UrlInfoType::kParentUrl));
                fmDebug() << "Direct save URL set to parent directory:" << info->urlOf(UrlInfoType::kParentUrl).toString();
            }
        }

        event->accept();
    } else {
        QModelIndex hoverIndex = view->indexAt(event->pos());

        if (event->source() == view && dragFileFromCurrent && (!hoverIndex.isValid() || view->isSelected(hoverIndex)) && !WindowUtils::keyCtrlIsPressed()) {
            fmDebug() << "Drop canceled - dragging from current view to selected item";
            return true;
        }

        if (!hoverIndex.isValid()) {
            fmDebug() << "No valid hover index - using root index";
            hoverIndex = view->rootIndex();
        } else {
            FileInfoPointer fileInfo = view->model()->fileInfo(hoverIndex);
            if (fileInfo.isNull()) {
                fmDebug() << "File info is null - creating file info";
                hoverIndex.data(Global::ItemRoles::kItemCreateFileInfoRole);
            }
            fileInfo = view->model()->fileInfo(hoverIndex);

            if (fileInfo) {
                bool isDrop = dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_IsDrop", fileInfo->urlOf(UrlInfoType::kUrl));
                // NOTE: if item can not drop, the drag item will drop to root dir.
                if (fileInfo->isAttributes(OptInfoType::kIsFile)
                    && !FileUtils::isDesktopFileSuffix(fileInfo->urlOf(UrlInfoType::kUrl))
                    && !isDrop) {
                    fmDebug() << "File cannot accept drop - redirecting to root directory";
                    hoverIndex = view->rootIndex();
                }
            }
        }

        if (!hoverIndex.isValid()) {
            fmDebug() << "No valid hover index after processing";
            return true;
        }

        QUrl toUrl = view->model()->data(hoverIndex, DFMGLOBAL_NAMESPACE::ItemRoles::kItemUrlRole).toUrl();
        if (dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileDrop", currentDragSourceUrls, toUrl)) {
            fmDebug() << "Drop handled by hook";
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
                fmInfo() << "Drop operation completed successfully";
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
                    fmDebug() << "Wine application drop handled with copy action";
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

bool DragDropHelper::checkDragEnable(const QUrl &dragUrl, const QUrl &targetUrl) const
{
    auto info = InfoFactory::create<FileInfo>(dragUrl);
    if (!info)
        return false;

    // Check for standard move/copy/rename capabilities.
    // BUG-350297
    if (!info->exists()
        || info->canAttributes(CanableInfoType::kCanMoveOrCopy)
        || info->canAttributes(CanableInfoType::kCanRename))
        return true;

    // Some desktop files may allow trash even if not movable/renamable.
    bool dragToDelete = (FileUtils::isTrashFile(targetUrl) || FileUtils::isTrashDesktopFile(targetUrl)) && info->canAttributes(CanableInfoType::kCanTrash);

    return dragToDelete;
}

bool DragDropHelper::checkMoveEnable(const QUrl &dragUrl, const QUrl &toUrl) const
{
    if (dpfHookSequence->run("dfmplugin_workspace", "hook_DragDrop_FileCanMove", dragUrl))
        return true;

    // some desktopfile can not rename (depend AM), and some can not move or copy (computer etc..)
    // but they all allow to be deleted to trash
    FileInfoPointer info = InfoFactory::create<FileInfo>(dragUrl);
    if (FileUtils::isDesktopFile(info->urlOf(UrlInfoType::kUrl))) {
        return info->canAttributes(CanableInfoType::kCanMoveOrCopy) || (FileUtils::isTrashFile(toUrl) || FileUtils::isTrashDesktopFile(toUrl));
    }
    return !info->exists() || info->canAttributes(CanableInfoType::kCanRename);
}
