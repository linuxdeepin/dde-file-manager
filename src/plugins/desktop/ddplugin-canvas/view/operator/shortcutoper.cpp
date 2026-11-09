// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcutoper.h"
#include "view/canvasview.h"
#include "model/canvasselectionmodel.h"
#include "delegate/canvasitemdelegate.h"
#include "view/canvasview_p.h"
#include "fileoperatorproxy.h"
#include "canvasmanager.h"
#include "utils/fileutil.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/private/windowtransition_p.h>

#include <dfm-framework/event/event.h>

#include <DApplication>
#include <DGuiApplicationHelper>

#include <QAction>
#include <QPointer>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace ddplugin_canvas;

ShortcutOper::ShortcutOper(CanvasView *parent)
    : QObject(parent), view(parent)
{
    view->setObjectName("CanvasView");
    view->setAccessibleName("CanvasView");

    // Bind the transition managers long before any preview, so the Wayland
    // globals are ready when the first space press happens.
    windowtransition::bindManagers();
}

bool ShortcutOper::keyPressed(QKeyEvent *event)
{
    if (!event || !view)
        return false;

    {
        QVariantHash extData;
        extData.insert("DisableShortcut", disableShortcut());
        extData.insert("QKeyEvent", (qlonglong)event);
        if (view->d->hookIfs && view->d->hookIfs->shortcutkeyPress(view->screenNum(), event->key(), event->modifiers(), &extData))
            return true;
    }

    if (disableShortcut()) {
        bool specialShortcut = false;
        if (event->modifiers() == Qt::NoModifier || event->modifiers() == Qt::KeypadModifier) {
            switch (event->key()) {
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Left:
            case Qt::Key_Right:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                specialShortcut = true;
                break;
            default:
                specialShortcut = false;   // other keys is disable
            }
        }
        if (!specialShortcut)
            return true;   // return true to ingore the event.
    }

    Qt::KeyboardModifiers modifiers = event->modifiers();
    auto key = event->key();
    if (modifiers == Qt::NoModifier) {
        switch (key) {
        case Qt::Key_F1:
            helpAction();
            return true;
        case Qt::Key_Tab:
            tabToFirst();
            return true;
        case Qt::Key_Escape:
            clearClipBoard();
            return true;
        default:
            break;
        }
    }

    if (modifiers == Qt::KeypadModifier || modifiers == Qt::NoModifier) {
        switch (key) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            FileOperatorProxyIns->openFiles(view);
            return true;
        case Qt::Key_Space:
            if (!event->isAutoRepeat())
                previewFiles();
            return true;
        case Qt::Key_F5:
            view->refresh(false);
            return true;
        case Qt::Key_Delete:
            FileOperatorProxyIns->moveToTrash(view);
            return true;
        default:
            break;
        }
    } else if (modifiers == Qt::ShiftModifier) {
        switch (key) {
        case Qt::Key_Delete:
            FileOperatorProxyIns->deleteFiles(view);
            return true;
        case Qt::Key_T:
            // open terminal. but no need to do it.
            return true;
        default:
            break;
        }
    } else if (modifiers == Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_Minus:
            CanvasIns->onChangeIconLevel(false);
            return true;
        case Qt::Key_Equal:
            CanvasIns->onChangeIconLevel(true);
            return true;
        case Qt::Key_H:
            switchHidden();
            return true;
        case Qt::Key_I:
            FileOperatorProxyIns->showFilesProperty(view);
            return true;
        case Qt::Key_A:
            view->selectAll();
            return true;
        case Qt::Key_C:
            FileOperatorProxyIns->copyFiles(view);
            return true;
        case Qt::Key_X:
            FileOperatorProxyIns->cutFiles(view);
            return true;
        case Qt::Key_V:
            FileOperatorProxyIns->pasteFiles(view);
            return true;
        case Qt::Key_Z:
            FileOperatorProxyIns->undoFiles(view);
            return true;
        case Qt::Key_D:
            FileOperatorProxyIns->moveToTrash(view);
            return true;
        case Qt::Key_Y:
            FileOperatorProxyIns->redoFiles(view);
            return true;
        default:
            break;
        }
    } else if (modifiers == Qt::AltModifier) {
        if (key == Qt::Key_M) {
            showMenu();
            return true;
        }
    } else if (modifiers == (Qt::ControlModifier | Qt::ShiftModifier)) {
        if (key == Qt::Key_I) {
            view->d->keySelector->toggleSelect();
            return true;
        } else if (key == Qt::Key_N) {
            FileOperatorProxyIns->touchFolder(view, view->d->gridAt(QCursor::pos()));
            return true;
        } else if (key == Qt::Key_C) {
            FileOperatorProxyIns->copyFilePath(view);
            return true;
        }
    }

    return false;
}

bool ShortcutOper::disableShortcut() const
{
    return DFMBASE_NAMESPACE::Application::appObtuselySetting()->value(
                                                                       "ApplicationAttribute", "DisableDesktopShortcuts", false)
            .toBool();
}

void ShortcutOper::helpAction()
{
    QString appName = qApp->applicationName();
    qApp->setApplicationName("dde");
    DGuiApplicationHelper::instance()->handleHelpAction();
    qApp->setApplicationName(appName);
}

void ShortcutOper::tabToFirst()
{
    view->selectionModel()->clear();
    QKeyEvent downKey(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QCoreApplication::sendEvent(view, &downKey);
}

void ShortcutOper::showMenu()
{
    if (CanvasViewMenuProxy::disableMenu())
        return;

    QModelIndexList indexList = view->selectionModel()->selectedIndexesCache();
    bool isEmptyArea = indexList.isEmpty();
    Qt::ItemFlags flags;
    QModelIndex index;
    if (isEmptyArea) {
        index = view->rootIndex();
        flags = view->model()->flags(index);
        if (!flags.testFlag(Qt::ItemIsEnabled))
            return;
    } else {
        index = view->currentIndex();

        // the current index may be not in selected indexs."
        if (!indexList.contains(index)) {
            fmDebug() << "current index is not selected.";
            index = indexList.last();
        }

        flags = view->model()->flags(index);
        if (!flags.testFlag(Qt::ItemIsEnabled)) {
            fmInfo() << "file is disbale, switch to empty area" << view->model()->fileUrl(index);
            isEmptyArea = true;
            flags = view->rootIndex().flags();
        }
    }

    view->itemDelegate()->revertAndcloseEditor();
    if (isEmptyArea) {
        view->selectionModel()->clear();
        view->d->menuProxy->showEmptyAreaMenu(flags, QPoint(0, 0));
    } else {
        auto gridPos = view->d->gridAt(view->visualRect(index).center());
        view->d->menuProxy->showNormalMenu(index, flags, gridPos);
    }
}

void ShortcutOper::clearClipBoard()
{
    auto urls = ClipBoard::instance()->clipboardFileUrlList();
    auto homePath = view->model()->rootUrl();
    if (!urls.isEmpty()) {
        auto itemInfo = FileCreator->createFileInfo(urls.first(), Global::CreateFileInfoType::kCreateFileInfoSync);
        if (itemInfo && (itemInfo->pathOf(PathInfoType::kAbsolutePath) == homePath.toLocalFile()))
            ClipBoard::instance()->clearClipboard();
    }
}

void ShortcutOper::switchHidden()
{
    bool isShowedHiddenFiles = Application::instance()->genericAttribute(Application::kShowedHiddenFiles).toBool();
    Application::instance()->setGenericAttribute(Application::kShowedHiddenFiles, !isShowedHiddenFiles);
}
void ShortcutOper::previewFiles()
{
    const QList<QUrl> urls = view->selectionModel()->selectedUrls();
    if (urls.isEmpty())
        return;
    auto currentDirUrls = view->model()->files();

    const QUrl anchorUrl = urls.first();
    QWidget *window = view->topLevelWidget();
    if (!window || !window->windowHandle()) {
        fmWarning() << "Preview without transition: window:" << (window != nullptr)
                    << "windowHandle:" << (window ? window->windowHandle() != nullptr : false);
        dpfSlotChannel->push("dfmplugin_fileoperations", "slot_Operation_FilesPreview",
                             view->topLevelWidget()->winId(), urls, currentDirUrls, QString(), QString());
        return;
    }

    // The anchor is recomputed while the preview lives, so the close animation
    // follows the currently shown file (the preview may switch files) as well
    // as icon size changes and rearranges; the view may go away meanwhile,
    // which the provider reports by returning false. The viewport geometry is
    // read per call so window resizes are picked up.
    QPointer<CanvasView> v(view);

    auto provider = [v](const QUrl &url, QRect *anchor, QPixmap *snapshot) -> bool {
        if (!v) {
            fmWarning() << "Preview transition provider: view gone";
            return false;
        }

        QWidget *window = v->window();
        const QPoint viewportOrigin = v->viewport()->mapTo(window, QPoint(0, 0));
        const QRect viewportRect = v->viewport()->rect();

        const QRect cell = v->iconRect(url);   // viewport coordinates, grid lookup
        if (!cell.isValid()) {
            fmWarning() << "Preview transition provider: icon cell invalid for" << url;
            return false;
        }

        // The icon is resolved per url: the preview may switch files while it
        // is open, and the anchor has to follow the currently shown one.
        QIcon icon;
        const QModelIndex index = v->model()->index(url);
        if (index.isValid())
            icon = index.data(Global::kItemIconRole).value<QIcon>();
        if (icon.isNull()) {
            const FileInfoPointer &info = InfoFactory::create<FileInfo>(url);
            if (info)
                icon = info->fileIcon();
        }

        QPixmap cropped;
        const QRectF visible = windowtransition::paintedIconRect(icon, QRectF(cell), &cropped);
        if (visible.isEmpty()) {
            fmWarning() << "Preview transition provider: no visible icon, icon null:" << icon.isNull()
                        << "cell:" << cell;
            return false;
        }

        // Clip to the visible viewport; the snapshot follows the clip so it
        // keeps matching the anchor geometry.
        const QRect visibleRect = visible.toAlignedRect();
        const QRect clipped = visibleRect.intersected(viewportRect);
        if (clipped.isEmpty()) {
            fmWarning() << "Preview transition provider: anchor outside viewport, visible:" << visible
                        << "viewport:" << viewportRect;
            return false;
        }

        const QRect anchorRect = windowtransition::anchorInWindow(visibleRect, viewportRect, viewportOrigin);
        if (anchorRect.isEmpty())
            return false;

        *anchor = anchorRect;
        *snapshot = windowtransition::cropPixmapTo(cropped, visible, clipped);
        return true;
    };

    auto onToken = [this, urls, currentDirUrls](const QString &token, const QString &channel) {
        dpfSlotChannel->push("dfmplugin_fileoperations", "slot_Operation_FilesPreview",
                             view->topLevelWidget()->winId(), urls, currentDirUrls, token, channel);
    };

    if (!windowtransition::WindowTransitionLaunch::create(window->windowHandle(), anchorUrl, provider, onToken, this))
        onToken(QString(), QString());
}
