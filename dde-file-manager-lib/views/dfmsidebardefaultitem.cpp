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
#include "dfmsidebardefaultitem.h"

#include "app/define.h"
#include "controllers/pathmanager.h"

#include "singleton.h"

DFM_BEGIN_NAMESPACE

DFMSideBarDefaultItem::DFMSideBarDefaultItem(DFMStandardPaths::StandardLocation location, QWidget *parent)
    : DFMSideBarItem(getDUrlFromStandardLocation(location), parent)
{
    initItemByLocation(location);
    setReorderable(false);
}

int DFMSideBarDefaultItem::sortingPriority() const
{
    if (url() == getDUrlFromStandardLocation(DFMStandardPaths::ComputerRootPath)) {
        return -2;
    }
    return 0;
}

DUrl DFMSideBarDefaultItem::getDUrlFromStandardLocation(DFMStandardPaths::StandardLocation location) const
{
    DUrl path;

    switch (location) {
    case DFMStandardPaths::StandardLocation::TrashPath:
        path = DUrl::fromTrashFile("/");
        break;
    default:
        path = DUrl::fromUserInput(DFMStandardPaths::location(location));
        break;
    }

    return path;
}

void DFMSideBarDefaultItem::initItemByLocation(DFMStandardPaths::StandardLocation location)
{
    switch (location) {
    case DFMStandardPaths::StandardLocation::RecentPath:
        setIconFromThemeConfig("BookmarkItem.Recent");
        setText(systemPathManager->getSystemPathDisplayName("Recent"));
        break;
    case DFMStandardPaths::StandardLocation::HomePath:
        setIconFromThemeConfig("BookmarkItem.Home");
        break;
    case DFMStandardPaths::StandardLocation::DesktopPath:
        setIconFromThemeConfig("BookmarkItem.Desktop");
        break;
    case DFMStandardPaths::StandardLocation::VideosPath:
        setIconFromThemeConfig("BookmarkItem.Videos");
        break;
    case DFMStandardPaths::StandardLocation::MusicPath:
        setIconFromThemeConfig("BookmarkItem.Music");
        break;
    case DFMStandardPaths::StandardLocation::PicturesPath:
        setIconFromThemeConfig("BookmarkItem.Pictures");
        break;
    case DFMStandardPaths::StandardLocation::DocumentsPath:
        setIconFromThemeConfig("BookmarkItem.Documents");
        break;
    case DFMStandardPaths::StandardLocation::DownloadsPath:
        setIconFromThemeConfig("BookmarkItem.Downloads");
        break;
    case DFMStandardPaths::StandardLocation::TrashPath:
        setIconFromThemeConfig("BookmarkItem.Trash");
        break;
    case DFMStandardPaths::StandardLocation::ComputerRootPath:
        setText(systemPathManager->getSystemPathDisplayName("Computer"));
        setIconFromThemeConfig("BookmarkItem.Computer");
        break;
    case DFMStandardPaths::StandardLocation::Root:
        setText(systemPathManager->getSystemPathDisplayName("System Disk"));
        setIconFromThemeConfig("BookmarkItem.Disk");
        break;
    case DFMStandardPaths::StandardLocation::NetworkRootPath:
        setText(systemPathManager->getSystemPathDisplayName("Network"));
        setIconFromThemeConfig("BookmarkItem.Network");
        break;
    case DFMStandardPaths::StandardLocation::UserShareRootPath:
        setText(systemPathManager->getSystemPathDisplayName("UserShare"));
        setIconFromThemeConfig("BookmarkItem.UserShare");
        break;
    default:
        break;
    }
}

DFM_END_NAMESPACE
