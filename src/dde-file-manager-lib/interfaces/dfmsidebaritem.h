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
#pragma once

#include <DStandardItem>

#include "durl.h"
#include "dfmglobal.h"

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMSideBarItem : public DStandardItem
{
public:
    enum Roles {
        ItemTypeRole = Dtk::UserRole + 1,
        ItemUrlRole,
        ItemGroupNameRole,
        ItemUseRegisteredHandlerRole,
        ItemUniqueKeyRole,
        ItemVolTagRole,
        ItemUserCustomRole = Dtk::UserRole + 0x0100
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

    DFMSideBarItem(const DUrl &url = DUrl(), const QString &groupName = "default") : DFMSideBarItem (QIcon(), QString(), url, groupName) {}
    DFMSideBarItem(const QIcon &icon, const QString &text, const DUrl &url = DUrl(), const QString &groupName = "default");

    static DFMSideBarItem *createSeparatorItem(const QString &groupName);

    DUrl url() const;
    QString groupName() const;
    int itemType() const;
    int type() const;
    bool useRegisteredHandler() const;
    QString registeredHandler(const QString &fallback = QString()) const;

    void setUrl(const DUrl &url);
    void setGroupName(const QString &groupName);
    void setRegisteredHandler(const QString &identifier);

    constexpr static int SidebarItemType = QStandardItem::UserType + 1;

private:
    void initModelData();
};

DFM_END_NAMESPACE
