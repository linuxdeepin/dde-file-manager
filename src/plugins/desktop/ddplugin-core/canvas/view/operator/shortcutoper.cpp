/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "shortcutoper.h"
#include "view/canvasview.h"
#include "view/canvasmodel.h"
#include "view/canvasselectionmodel.h"
#include "delegate/canvasitemdelegate.h"
#include "view/canvasview_p.h"
#include "fileoperaterproxy.h"

#include <DApplication>

#include <QAction>

DWIDGET_USE_NAMESPACE
DSB_D_USE_NAMESPACE

#define regAction(shortcut, slotFunc) \
    {\
        QAction *action = new QAction(view);\
        action->setShortcut(shortcut);\
        view->addAction(action);\
        connect(action, &QAction::triggered, this, slotFunc);\
    }

ShortcutOper::ShortcutOper(CanvasView *parent)
    : QObject(parent)
    , view(parent)
{
}

void ShortcutOper::regShortcut()
{
    regAction(QKeySequence::HelpContents, &ShortcutOper::helpAction); // F1
    regAction(QKeySequence::Refresh, [this](){view->refresh();}); // F5
    regAction(QKeySequence::Delete, &ShortcutOper::moveToTrash); // Del
    regAction(QKeySequence::SelectAll, [this](){view->selectAll();}); // ctrl+a
    regAction(QKeySequence::ZoomIn, [this](){qDebug() << "zoom in";}); // ctrl+-
    regAction(QKeySequence::ZoomOut, [this](){qDebug() << "zoom out";});// ctrl++(c_s_=)
    regAction(QKeySequence::Copy, &ShortcutOper::copyFiles);// ctrl+c
    regAction(QKeySequence::Cut, &ShortcutOper::cutFiles);// ctrl+x
    regAction(QKeySequence::Paste, &ShortcutOper::pasteFiles);// ctrl+v
    regAction(QKeySequence::Undo, [this](){qDebug() << "Undo";});// ctrl+z
    //regAction(QKeySequence::New, [this](){qDebug() << "New";});// ctrl+n
    //regAction(QKeySequence::Open, &ShortcutOper::openAction); // ctrl+o
}

bool ShortcutOper::keyPressed(QKeyEvent *event)
{
    if (!event || !view)
        return false;

    Qt::KeyboardModifiers modifiers = event->modifiers();
    auto key = event->key();
    if (modifiers == Qt::NoModifier) {
        switch (key) {
        case Qt::Key_Tab:
            tabToFirst();
            return true;
        case Qt::Key_Escape:
            // todo clear clipboard
        default:
            break;
        }
    }

    if (modifiers == Qt::KeypadModifier || modifiers == Qt::NoModifier) {
        switch (key) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            openAction();
            return true;
        case Qt::Key_Space:
            // todo preview
            break;
        default:
            break;
        }
    } else if (modifiers == Qt::ShiftModifier) {
        switch (key) {
        case Qt::Key_Delete:
            deleteFile();
            break;
        case Qt::Key_T:
            // open terminal. but no need to do it.
            return true;
        default:
            break;
        }
    } else if (modifiers == Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_Equal:
            qDebug() << "zoom in";
            return true;
        case Qt::Key_H:
            // todo swich hidden
            return true;
        case Qt::Key_I:
            //todo show property dialog
            return true;
        default:
            break;
        }
    } else if (modifiers == Qt::AltModifier) {
        if (key == Qt::Key_M) {
            showMenu();
            return true;
        }
    }

    return false;
}

void ShortcutOper::helpAction()
{
    class PublicApplication : public DApplication
    {
    public:
        using  DApplication::handleHelpAction;
    };

    QString appName = qApp->applicationName();
    qApp->setApplicationName("dde");
    reinterpret_cast<PublicApplication *>(DApplication::instance())->handleHelpAction();
    qApp->setApplicationName(appName);
}

void ShortcutOper::openAction()
{
    FileOperaterProxyIns->openFiles(view);
}

void ShortcutOper::moveToTrash()
{
    FileOperaterProxyIns->moveToTrash(view);
}

void ShortcutOper::deleteFile()
{
    FileOperaterProxyIns->deleteFiles(view);
}

void ShortcutOper::copyFiles()
{
    FileOperaterProxyIns->copyFiles(view);
}

void ShortcutOper::cutFiles()
{
    FileOperaterProxyIns->cutFiles(view);
}

void ShortcutOper::pasteFiles()
{
    FileOperaterProxyIns->pasteFiles(view);
}

void ShortcutOper::tabToFirst()
{
    view->selectionModel()->clear();
    QKeyEvent downKey(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QCoreApplication::sendEvent(view, &downKey);
}

void ShortcutOper::showMenu()
{
    //todo enable menu

    QModelIndexList indexList = view->selectionModel()->selectedIndexes();
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
        flags = view->model()->flags(index);
        if (!flags.testFlag(Qt::ItemIsEnabled)) {
            isEmptyArea = true;
            flags = view->rootIndex().flags();
        }
    }

    view->itemDelegate()->revertAndcloseEditor();
    if (isEmptyArea) {
        view->selectionModel()->clearSelection();
        view->d->menuProxy->showEmptyAreaMenu(flags, QPoint(0, 0));
    } else {
       auto gridPos = view->d->gridAt(view->visualRect(index).center());
       view->d->menuProxy->showNormalMenu(index, flags, gridPos);
    }
}
