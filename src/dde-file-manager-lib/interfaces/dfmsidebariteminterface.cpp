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

#include "dfmsidebariteminterface.h"

#include "singleton.h"
#include "app/filesignalmanager.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "views/windowmanager.h"
#include "interfaces/dfmsidebaritem.h"
#include "interfaces/dfilemenu.h"
#include "controllers/vaultcontroller.h"


DFMSideBarItemInterface::DFMSideBarItemInterface(QObject *parent) : QObject(parent)
{

}

void DFMSideBarItemInterface::cdAction(const DFMSideBar *sidebar, const DFMSideBarItem* item)
{
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
    if (item->itemType() != DFMSideBarItem::Separator) {
        qDebug() << " item->url() " << item->url();

        wnd->cd(item->url()); // don't `setChecked` here, wait for a signal.
    }
}

QMenu *DFMSideBarItemInterface::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    DFileMenu *menu = new DFileMenu();

    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());

    menu->addAction(QObject::tr("Open in new window"), [item]() {
        WindowManager::instance()->showNewWindow(item->url(), true);
    });

    menu->addAction(QObject::tr("Open in new tab"), [wnd, item]() {
        wnd->openNewTab(item->url());
    })->setDisabled(shouldDisable);

    menu->addSeparator();

    menu->addAction(QObject::tr("Properties"), [item]() {
        DUrlList list;
        list.append(item->url());
        Singleton<FileSignalManager>::instance()->requestShowPropertyDialog(DFMUrlListBaseEvent(nullptr, list));
    });

    return menu;
}

void DFMSideBarItemInterface::rename(const DFMSideBarItem *item, QString name)
{
    Q_UNUSED(item)
    Q_UNUSED(name)
    //no-op
}

