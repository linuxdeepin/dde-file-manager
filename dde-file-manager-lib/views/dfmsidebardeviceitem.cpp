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
#include "dfilemanagerwindow.h"
#include "dfmsidebardeviceitem.h"

#include "singleton.h"

#include "app/define.h"
#include "app/filesignalmanager.h"
#include "views/windowmanager.h"

#include <QMenu>

DFM_BEGIN_NAMESPACE

DFMSideBarDeviceItem::DFMSideBarDeviceItem(UDiskDeviceInfoPointer infoPointer, QWidget *parent)
    : DFMSideBarItem(infoPointer->getMountPointUrl(), parent)
{
    deviceInfo = infoPointer;
    setText(deviceInfo->getName());

    switch (deviceInfo->getMediaType()) {
    case UDiskDeviceInfo::MediaType::native:
        setIconFromThemeConfig("BookmarkItem.Disk");
        break;
    case UDiskDeviceInfo::MediaType::removable:
        setIconFromThemeConfig("BookmarkItem.Usb");
        break;
    case UDiskDeviceInfo::MediaType::dvd:
        setIconFromThemeConfig("BookmarkItem.Dvd");
        break;
    case UDiskDeviceInfo::MediaType::phone:
        setIconFromThemeConfig("BookmarkItem.Android");
        break;
    case UDiskDeviceInfo::MediaType::iphone:
        setIconFromThemeConfig("BookmarkItem.Iphone");
        break;
    default:
        break;
    }
}

QMenu *DFMSideBarDeviceItem::createStandardContextMenu() const
{
    QMenu *menu = new QMenu(const_cast<DFMSideBarDeviceItem *>(this));
    DFileManagerWindow *wnd = qobject_cast<DFileManagerWindow *>(topLevelWidget());

    menu->addAction(QObject::tr("Open in new window"), [this]() {
        WindowManager::instance()->showNewWindow(url(), true);
    });

    menu->addAction(QObject::tr("Open in new tab"), [wnd, this]() {
        wnd->openNewTab(url());
    });

    menu->addAction(QObject::tr("Unmount"));

    menu->addAction(QObject::tr("Properties"), [this]() {
        DUrlList list;
        list.append(url());
        fileSignalManager->requestShowPropertyDialog(DFMUrlListBaseEvent(this, list));
    });

    return menu;
}

DFM_END_NAMESPACE
