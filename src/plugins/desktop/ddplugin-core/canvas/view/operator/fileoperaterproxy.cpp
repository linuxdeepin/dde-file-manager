/*
 * Copyright (C) 2022 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "fileoperaterproxy.h"
#include "canvas/view/canvasview.h"
#include "canvas/view/canvasmodel.h"
#include "canvas/view/canvasselectionmodel.h"
#include "dfm-framework/framework.h"
#include "dfm-base/dfm_global_defines.h"
#include "dfm-base/dfm_event_defines.h"
#include "dfm-base/utils/clipboard.h"

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_D_USE_NAMESPACE

class FileBusinessGlobal : public FileOperaterProxy {};
Q_GLOBAL_STATIC(FileBusinessGlobal, fileBusinessGlobal)

FileOperaterProxy::FileOperaterProxy(QObject *parent)
    : QObject (parent)
{

}

FileOperaterProxy *FileOperaterProxy::instance()
{
    return fileBusinessGlobal;
}

void FileOperaterProxy::touchFile(const CanvasView *view, const dfmbase::Global::CreateFileType type)
{
    // todo(wangcl):callbak
    emit createFileByMenu(view->screenNum(), view->lastMenuPos());

    dpfInstance.eventDispatcher().publish(GlobalEventType::kTouchFile
                                          , view->winId()
                                          , view->model()->rootUrl()
                                          , type);
}

void FileOperaterProxy::touchFolder(const CanvasView *view)
{
    // todo(wangcl):callbak
    emit createFileByMenu(view->screenNum(), view->lastMenuPos());

    dpfInstance.eventDispatcher().publish(GlobalEventType::kMkdir
                                          , view->winId()
                                          , view->model()->rootUrl()
                                          , kCreateFileTypeFolder);
}

void FileOperaterProxy::copyFiles(const CanvasView *view)
{
    dpfInstance.eventDispatcher().publish(GlobalEventType::kWriteUrlsToClipboard
                                          , view->winId()
                                          , ClipBoard::ClipboardAction::kCopyAction
                                          , view->selectionModel()->selectedUrls());
}

void FileOperaterProxy::cutFiles(const CanvasView *view)
{
    dpfInstance.eventDispatcher().publish(GlobalEventType::kWriteUrlsToClipboard
                                          , view->winId()
                                          , ClipBoard::ClipboardAction::kCutAction
                                          , view->selectionModel()->selectedUrls());
}

void FileOperaterProxy::pasteFiles(const CanvasView *view)
{
    auto urls = ClipBoard::instance()->clipboardFileUrlList();
    ClipBoard::ClipboardAction action = ClipBoard::instance()->clipboardAction();
    if (ClipBoard::kCopyAction == action) {
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCopy
                                              , view->winId()
                                              , urls
                                              , view->model()->rootUrl()
                                              , AbstractJobHandler::JobFlag::kNoHint);
    } else if (ClipBoard::kCutAction == action) {
        dpfInstance.eventDispatcher().publish(GlobalEventType::kCutFile
                                              , view->winId()
                                              , urls
                                              , view->model()->rootUrl()
                                              , AbstractJobHandler::JobFlag::kNoHint);
    } else {
        qWarning() << "clipboard action:" << action << "    urls:" << urls;
    }
}

void FileOperaterProxy::openFiles(const CanvasView *view)
{
    dpfInstance.eventDispatcher().publish(GlobalEventType::kOpenFiles
                                          , view->winId()
                                          , view->selectionModel()->selectedUrls());
}

void FileOperaterProxy::renameFiles(const CanvasView *view)
{
    Q_UNUSED(view)
    // todo(wangcl)
}

void FileOperaterProxy::openFilesByApp(const CanvasView *view)
{
    Q_UNUSED(view)
    // todo(wangcl)
}

void FileOperaterProxy::moveToTrash(const CanvasView *view)
{
    dpfInstance.eventDispatcher().publish(GlobalEventType::kMoveToTrash
                                          , view->winId()
                                          , view->selectionModel()->selectedUrls()
                                          , AbstractJobHandler::JobFlag::kNoHint);
}

void FileOperaterProxy::deleteFiles(const CanvasView *view)
{
    dpfInstance.eventDispatcher().publish(GlobalEventType::kDeleteFiles
                                          , view->winId()
                                          , view->selectionModel()->selectedUrls()
                                          , AbstractJobHandler::JobFlag::kNoHint);
}


