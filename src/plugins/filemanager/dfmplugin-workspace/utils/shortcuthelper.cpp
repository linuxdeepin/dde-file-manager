// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcuthelper.h"
#include "fileoperatorhelper.h"
#include "models/fileviewmodel.h"
#include "views/baseitemdelegate.h"
#include "views/fileview.h"
#include "utils/workspacehelper.h"
#include "events/workspaceeventcaller.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/base/standardpaths.h>

#include <dfm-framework/dpf.h>
#include <dfm-io/dfmio_utils.h>

#include <QAction>
#include <QDebug>

Q_DECLARE_METATYPE(QList<QUrl> *)

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

static constexpr char kViewShortcutKey[] = "_view_shortcut_key";
static constexpr char kCurrentEventSpace[] = { DPF_MACRO_TO_STR(DPWORKSPACE_NAMESPACE) };

ShortcutHelper::ShortcutHelper(FileView *parent)
    : QObject(parent),
      view(parent)
{
    fmDebug() << "ShortcutHelper created for FileView";

    registerShortcut();
    initRenameProcessTimer();

    fmDebug() << "ShortcutHelper initialization completed";
}

void ShortcutHelper::registerShortcut()
{
    fmDebug() << "Registering standard shortcuts";

    registerAction(QKeySequence::Copy, false);
    registerAction(QKeySequence::Cut, false);
    registerAction(QKeySequence::Paste, false);
    registerAction(QKeySequence::Undo);

    fmDebug() << "Standard shortcuts registration completed";
}

void ShortcutHelper::registerAction(QKeySequence::StandardKey shortcut, bool autoRepeat)
{
    QAction *action = new QAction(parent());
    action->setAutoRepeat(autoRepeat);
    action->setShortcut(shortcut);
    action->setProperty(kViewShortcutKey, shortcut);
    view->addAction(action);
    connect(action, &QAction::triggered, this, &ShortcutHelper::acitonTriggered);
    fmDebug() << "Registered shortcut action - key:" << static_cast<int>(shortcut) << "autoRepeat:" << autoRepeat;
}

bool ShortcutHelper::normalKeyPressEventHandle(const QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter: {
        if (renameProcessTimer->isActive()) {
            fmDebug() << "Rename process timer active, setting enter trigger flag";
            enterTriggerFlag = true;
            return false;
        }

        return doEnterPressed();
    }
    case Qt::Key_Backspace: {
        fmDebug() << "Backspace key pressed - navigating up";
        cdUp();
        return true;
    }
    case Qt::Key_Delete: {
        fmDebug() << "Delete key pressed - moving to trash";
        moveToTrash();
        break;
    }
    case Qt::Key_End: {
        fmDebug() << "End key pressed - navigating to last item";
        const auto &urls = view->selectedUrlList();
        if (urls.isEmpty()) {
            int rowCount = view->model()->rowCount(view->rootIndex());
            auto index = view->model()->index(rowCount - 1, 0, view->rootIndex());
            view->setCurrentIndex(index);
            return true;
        }
        break;
    }
    case Qt::Key_Escape:
        fmDebug() << "Escape key pressed - clearing clipboard";
        ClipBoard::clearClipboard();
        return true;
    case Qt::Key_F2: {
        fmDebug() << "F2 key pressed - refreshing selected files";
        const auto &urls = view->selectedUrlList();
        for (const QUrl &url : urls) {
            FileInfoPointer info = InfoFactory::create<FileInfo>(url);
            info->refresh();
        }
        break;
    }
    default:
        break;
    }

    return false;
}

bool ShortcutHelper::doEnterPressed()
{
    const auto &urls = view->selectedUrlList();
    quint64 winId = WorkspaceHelper::instance()->windowId(view);
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_ShortCut_EnterPressed", winId, urls)) {
        fmDebug() << "Enter pressed handled by hook";
        return true;
    }

    int dirCount = 0;
    for (const QUrl &url : urls) {
        FileInfoPointer info = InfoFactory::create<FileInfo>(url);
        if (info->isAttributes(OptInfoType::kIsDir))
            ++dirCount;
        if (dirCount > 1)
            break;
    }

    fmDebug() << "Opening files - count:" << urls.size() << "directories:" << dirCount;
    openAction(urls, view->currentDirOpenMode());
    return true;
}

void ShortcutHelper::initRenameProcessTimer()
{
    renameProcessTimer = new QTimer(this);
    renameProcessTimer->setSingleShot(true);
    renameProcessTimer->setInterval(500);

    connect(renameProcessTimer, &QTimer::timeout, this, [=] {
        if (enterTriggerFlag) {
            enterTriggerFlag = false;
            fmDebug() << "Rename process timer timeout - executing enter action";
            doEnterPressed();
        }
    });

    fmDebug() << "Rename process timer initialized with 500ms interval";
}

bool ShortcutHelper::processKeyPressEvent(QKeyEvent *event)
{
    if (!event || !view) {
        fmWarning() << "Invalid event or view in key press processing";
        return false;
    }
    switch (event->modifiers()) {
    case Qt::NoModifier:
        if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
            fmDebug() << "Space key pressed - preview files";
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
            fmDebug() << "Ctrl+H pressed - toggle hidden files";
            toggleHiddenFiles();
            return true;
        }
        case Qt::Key_I: {
            fmDebug() << "Ctrl+I pressed - show files property";
            showFilesProperty();
            return true;
        }
        case Qt::Key_Up: {
            fmDebug() << "Ctrl+Up pressed - cd up";
            cdUp();
            return true;
        }
        case Qt::Key_Down: {
            fmDebug() << "Ctrl+Down pressed - open selected";
            openAction(view->selectedUrlList());
            return true;
        }
        case Qt::Key_D: {
            fmDebug() << "Ctrl+D pressed - move to trash";
            moveToTrash();
            return true;
        }
        case Qt::Key_Y: {
            fmDebug() << "Ctrl+Y pressed - redo files";
            // redo
            redoFiles();
            return true;
        }
        default:
            break;
        }
        break;
    case Qt::ShiftModifier:
        if (event->key() == Qt::Key_Delete) {
            fmDebug() << "Shift+Delete pressed - delete files permanently";
            deleteFiles();
            return true;
        } else if (event->key() == Qt::Key_T) {
            fmDebug() << "Shift+T pressed - open in terminal";
            openInTerminal();
            return true;
        }
        break;
    case Qt::ControlModifier | Qt::ShiftModifier:
        switch (event->key()) {
        case Qt::Key_N:
            fmDebug() << "Ctrl+Shift+N pressed - create new folder";
            touchFolder();
            return true;
        case Qt::Key_I:
            fmDebug() << "Ctrl+Shift+I pressed - reverse selection";
            return reverseSelect();
        case Qt::Key_C:
            copyFilePath();
            return true;
        }
        break;
    case Qt::AltModifier:
    case Qt::AltModifier | Qt::KeypadModifier:
        switch (event->key()) {
        case Qt::Key_Up:
            fmDebug() << "Alt+Up pressed - cd up";
            cdUp();
            return true;
        case Qt::Key_Down: {
            fmDebug() << "Alt+Down pressed - open selected";
            openAction(view->selectedUrlList());
            return true;
        }
        case Qt::Key_Home:
            fmDebug() << "Alt+Home pressed - go to home directory";
            openAction({ QUrl::fromLocalFile(StandardPaths::location(StandardPaths::kHomePath)) });
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
    if (!action) {
        fmWarning() << "Action triggered with invalid sender";
        return;
    }

    auto key = action->property(kViewShortcutKey).value<QKeySequence::StandardKey>();
    fmDebug() << "Standard action triggered - key:" << static_cast<int>(key);

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
    const QList<QUrl> &selectUrls = view->selectedUrlList();
    if (selectUrls.isEmpty()) {
        fmDebug() << "Copy operation canceled - no files selected";
        return;
    }

    fmInfo() << "Copy files operation - count:" << selectUrls.size();

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_ShortCut_CopyFiles", windowId, selectUrls, view->rootUrl())) {
        fmDebug() << "Copy files handled by hook";
        return;
    }
    FileOperatorHelperIns->copyFiles(view);
}

void ShortcutHelper::copyFilePath()
{
    const QList<QUrl> &selectUrls = view->selectedUrlList();
    if (selectUrls.isEmpty()) {
        fmDebug() << "Copy file path operation canceled - no files selected";
        return;
    }

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_ShortCut_CopyFilePath", windowId, selectUrls, view->rootUrl())) {
        fmDebug() << "Copy file path handled by hook";
        return;
    }
    FileOperatorHelperIns->copyFilePath(view);
}

void ShortcutHelper::cutFiles()
{
    const QList<QUrl> &selectUrls = view->selectedUrlList();
    if (selectUrls.isEmpty()) {
        fmDebug() << "Cut operation canceled - no files selected";
        return;
    }

    fmInfo() << "Cut files operation - count:" << selectUrls.size();

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_ShortCut_CutFiles", windowId, selectUrls, view->rootUrl())) {
        fmDebug() << "Cut files handled by hook";
        return;
    }
    FileOperatorHelperIns->cutFiles(view);
}

void ShortcutHelper::pasteFiles()
{
    fmInfo() << "Paste files operation initiated";

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    auto sourceUrls = ClipBoard::instance()->clipboardFileUrlList();

    fmDebug() << "Paste operation - source files count:" << sourceUrls.size() << "target:" << view->rootUrl().toString();
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_ShortCut_PasteFiles", windowId, sourceUrls, view->rootUrl())) {
        fmDebug() << "Paste files handled by hook";
        return;
    }

    FileOperatorHelperIns->pasteFiles(view);
}

void ShortcutHelper::undoFiles()
{
    fmInfo() << "Undo files operation";
    FileOperatorHelperIns->undoFiles(view);
}

void ShortcutHelper::deleteFiles()
{
    const QList<QUrl> &selectUrls = view->selectedTreeViewUrlList();
    if (selectUrls.isEmpty()) {
        fmDebug() << "Delete operation canceled - no files selected";
        return;
    }

    fmInfo() << "Delete files permanently - count:" << selectUrls.size();

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_ShortCut_DeleteFiles", windowId, selectUrls, view->rootUrl())) {
        fmDebug() << "Delete files handled by hook";
        return;
    }

    // Todo(yanghao):only support trash on root url
    // 共享文件夹不用弹出彻底删除对话框
    // 网络邻居目录不用弹出彻底删除对话框
    // 最近使用目录不用弹出彻底删除对话框
    FileOperatorHelperIns->deleteFiles(view);
}

void ShortcutHelper::moveToTrash()
{
    const QList<QUrl> &selectUrls = view->selectedTreeViewUrlList();
    if (selectUrls.isEmpty()) {
        fmDebug() << "Move to trash canceled - no files selected";
        return;
    }

    fmInfo() << "Move to trash operation - count:" << selectUrls.size();

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_ShortCut_MoveToTrash", windowId, selectUrls, view->rootUrl())) {
        fmDebug() << "Move to trash handled by hook";
        return;
    }
    // Todo(lanxs): QUrl to LocalFile
    // complete deletion eg: gvfs, vault
    // only support trash on root url
    FileOperatorHelperIns->moveToTrash(view, selectUrls);
}

void ShortcutHelper::touchFolder()
{
    fmInfo() << "Creating new folder";
    // Todo(yanghao):editingIndex handle
    view->clearSelection();
    FileOperatorHelperIns->touchFolder(view);
}

void ShortcutHelper::toggleHiddenFiles()
{
    bool isShowedHiddenFiles = Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool();
    bool newState = !isShowedHiddenFiles;
    fmInfo() << "Toggling hidden files visibility from" << isShowedHiddenFiles << "to" << newState;
    Application::instance()->setGenericAttribute(Application::kShowedHiddenFiles, newState);
}

void ShortcutHelper::showFilesProperty()
{
    // ToDo(yanghao): network not support
    FileOperatorHelperIns->showFilesProperty(view);
}

void ShortcutHelper::previewFiles()
{
    QList<QUrl> urls = view->selectedUrlList();

    if (urls.isEmpty()) {
        fmDebug() << "Preview canceled - no files selected";
        return;
    }

    fmInfo() << "Preview files operation - count:" << urls.size();

    auto windowId = WorkspaceHelper::instance()->windowId(view);
    if (dpfHookSequence->run(kCurrentEventSpace, "hook_ShortCut_PreViewFiles", windowId, urls, view->rootUrl())) {
        fmDebug() << "Preview files handled by hook";
        return;
    }

    QList<QUrl> selectUrls = urls;
    QList<QUrl> urlsTrans {};
    bool ok = UniversalUtils::urlsTransformToLocal(urls, &urlsTrans);
    if (ok && !urlsTrans.isEmpty())
        selectUrls = urlsTrans;

    urls = view->model()->getChildrenUrls();

    urlsTrans.clear();
    QList<QUrl> currentDirUrls = urls;
    ok = UniversalUtils::urlsTransformToLocal(currentDirUrls, &urlsTrans);
    if (ok && !urlsTrans.isEmpty())
        currentDirUrls = urlsTrans;

    FileOperatorHelperIns->previewFiles(view, selectUrls, currentDirUrls);
}

void ShortcutHelper::openAction(const QList<QUrl> &urls, const DirOpenMode openMode)
{
    const auto mode = urls.count() > 1 ? DirOpenMode::kOpenNewWindow : openMode;
    fmInfo() << "Open action - files count:" << urls.size() << "mode:" << static_cast<int>(mode);
    FileOperatorHelperIns->openFilesByMode(view, urls, mode);
}

void ShortcutHelper::openInTerminal()
{
    FileOperatorHelperIns->openInTerminal(view);
}

void ShortcutHelper::cdUp()
{
    view->cdUp();
}

void ShortcutHelper::redoFiles()
{
    FileOperatorHelperIns->redoFiles(view);
}

bool ShortcutHelper::reverseSelect()
{
    if (view->selectionMode() == FileView::SingleSelection) {
        fmDebug() << "Reverse selection canceled - single selection mode";
        return false;
    }

    auto urls = view->selectedUrlList();
    if (urls.isEmpty()) {
        fmDebug() << "Reverse selection canceled - no files selected";
        return false;
    }

    fmInfo() << "Reverse selection operation - currently selected:" << urls.size();

    QList<QUrl> list = view->model()->getChildrenUrls();
    for (const QUrl &url : urls) {
        list.removeAll(url);
    }

    if (list.isEmpty()) {
        fmDebug() << "All files were selected - clearing selection";
        view->selectionModel()->clear();
    }

    view->selectFiles(list);
    fmInfo() << "Reverse selection completed - new selection count:" << list.size();
    return true;
}

void ShortcutHelper::renameProcessing()
{
    renameProcessTimer->start();
}
