/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#include "shortcuthelper.h"
#include "fileoperaterhelper.h"
#include "models/fileviewmodel.h"
#include "models/filesortfilterproxymodel.h"
#include "views/baseitemdelegate.h"
#include "views/fileview.h"
#include "utils/workspacehelper.h"
#include "events/workspaceeventcaller.h"

#include "dfm-base/base/schemefactory.h"

#include <QAction>
#include <QDebug>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

static constexpr char kViewShortcutKey[] = "_view_shortcut_key";

ShortcutHelper::ShortcutHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
    connect(view, &FileView::reqOpenAction, this, &ShortcutHelper::openAction);
    registerShortcut();
}

void ShortcutHelper::registerShortcut()
{
    registerAction(QKeySequence::Copy, false);
    registerAction(QKeySequence::Cut, false);
    registerAction(QKeySequence::Paste);
    registerAction(QKeySequence::Undo);
}

void ShortcutHelper::registerAction(QKeySequence::StandardKey shortcut, bool autoRepeat)
{
    QAction *action = new QAction(parent());
    action->setAutoRepeat(autoRepeat);
    action->setShortcut(shortcut);
    action->setProperty(kViewShortcutKey, shortcut);
    view->addAction(action);
    connect(action, &QAction::triggered, this, &ShortcutHelper::acitonTriggered);
}

bool ShortcutHelper::normalKeyPressEventHandle(const QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter: {
        // Todo(yanghao):editingIndex handle
        const auto mode = view->currentDirOpenMode();
        const auto &urls = view->selectedUrlList();
        openAction(urls, mode);
        return true;
    }
    case Qt::Key_Backspace: {
        cdUp();
        return true;
    }
    case Qt::Key_Delete: {
        moveToTrash();
        break;
    }
    case Qt::Key_End: {
        const auto &urls = view->selectedUrlList();
        if (urls.isEmpty()) {
            int rowCount = view->proxyModel()->rowCount(view->rootIndex());
            auto index = view->proxyModel()->index(rowCount - 1, 0);
            view->setCurrentIndex(index);
            return true;
        }
        break;
    }
    default:
        break;
    }

    return false;
}

bool ShortcutHelper::processKeyPressEvent(QKeyEvent *event)
{
    if (!event || !view)
        return false;
    switch (event->modifiers()) {
    case Qt::NoModifier:
        if (event->key() == Qt::Key_Space) {
            previewFiles();
            return true;
        } else if (normalKeyPressEventHandle(event)) {
            return true;
        }
        break;
    case Qt::KeypadModifier:
        if (normalKeyPressEventHandle(event)) {
            return true;
        }
        break;
    case Qt::ControlModifier:
        switch (event->key()) {
        case Qt::Key_H: {
            toggleHiddenFiles();
            return true;
        }
        case Qt::Key_I: {
            showFilesProperty();
            return true;
        }
        case Qt::Key_Up: {
            cdUp();
            return true;
        }
        case Qt::Key_Down: {
            openAction(view->selectedUrlList());
            return true;
        }
        default:
            break;
        }
        break;
    case Qt::ShiftModifier:
        if (event->key() == Qt::Key_Delete) {
            deleteFiles();
            return true;
        } else if (event->key() == Qt::Key_T) {
            openInTerminal();
            return true;
        }
        break;
    case Qt::ControlModifier | Qt::ShiftModifier:
        if (event->key() == Qt::Key_N) {
            touchFolder();
            return true;
        }
        break;
    case Qt::AltModifier:
    case Qt::AltModifier | Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Up:
            cdUp();
            return true;
        case Qt::Key_Down: {
            showFilesProperty();
            return true;
        }
        case Qt::Key_Home:
            openAction(view->selectedUrlList());
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

void ShortcutHelper::acitonTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;

    auto key = action->property(kViewShortcutKey).value<QKeySequence::StandardKey>();
    switch (key) {
    case QKeySequence::Copy:
        copyFiles();
        break;
    case QKeySequence::Cut:
        cutFiles();
        break;
    case QKeySequence::Paste:
        pasteFiles();
        break;
    case QKeySequence::Undo:
        undoFiles();
        break;
    default:
        break;
    }
}

void ShortcutHelper::copyFiles()
{
    FileOperaterHelperIns->copyFiles(view);
}

void ShortcutHelper::cutFiles()
{
    FileOperaterHelperIns->cutFiles(view);
}

void ShortcutHelper::pasteFiles()
{
    FileOperaterHelperIns->pasteFiles(view);
}

void ShortcutHelper::undoFiles()
{
    FileOperaterHelperIns->undoFiles(view);
}

void ShortcutHelper::deleteFiles()
{
    if (view->selectedUrlList().isEmpty())
        return;
    // Todo(yanghao):only support trash on root url
    // 共享文件夹不用弹出彻底删除对话框
    // 网络邻居目录不用弹出彻底删除对话框
    // 最近使用目录不用弹出彻底删除对话框
    FileOperaterHelperIns->deleteFiles(view);
}

void ShortcutHelper::moveToTrash()
{
    // Todo(yanghao): QUrl to LocalFile
    // complete deletion eg: gvfs, vault
    // only support trash on root url
    const QList<QUrl> &urls = view->selectedUrlList();
    if (!urls.isEmpty()) {
        FileOperaterHelperIns->moveToTrash(view);
    }
}

void ShortcutHelper::touchFolder()
{
    // Todo(yanghao):editingIndex handle
    view->clearSelection();
    FileOperaterHelperIns->touchFolder(view);
}

void ShortcutHelper::toggleHiddenFiles()
{
    //Todo(yanghao)
    // Todo(yanghao): preSelectionUrls
    // d->preSelectionUrls = urls;
    view->itemDelegate()->hideAllIIndexWidget();
    view->clearSelection();
    view->proxyModel()->toggleHiddenFiles();
}

void ShortcutHelper::showFilesProperty()
{
    // ToDo(yanghao): network not support
    FileOperaterHelperIns->showFilesProperty(view);
}

void ShortcutHelper::previewFiles()
{
    // Todo(yanghao): sorted urls
    FileOperaterHelperIns->previewFiles(view);
}

void ShortcutHelper::openAction(const QList<QUrl> &urls, const DirOpenMode openMode)
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);

    for (const QUrl &url : urls) {
        const AbstractFileInfoPointer &fileInfoPtr = InfoFactory::create<AbstractFileInfo>(url);
        if (fileInfoPtr && fileInfoPtr->isDir()) {
            if (openMode == DirOpenMode::kOpenNewWindow) {
                WorkspaceEventCaller::sendOpenWindow({ url });
            } else {
                WorkspaceEventCaller::sendChangeCurrentUrl(windowId, url);
            }
        } else {
            FileOperaterHelperIns->openFiles(view, { url });
        }
    }
}

void ShortcutHelper::openInTerminal()
{
    FileOperaterHelperIns->openInTerminal(view);
}

void ShortcutHelper::cdUp()
{
    view->cdUp();
}
