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

#include "dfmsidebaritem.h"
#include "dfmsidebaritemgroup.h"

#include "app/define.h"
#include "deviceinfo/udisklistener.h"
#include "controllers/bookmarkmanager.h"
#include "controllers/pathmanager.h"

#include "singleton.h"

DFM_BEGIN_NAMESPACE

DFMSideBarItemGroup::DFMSideBarItemGroup(DFMSideBar::GroupName groupName)
{
    setSpacing(0);
    bottomSeparator = new DFMSideBarItemSeparator();

    switch (groupName) {
    case DFMSideBar::GroupName::Common:
        appendItem(DFMStandardPaths::StandardLocation::HomePath);
        appendItem(DFMStandardPaths::StandardLocation::DesktopPath);
        appendItem(DFMStandardPaths::StandardLocation::VideosPath);
        appendItem(DFMStandardPaths::StandardLocation::MusicPath);
        appendItem(DFMStandardPaths::StandardLocation::PicturesPath);
        appendItem(DFMStandardPaths::StandardLocation::DocumentsPath);
        appendItem(DFMStandardPaths::StandardLocation::DownloadsPath);
        appendItem(DFMStandardPaths::StandardLocation::TrashPath);
        break;
    case DFMSideBar::GroupName::Device:
        appendItem(DFMStandardPaths::StandardLocation::ComputerRootPath);
        appendItem(DFMStandardPaths::StandardLocation::Root); // TODO: check dfmPlatformManager->isRoot_hidden()
        for (const UDiskDeviceInfoPointer &device : deviceListener->getDeviceList()) {
            // take care of this part, it seems we should mount the device if it's not yet mounted
            // and also for other devices like smartphone and etc.
            // so maybe we need a `appendItem(UDiskDeviceInfoPointer device)`
            appendItem(device.data()->getMountPointUrl());
        }
        break;
    case DFMSideBar::GroupName::Bookmark: {
        const QList<BookMarkPointer> &m_list = bookmarkManager->getBookmarks();
        for (const BookMarkPointer &bm : m_list) {
            DFMSideBarItem *item = new DFMSideBarItem(bm.data()->getUrl());
            item->setText(bm->getName());
            itemList.append(item);
            this->addWidget(item);
            //item->setIsCustomBookmark(true);
            //item->setBookmarkModel(bm);
            //m_scene->addItem(item);
        }
        break;
    }
    case DFMSideBar::GroupName::Network:
        appendItem(DFMStandardPaths::StandardLocation::NetworkRootPath);
        break;
    case DFMSideBar::GroupName::Tag:
        // nothing here
        break;
    default: // make compiler happy
        break;
    }

    this->addWidget(bottomSeparator);
    bottomSeparator->setVisible(itemList.count() != 0);
}

void DFMSideBarItemGroup::appendItem(DUrl url)
{
    DFMSideBarItem *item = new DFMSideBarItem(url);

    // for better indexing, counting and etc.
    itemList.append(item);

    this->addWidget(new DFMSideBarItem(url));
    bottomSeparator->setVisible(itemList.count() != 0);
}

void DFMSideBarItemGroup::appendItem(DFMStandardPaths::StandardLocation location)
{
    // Pre process to get a property DUrl path.
    DUrl path;
    switch (location) {
    case DFMStandardPaths::StandardLocation::TrashPath:
        path = DUrl::fromTrashFile("/");
        break;
    default:
        path = DUrl::fromUserInput(DFMStandardPaths::standardLocation(location));
        break;
    }

    // Then we create the item object
    DFMSideBarItem *item = new DFMSideBarItem(path);

    // Should do change icon, update text here.
    switch (location) {
    case DFMStandardPaths::StandardLocation::HomePath:
        item->setIconFromThemeConfig("BookmarkItem.Home");
        break;
    case DFMStandardPaths::StandardLocation::DesktopPath:
        item->setIconFromThemeConfig("BookmarkItem.Desktop");
        break;
    case DFMStandardPaths::StandardLocation::VideosPath:
        item->setIconFromThemeConfig("BookmarkItem.Videos");
        break;
    case DFMStandardPaths::StandardLocation::MusicPath:
        item->setIconFromThemeConfig("BookmarkItem.Music");
        break;
    case DFMStandardPaths::StandardLocation::PicturesPath:
        item->setIconFromThemeConfig("BookmarkItem.Pictures");
        break;
    case DFMStandardPaths::StandardLocation::DocumentsPath:
        item->setIconFromThemeConfig("BookmarkItem.Documents");
        break;
    case DFMStandardPaths::StandardLocation::DownloadsPath:
        item->setIconFromThemeConfig("BookmarkItem.Downloads");
        break;
    case DFMStandardPaths::StandardLocation::TrashPath:
        item->setIconFromThemeConfig("BookmarkItem.Trash");
        break;
    case DFMStandardPaths::StandardLocation::ComputerRootPath:
        item->setText(systemPathManager->getSystemPathDisplayName("Computer"));
        item->setIconFromThemeConfig("BookmarkItem.Computer");
        break;
    case DFMStandardPaths::StandardLocation::NetworkRootPath:
        item->setText(systemPathManager->getSystemPathDisplayName("Network"));
        item->setIconFromThemeConfig("BookmarkItem.Network");
    default:
        break;
    }

    // for better indexing, counting and etc.
    itemList.append(item);

    this->addWidget(item);
    bottomSeparator->setVisible(itemList.count() != 0);
}

DFM_END_NAMESPACE
