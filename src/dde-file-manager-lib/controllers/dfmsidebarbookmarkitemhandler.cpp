/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "dfmsidebarbookmarkitemhandler.h"

#include "singleton.h"
#include "dfileservices.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "views/windowmanager.h"
#include "controllers/bookmarkmanager.h"
#include "interfaces/dfmsidebaritem.h"
#include "dialogs/dialogmanager.h"
#include "dfmsidebarmanager.h"
#include "interfaces/dfilemenu.h"

#include <DDialog>

DFM_BEGIN_NAMESPACE

DFMSideBarItem *DFMSideBarBookmarkItemHandler::createItem(const DUrl &url)
{
    // leave url a default display name.
    QString displayName = url.bookmarkName();

    QIcon icon = QIcon::fromTheme("folder-bookmark-symbolic");
    DFMSideBarItem * item = new DFMSideBarItem(icon, displayName, url);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemNeverHasChildren);
    item->setRegisteredHandler(SIDEBAR_ID_BOOKMARK);

    return item;
}

DFMSideBarBookmarkItemHandler::DFMSideBarBookmarkItemHandler()
{

}

void DFMSideBarBookmarkItemHandler::cdAction(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(nullptr, item->url());
    if (info->exists()) {
        DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
        wnd->cd(info->fileUrl());
    } else {
        int ret = dialogManager->showRemoveBookMarkDialog(DFMEvent(this));
        if (ret == QDialog::Accepted) {
            DFileService::instance()->deleteFiles(this, {item->url()}, false);
        }
    }
}

QMenu *DFMSideBarBookmarkItemHandler::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    // this part could be duplicate since it seems every sidebar item should got
    // a new window/tab option and a properties option. maybe we need a menu manager
    // or other workaround?

    DFileMenu *menu = new DFileMenu();
    menu->setAccessibleInfo(AC_FILE_MENU_SIDEBAR_BOOK_MARK);

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
    bool shouldEnable = WindowManager::tabAddableByWinId(wnd->windowId());
    const DAbstractFileInfoPointer& info = DFileService::instance()->createFileInfo(nullptr, item->url());
    bool fileExist = info->exists();

    menu->addAction(QObject::tr("Open in new window"), [item]() {
        WindowManager::instance()->showNewWindow(item->url(), true);
    })->setEnabled(fileExist);

    menu->addAction(QObject::tr("Open in new tab"), [wnd, item]() {
        wnd->openNewTab(item->url());
    })->setEnabled(fileExist && shouldEnable);

    menu->addSeparator();

    menu->addAction(QObject::tr("Rename"), [sidebar, item]() {
        int index = sidebar->findItem(item);
        if (index >= 0) {
            sidebar->openItemEditor(index);
        }
    })->setEnabled(fileExist);

    menu->addAction(QObject::tr("Remove"), [item]() {
        DFileService::instance()->deleteFiles(nullptr, DUrlList{item->url()}, false);
    });

    menu->addSeparator();

    menu->addAction(QObject::tr("Properties"), [ = ]() {
        DUrlList list;
        //fix bug 33005 在是smb文件和ftp文件加标签后redirectedFileUrl取出的url对应不上，自己补齐
        DUrl openurl = info->redirectedFileUrl();
        QString strscheme = openurl.scheme();
        if (strscheme == SMB_SCHEME || strscheme == FTP_SCHEME || strscheme == SFTP_SCHEME)
        {
            QString sharename = info->absolutePath().mid(info->absolutePath().lastIndexOf("=")+1)+"/";
            QString localname = openurl.path().replace(sharename,"");
            openurl = DUrl(info->absolutePath() + localname);
            openurl.setScheme(FILE_SCHEME);
        }
        list.append(openurl);
        fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(nullptr, list));
    })->setEnabled(fileExist);

#ifndef QT_NO_DEBUG
    menu->addAction(QStringLiteral("Debug Info"), [ = ]() {
        const DAbstractFileInfo * infoData = info.constData();
        const BookMark * bookmarkInfo = static_cast<const BookMark*>(infoData);
        DDialog d;
        QString messageLines;
        messageLines.append("Bookmark Url: " + item->url().toString() + "\n");
        messageLines.append(QStringLiteral("(i) Bookmark Exist: ") + (info->exists() ? "Y" : "N") + "\n");
        messageLines.append("(i) Bookmark Mount Point: " + bookmarkInfo->mountPoint + "\n");
//        messageLines.append("(i) Bookmark Locate Url: " + bookmarkInfo->locateUrl + "\n");
        messageLines.append("(i) Bookmark Locate Url: " + QString(QByteArray::fromBase64(bookmarkInfo->locateUrl.toLocal8Bit())) + "\n");
        messageLines.append("(i) Bookmark Redirected Url: " + bookmarkInfo->redirectedFileUrl().toString() + "\n");
        d.setMessage(messageLines);
        d.addButton("Gotcha", true, DDialog::ButtonNormal);
        d.exec();
    });
#endif

    return menu;
}

void DFMSideBarBookmarkItemHandler::rename(const DFMSideBarItem *item, QString name)
{
    DUrl newUrl = item->url();
    newUrl.setFragment(name);
    DFileService::instance()->renameFile(this, item->url(), newUrl, true);
}

DFM_END_NAMESPACE
