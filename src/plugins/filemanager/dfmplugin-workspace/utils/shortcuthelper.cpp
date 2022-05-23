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
#include "fileoperatorhelper.h"
#include "models/fileviewmodel.h"
#include "models/filesortfilterproxymodel.h"
#include "views/baseitemdelegate.h"
#include "views/fileview.h"
#include "utils/workspacehelper.h"
#include "events/workspaceeventcaller.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/utils/fileutils.h"

#include "services/common/delegate/delegateservice.h"

#include <dfm-framework/dpf.h>
#include <dfm-io/dfmio_utils.h>

#include <QAction>
#include <QDebug>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

static constexpr char kViewShortcutKey[] = "_view_shortcut_key";
static constexpr char kCurrentEventSpace[] = { DPF_MACRO_TO_STR(DPWORKSPACE_NAMESPACE) };

ShortcutHelper::ShortcutHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
    registerShortcut();
}

void ShortcutHelper::registerShortcut()
{
    registerAction(QKeySequence::Copy, false);
    registerAction(QKeySequence::Cut, false);
    registerAction(QKeySequence::Paste, false);
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
        const auto &urls = view->selectedUrlList();
        int dirCount = 0;
        for (const QUrl &url : urls) {
            AbstractFileInfoPointer info = InfoFactory::create<AbstractFileInfo>(url);
            if (info->isDir())
                ++dirCount;
            if (dirCount > 1)
                break;
        }

        const auto mode = dirCount > 1 ? DirOpenMode::kOpenNewWindow : view->currentDirOpenMode();
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
            int rowCount = view->model()->rowCount(view->rootIndex());
            auto index = view->model()->index(rowCount - 1, 0);
            view->setCurrentIndex(index);
            return true;
        }
        break;
    }
    case Qt::Key_Escape:
        ClipBoard::clearClipboard();
        return true;
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
        if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
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
    FileOperatorHelperIns->copyFiles(view);
}

void ShortcutHelper::cutFiles()
{
    FileOperatorHelperIns->cutFiles(view);
}

void ShortcutHelper::pasteFiles()
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_PasteFilesShortcut", windowId, view->rootUrl()))
        return;

    FileOperatorHelperIns->pasteFiles(view);
}

void ShortcutHelper::undoFiles()
{
    FileOperatorHelperIns->undoFiles(view);
}

void ShortcutHelper::deleteFiles()
{
    if (view->selectedUrlList().isEmpty())
        return;

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_DeleteFilesShortcut", windowId, view->selectedUrlList()))
        return;

    // Todo(yanghao):only support trash on root url
    // 共享文件夹不用弹出彻底删除对话框
    // 网络邻居目录不用弹出彻底删除对话框
    // 最近使用目录不用弹出彻底删除对话框
    FileOperatorHelperIns->deleteFiles(view);
}

void ShortcutHelper::moveToTrash()
{
    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_MoveToTrashShortcut", windowId, view->selectedUrlList()))
        return;
    // Todo(lanxs): QUrl to LocalFile
    // complete deletion eg: gvfs, vault
    // only support trash on root url
    const QList<QUrl> &urls = view->selectedUrlList();

    if (!urls.isEmpty())
        FileOperatorHelperIns->moveToTrash(view);
}

void ShortcutHelper::touchFolder()
{
    // Todo(yanghao):editingIndex handle
    view->clearSelection();
    FileOperatorHelperIns->touchFolder(view);
}

void ShortcutHelper::toggleHiddenFiles()
{
    //Todo(yanghao)
    // Todo(yanghao): preSelectionUrls
    // d->preSelectionUrls = urls;
    view->itemDelegate()->hideAllIIndexWidget();
    view->clearSelection();
    view->model()->toggleHiddenFiles();
}

void ShortcutHelper::showFilesProperty()
{
    // ToDo(yanghao): network not support
    FileOperatorHelperIns->showFilesProperty(view);
}

void ShortcutHelper::previewFiles()
{
    QList<QUrl> Urls = view->selectedUrlList();
    QList<QUrl> selectUrls;
    for (QUrl &url : Urls) {
        selectUrls.append(delegateServIns->urlTransform(url));
    }
    Urls = view->model()->getCurrentDirFileUrls();
    QList<QUrl> currentDirUrls;
    for (QUrl &url : Urls) {
        currentDirUrls.append(delegateServIns->urlTransform(url));
    }
    FileOperatorHelperIns->previewFiles(view, selectUrls, currentDirUrls);
}

void ShortcutHelper::openAction(const QList<QUrl> &urls, const DirOpenMode openMode)
{
    FileOperatorHelperIns->openFilesByMode(view, urls, openMode);
}

void ShortcutHelper::openInTerminal()
{
    FileOperatorHelperIns->openInTerminal(view);
}

void ShortcutHelper::cdUp()
{
    view->cdUp();
}
