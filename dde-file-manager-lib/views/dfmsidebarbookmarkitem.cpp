/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmsidebarbookmarkitem.h"
#include "dfilemanagerwindow.h"

#include "singleton.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "views/windowmanager.h"
#include "dialogs/dialogmanager.h"
#include "controllers/bookmarkmanager.h"

#include <ddialog.h>

#include <QMenu>
#include <QDialog>

DFM_BEGIN_NAMESPACE
DWIDGET_USE_NAMESPACE

DFMSideBarBookmarkItem::DFMSideBarBookmarkItem(const DUrl &url, QWidget *parent)
    : DFMSideBarItem(url, parent)
{
    setIconFromThemeConfig("BookmarkItem.BookMarks", "icon");
    setReorderable(true);
    setAutoOpenUrlOnClick(false);
    setCanDeleteViaDrag(true);

    connect(this, &DFMSideBarBookmarkItem::clicked, this,
    [this]() {
        DAbstractFileInfoPointer info = fileService->createFileInfo(this, this->url());
        if (info->exists()) {
            DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
            wnd->cd(this->url()/*.bookmarkTargetUrl()*/);
        } else {
            int ret = dialogManager->showRemoveBookMarkDialog(DFMEvent(this));
            if (ret == QDialog::Accepted) {
                fileService->deleteFiles(this, {this->url()}, false);
            }
        }
    }, Qt::QueuedConnection);
    // blumia: ^ This argument should be here since once we remove item from bookmark sidebar,
    //          the instance of DFMSideBarItem will be destoryed, thus we can't invoke the signal
    //          immediately, put this into the eventLoop will solve the problem.
}

QMenu *DFMSideBarBookmarkItem::createStandardContextMenu() const
{
    // this part could be duplicate since it seems every sidebar item should got
    // a new window/tab option and a properties option. maybe we need a menu manager
    // or other workaround?

    QMenu *menu = new QMenu();
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());
    bool shouldEnable = WindowManager::tabAddableByWinId(wnd->windowId());
    const DAbstractFileInfoPointer& info = DFileService::instance()->createFileInfo(this, url());
    bool fileExist = info->exists();

    menu->addAction(QObject::tr("Open in new window"), [this]() {
        WindowManager::instance()->showNewWindow(url(), true);
    })->setEnabled(fileExist);

    menu->addAction(QObject::tr("Open in new tab"), [wnd, this]() {
        wnd->openNewTab(url());
    })->setEnabled(fileExist && shouldEnable);

    menu->addSeparator();

    menu->addAction(QObject::tr("Rename"), [this]() {
        DFMSideBarBookmarkItem *ccItem = const_cast<DFMSideBarBookmarkItem *>(this);
        ccItem->showRenameEditor();
    })->setEnabled(fileExist);

    menu->addAction(QObject::tr("Remove"), [ = ]() {
        fileService->deleteFiles(this, DUrlList{url()}, false);
    });

    menu->addSeparator();

    menu->addAction(QObject::tr("Properties"), [ = ]() {
        DUrlList list;
        list.append(info->redirectedFileUrl());
        fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(this, list));
    })->setEnabled(fileExist);

#ifndef QT_NO_DEBUG
    menu->addAction(QStringLiteral("Debug Info"), [ = ]() {
        const DAbstractFileInfo * infoData = info.constData();
        const BookMark * bookmarkInfo = static_cast<const BookMark*>(infoData);
        DDialog d;
        QString messageLines;
        messageLines.append("Bookmark Url: " + url().toString() + "\n");
        messageLines.append(QStringLiteral("(i) Bookmark Exist: ") + (info->exists() ? "Y" : "N") + "\n");
        messageLines.append("(i) Bookmark Mount Point: " + bookmarkInfo->mountPoint + "\n");
        messageLines.append("(i) Bookmark Locate Url: " + bookmarkInfo->locateUrl + "\n");
        d.setMessage(messageLines);
        d.addButton("Gotcha", true, DDialog::ButtonNormal);
        d.exec();
    });
#endif

    return menu;
}


DFM_END_NAMESPACE
