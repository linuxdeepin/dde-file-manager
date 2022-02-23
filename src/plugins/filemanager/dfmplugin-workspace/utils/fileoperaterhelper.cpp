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
#include "fileoperaterhelper.h"
#include "workspacehelper.h"
#include "views/fileview.h"
#include "dfm-framework/framework.h"
#include "dfm_global_defines.h"

#include "services/common/propertydialog/property_defines.h"

#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/base/schemefactory.h"

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE
FileOperaterHelper *FileOperaterHelper::instance()
{
    static FileOperaterHelper helper;
    return &helper;
}

void FileOperaterHelper::touchFolder(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(GlobalEventType::kMkdir,
                                          windowId,
                                          view->rootUrl(),
                                          kCreateFileTypeFolder);
}

void FileOperaterHelper::openFiles(const FileView *view)
{
    openFiles(view, view->selectedUrlList());
}

void FileOperaterHelper::openFiles(const FileView *view, const QList<QUrl> &urls)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenFiles,
                                          windowId,
                                          urls);
}

void FileOperaterHelper::openFilesByApp(const FileView *view)
{
    // Todo(yanghao)
}

void FileOperaterHelper::renameFile(const FileView *view, const QUrl &oldUrl, const QUrl &newUrl)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(GlobalEventType::kRenameFile,
                                          windowId,
                                          oldUrl,
                                          newUrl);
}

void FileOperaterHelper::copyFiles(const FileView *view)
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

void FileOperaterHelper::cutFiles(const FileView *view)
{
    // Todo(yanghao): 只支持回收站根目录下的文件执行剪切
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

void FileOperaterHelper::pasteFiles(const FileView *view)
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
                                              AbstractJobHandler::JobFlag::kNoHint);
    } else if (ClipBoard::kCutAction == action) {
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCutFile,
                                              windowId,
                                              sourceUrls,
                                              view->rootUrl(),
                                              AbstractJobHandler::JobFlag::kNoHint);
    } else {
        qWarning() << "clipboard action:" << action << "    urls:" << sourceUrls;
    }
}

void FileOperaterHelper::moveToTrash(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(GlobalEventType::kMoveToTrash,
                                          windowId,
                                          view->selectedUrlList(),
                                          AbstractJobHandler::JobFlag::kNoHint);
}

void FileOperaterHelper::deleteFiles(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(GlobalEventType::kDeleteFiles,
                                          windowId,
                                          view->selectedUrlList(),
                                          AbstractJobHandler::JobFlag::kNoHint);
}

void FileOperaterHelper::openInTerminal(const FileView *view)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenInTerminal,
                                          windowId,
                                          view->selectedUrlList());
}

void FileOperaterHelper::showFilesProperty(const FileView *view)
{
    dpfInstance.eventDispatcher().publish(DSC_NAMESPACE::PropertyEventType::kEvokeDefaultFileProperty,
                                          view->selectedUrlList());
}

void FileOperaterHelper::previewFiles(const FileView *view)
{
    //Todo(yanghao):
}

void FileOperaterHelper::dropFiles(const FileView *view, const Qt::DropAction &action, const QUrl &targetUrl, const QList<QUrl> &urls)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (action == Qt::MoveAction) {
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCutFile,
                                              windowId,
                                              urls,
                                              targetUrl,
                                              AbstractJobHandler::JobFlag::kNoHint);
    } else {
        // default is copy file
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCopy,
                                              windowId,
                                              urls,
                                              targetUrl,
                                              AbstractJobHandler::JobFlag::kNoHint);
    }
}

FileOperaterHelper::FileOperaterHelper(QObject *parent)
    : QObject(parent)
{
}
