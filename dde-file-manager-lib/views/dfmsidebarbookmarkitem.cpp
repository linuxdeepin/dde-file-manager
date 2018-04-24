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
#include "controllers/bookmarkmanager.h"

#include <QMenu>

DFM_BEGIN_NAMESPACE

DFMSideBarBookmarkItem::DFMSideBarBookmarkItem(const DUrl &url)
    : DFMSideBarItem(url)
{
    setIconFromThemeConfig("BookmarkItem.BookMarks", "icon");
    setReorderable(true);

    connect(this, &DFMSideBarBookmarkItem::reorder, this,
    [](DFMSideBarItem * ori, DFMSideBarItem * dst, bool insertBefore) {
        int oriIdx = bookmarkManager->getBookmarkIndex(ori->url());
        int dstIdx = bookmarkManager->getBookmarkIndex(dst->url());
        if (oriIdx == dstIdx) {
            return;
        }
        int newIdx = dstIdx;
        if (insertBefore) {
            if (dstIdx > oriIdx) {
                newIdx = dstIdx - 1;
            }
        } else {
            if (dstIdx < oriIdx) {
                newIdx = dstIdx + 1;
            }
        }
        bookmarkManager->moveBookmark(oriIdx, newIdx);
    });
}

QMenu *DFMSideBarBookmarkItem::createStandardContextMenu() const
{
    // this part could be duplicate since it seems every sidebar item should got
    // a new window/tab option and a properties option. maybe we need a menu manager
    // or other workaround?
    QMenu *menu = new QMenu();
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());

    menu->addAction(QObject::tr("Open in new window"), [this]() {
        WindowManager::instance()->showNewWindow(url(), true);
    });

    menu->addAction(QObject::tr("Open in new tab"), [wnd, this]() {
        wnd->openNewTab(url());
    });

    menu->addAction(QObject::tr("Rename"), [this]() {
        DFMSideBarBookmarkItem *ccItem = const_cast<DFMSideBarBookmarkItem *>(this);
        ccItem->showRenameEditor();
    });

    menu->addAction(QObject::tr("Remove"), [ = ]() {
        fileService->deleteFiles(this, DUrlList{url()}, true);
    });

    menu->addAction(QObject::tr("Properties"), [this]() {
        DUrlList list;

        const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(this, url());

        list.append(info->redirectedFileUrl());
        fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(this, list));
    });

    return menu;
}


DFM_END_NAMESPACE
