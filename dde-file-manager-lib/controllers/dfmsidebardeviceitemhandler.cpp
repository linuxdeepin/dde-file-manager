/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
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

#include "dfmsidebardeviceitemhandler.h"

#include "singleton.h"
#include "app/define.h"
#include "app/filesignalmanager.h"
#include "dfileservices.h"
#include "views/dfilemanagerwindow.h"
#include "views/dfmsidebar.h"
#include "views/windowmanager.h"
#include "gvfs/gvfsmountmanager.h"
#include "interfaces/dfmsidebaritem.h"
#include "deviceinfo/udiskdeviceinfo.h"
#include "dfmsidebarmanager.h"
#include "dfilemenumanager.h"
#include "dfilemenu.h"

#include <QAction>

DFM_BEGIN_NAMESPACE

DViewItemAction *DFMSideBarDeviceItemHandler::createUnmountOrEjectAction(const DUrl &url, bool withText)
{
    DViewItemAction * action = new DViewItemAction(Qt::AlignCenter, QSize(16, 16), QSize(), true);
    if (withText) {
        action->setText(QObject::tr("Unmount"));
    }
    action->setIcon(QIcon::fromTheme("media-eject-symbolic"));

    QObject::connect(action, &QAction::triggered, action, [url](){
        const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(nullptr, url);
        QVariantHash info = infoPointer->extraProperties();
        if (info.value("canUnmount", false).toBool()) {
            AppController::instance()->actionUnmount(dMakeEventPointer<DFMUrlBaseEvent>(nullptr, url));
        }
    });

    return action;
}

DFMSideBarItem *DFMSideBarDeviceItemHandler::createItem(const DUrl &url)
{
    const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(nullptr, url);
    QVariantHash info = infoPointer->extraProperties();
    QString displayName = infoPointer->fileDisplayName();
    QString iconName = infoPointer->iconName() + "-symbolic";

    DFMSideBarItem * item = new DFMSideBarItem(QIcon::fromTheme(iconName), displayName, url);

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    if (infoPointer->menuActionList().contains(MenuAction::Rename))
        flags |= Qt::ItemIsEditable;
    item->setFlags(flags);

    item->setData(SIDEBAR_ID_DEVICE, DFMSideBarItem::ItemUseRegisteredHandlerRole);

    DViewItemActionList lst;
    DViewItemAction * act = createUnmountOrEjectAction(url, false);
    act->setIcon(QIcon::fromTheme("media-eject-symbolic"));
    act->setVisible(infoPointer->menuActionList().contains(MenuAction::Unmount));
    lst.push_back(act);
    item->setActionList(Qt::RightEdge, lst);

    return item;
}

DFMSideBarDeviceItemHandler::DFMSideBarDeviceItemHandler(QObject *parent)
    : DFMSideBarItemInterface (parent)
{

}

void DFMSideBarDeviceItemHandler::cdAction(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    AppController::instance()->actionOpenDisk(dMakeEventPointer<DFMUrlBaseEvent>(sidebar, item->url()));
}

QMenu *DFMSideBarDeviceItemHandler::contextMenu(const DFMSideBar *sidebar, const DFMSideBarItem *item)
{
    const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(this, item->url());
    QVariantHash info = infoPointer->extraProperties();
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(sidebar->topLevelWidget());
    bool shouldDisable = !WindowManager::tabAddableByWinId(wnd->windowId());
    QSet<MenuAction> disabled;

    if (shouldDisable) {
        disabled.insert(MenuAction::OpenInNewTab);
        disabled.insert(MenuAction::OpenDiskInNewTab);
    }
    if (!info["mounted"].toBool()) {
        disabled.insert(MenuAction::Property);
    }
    DFileMenu *menu = DFileMenuManager::genereteMenuByKeys(infoPointer->menuActionList(), disabled);
    menu->setEventData(DUrl(), {item->url()}, WindowManager::getWindowId(wnd), sidebar);

    return menu;
}

void DFMSideBarDeviceItemHandler::rename(const DFMSideBarItem *item, QString name)
{
    const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(this, item->url());
    if (infoPointer->fileDisplayName() != name) {
        DFileService::instance()->renameFile(this, item->url(), DUrl(name));
    }
}

DFM_END_NAMESPACE
