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
#pragma once

#include <QStandardItem>

#include "durl.h"

class DFMLeftSideBarItem : public QStandardItem
{
public:
    enum Roles {
        ItemTypeRole = Qt::UserRole + 1,
        ItemUrlRole = Qt::UserRole + 2,
        ItemGroupNameRole = Qt::UserRole + 3,
        ItemCdActionRole = Qt::UserRole + 4,
        ItemUserCustomRole = Qt::UserRole + 0x0100
    };

    enum ItemTypes {
        SidebarItem,
        Separator
    };

    enum CdAction {
        ChangeDirectory,
        MountPartitionThenCd,
        UserCustom = 0x0100
    };

    DFMLeftSideBarItem(const DUrl &url = DUrl(), const QString &groupName = "default") : DFMLeftSideBarItem (QIcon(), QString(), url, groupName) {}
    DFMLeftSideBarItem(const QIcon &icon, const QString &text, const DUrl &url = DUrl(), const QString &groupName = "default");

    static DFMLeftSideBarItem *createSystemPathItem(const QString &pathKey, const QString &groupName);
    static DFMLeftSideBarItem *createDeviceItem(const DUrl &url, const QString &groupName);
    static DFMLeftSideBarItem *createBookmarkItem(const DUrl &url, const QString &groupName);
    static DFMLeftSideBarItem *createTagItem(const DUrl &url, const QString &groupName);
    static DFMLeftSideBarItem *createSeparatorItem(const QString &groupName);

    DUrl url() const;
    QString groupName() const;
    int cdActionType() const;
    int itemType() const;
    int type() const;

    void setUrl(const DUrl &url);

    constexpr static int SidebarItemType = QStandardItem::UserType + 1;

private:
    void initModelData();
};
