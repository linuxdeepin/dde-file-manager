/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhangsheng<zhangsheng@uniontech.com>
 *             liuyangming<liuyangming@uniontech.com>
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
#include "fileoperatorhelper.h"
#include "workspacehelper.h"
#include "events/workspaceeventcaller.h"
#include "views/fileview.h"
#include "models/filesortfilterproxymodel.h"
#include "dfm-framework/framework.h"

#include "services/common/propertydialog/property_defines.h"
#include "services/common/preview/preview_defines.h"
#include "services/common/bluetooth/bluetooth_defines.h"
#include "services/common/delegate/delegateservice.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/schemefactory.h"

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE
FileOperatorHelper *FileOperatorHelper::instance()
{
    static FileOperatorHelper helper;
    return &helper;
}

void FileOperatorHelper::touchFolder(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(GlobalEventType::kMkdir,
                                          windowId,
                                          view->rootUrl(),
                                          kCreateFileTypeFolder);
}

void FileOperatorHelper::touchFiles(const FileView *view, const CreateFileType type, QString suffix)
{
    const quint64 windowId = WorkspaceHelper::instance()->windowId(view);
    const QUrl &url = view->rootUrl();

    dpfInstance.eventDispatcher().publish(GlobalEventType::kTouchFile,
                                          windowId,
                                          url,
                                          type,
                                          suffix);
}

void FileOperatorHelper::openFiles(const FileView *view)
{
    openFiles(view, view->selectedUrlList());
}

void FileOperatorHelper::openFiles(const FileView *view, const QList<QUrl> &urls)
{
    DirOpenMode openMode = view->currentDirOpenMode();

    openFilesByMode(view, urls, openMode);
}

void FileOperatorHelper::openFilesByMode(const FileView *view, const QList<QUrl> &urls, const DirOpenMode mode)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);

    for (const QUrl &url : urls) {
        const AbstractFileInfoPointer &fileInfoPtr = InfoFactory::create<AbstractFileInfo>(url);
        if (fileInfoPtr && fileInfoPtr->isDir()) {
            if (mode == DirOpenMode::kOpenNewWindow) {
                WorkspaceEventCaller::sendOpenWindow({ url });
            } else {
                WorkspaceEventCaller::sendChangeCurrentUrl(windowId, url);
            }
        } else {
            const QList<QUrl> &openUrls = { url };
            dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenFiles,
                                                  windowId,
                                                  openUrls);
        }
    }
}

void FileOperatorHelper::openFilesByApp(const FileView *view)
{
    // Todo(yanghao)
}

void FileOperatorHelper::renameFile(const FileView *view, const QUrl &oldUrl, const QUrl &newUrl)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(GlobalEventType::kRenameFile,
                                          windowId,
                                          oldUrl,
                                          newUrl);
}

void FileOperatorHelper::copyFiles(const FileView *view)
{

    QList<QUrl> selectedUrls = view->selectedUrlList();
    if (selectedUrls.size() == 1) {
        const AbstractFileInfoPointer &fileInfo = InfoFactory::create<AbstractFileInfo>(selectedUrls.first());
        if (!fileInfo || !fileInfo->isReadable())
            return;
    }
    qInfo() << "copy shortcut key to clipboard, selected urls: " << selectedUrls
            << " currentUrl: " << view->rootUrl();
    auto windowId = WorkspaceHelper::instance()->windowId(view);

    dpfInstance.eventDispatcher().publish(GlobalEventType::kWriteUrlsToClipboard,
                                          windowId,
                                          ClipBoard::ClipboardAction::kCopyAction,
                                          selectedUrls);
}

void FileOperatorHelper::cutFiles(const FileView *view)
{
    qInfo() << "cut shortcut key to clipboard";
    const AbstractFileInfoPointer &fileInfo = InfoFactory::create<AbstractFileInfo>(view->rootUrl());
    if (!fileInfo || !fileInfo->isWritable())
        return;
    QList<QUrl> selectedUrls = view->selectedUrlList();
    qInfo() << "selected urls: " << selectedUrls
            << " currentUrl: " << view->rootUrl();
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(GlobalEventType::kWriteUrlsToClipboard,
                                          windowId,
                                          ClipBoard::ClipboardAction::kCutAction,
                                          selectedUrls);
}

void FileOperatorHelper::pasteFiles(const FileView *view)
{
    qInfo() << " paste file by clipboard and currentUrl: " << view->rootUrl();
    auto action = ClipBoard::instance()->clipboardAction();
    auto sourceUrls = ClipBoard::instance()->clipboardFileUrlList();
    auto windowId = WorkspaceHelper::instance()->windowId(view);

    if (ClipBoard::kCopyAction == action) {
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCopy,
                                              windowId,
                                              sourceUrls,
                                              view->rootUrl(),
                                              AbstractJobHandler::JobFlag::kNoHint, nullptr);
    } else if (ClipBoard::kCutAction == action) {

        if (ClipBoard::supportCut()) {
            ClipBoard::clearClipboard();
            dpfInstance.eventDispatcher().publish(GlobalEventType::kCutFile,
                                                  windowId,
                                                  sourceUrls,
                                                  view->rootUrl(),
                                                  AbstractJobHandler::JobFlag::kNoHint, nullptr);
        }
    } else {
        qWarning() << "clipboard action:" << action << "    urls:" << sourceUrls;
    }
}

void FileOperatorHelper::undoFiles(const FileView *view)
{
    qInfo() << " undoFiles file, currentUrl: " << view->rootUrl();
    auto windowId = WorkspaceHelper::instance()->windowId(view);

    dpfInstance.eventDispatcher().publish(GlobalEventType::kRevocation,
                                          windowId);
}

void FileOperatorHelper::moveToTrash(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(GlobalEventType::kMoveToTrash,
                                          windowId,
                                          view->selectedUrlList(),
                                          AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperatorHelper::deleteFiles(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(GlobalEventType::kDeleteFiles,
                                          windowId,
                                          view->selectedUrlList(),
                                          AbstractJobHandler::JobFlag::kNoHint, nullptr);
}

void FileOperatorHelper::createSymlink(const FileView *view, QUrl targetParent)
{
    if (targetParent.isEmpty())
        targetParent = view->rootUrl();

    auto windowId = WorkspaceHelper::instance()->windowId(view);

    for (const QUrl &fileUrl : view->selectedUrlList()) {
        QString linkName = FileUtils::getSymlinkFileName(fileUrl);
        QUrl linkUrl;
        linkUrl.setScheme(targetParent.scheme());
        linkUrl.setPath(targetParent.path() + "/" + linkName);

        dpfInstance.eventDispatcher().publish(GlobalEventType::kCreateSymlink,
                                              windowId,
                                              fileUrl,
                                              linkUrl,
                                              AbstractJobHandler::JobFlag::kNoHint);
    }
}

void FileOperatorHelper::openInTerminal(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    QList<QUrl> urls = view->selectedUrlList();
    if (urls.isEmpty())
        urls.append(view->rootUrl());
    dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenInTerminal,
                                          windowId,
                                          urls);
}

void FileOperatorHelper::showFilesProperty(const FileView *view)
{
    QList<QUrl> urls = view->selectedUrlList();
    if (urls.isEmpty())
        urls.append(view->rootUrl());
    dpfInstance.eventDispatcher().publish(DSC_NAMESPACE::Property::EventType::kEvokePropertyDialog,
                                          urls);
}

void FileOperatorHelper::sendBluetoothFiles(const FileView *view)
{
    QList<QUrl> urls = view->selectedUrlList();
    if (!urls.isEmpty())
        dpfInstance.eventDispatcher().publish(DSC_NAMESPACE::EventType::kSendFiles, urls);
}

void FileOperatorHelper::previewFiles(const FileView *view, const QList<QUrl> &selectUrls, const QList<QUrl> &currentDirUrls)
{
    quint64 winID = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(DSC_NAMESPACE::Preview::EventType::kShowPreviewEvent, winID, selectUrls, currentDirUrls);
}

void FileOperatorHelper::dropFiles(const FileView *view, const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (action == Qt::MoveAction) {
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCutFile,
                                              windowId,
                                              urls,
                                              targetUrl,
                                              AbstractJobHandler::JobFlag::kNoHint, nullptr);
    } else {
        // default is copy file
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCopy,
                                              windowId,
                                              urls,
                                              targetUrl,
                                              AbstractJobHandler::JobFlag::kNoHint, nullptr);
    }
}

FileOperatorHelper::FileOperatorHelper(QObject *parent)
    : QObject(parent)
{
}
