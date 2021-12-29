/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMSIDEBARITEM_H
#define DFMSIDEBARITEM_H

#include "dfmplugin_sidebar_global.h"

#include <DStandardItem>
#include <QUrl>

DWIDGET_USE_NAMESPACE

DPSIDEBAR_BEGIN_NAMESPACE

class SideBarItem : public DStandardItem
{
public:
    enum Roles {
        ItemUrlRole = Dtk::UserRole + 1,
        ItemGroupRole,
        ItemUserCustomRole = Dtk::UserRole + 0x0100
    };

    explicit SideBarItem(const QUrl &url);
    explicit SideBarItem(const QIcon &icon,
                         const QString &text,
                         const QString &group,
                         const QUrl &url);
    explicit SideBarItem(const SideBarItem &item);
    virtual ~SideBarItem();

    void setUrl(const QUrl &url);
    void setGroup(const QString &group = "");
    QUrl url() const;
    QString group() const;
};
DPSIDEBAR_END_NAMESPACE

class DFMSideBarItemSeparator : public DPSIDEBAR_NAMESPACE::SideBarItem
{
public:
    explicit DFMSideBarItemSeparator(const QString &group = "");
    virtual ~DFMSideBarItemSeparator();
};

Q_DECLARE_METATYPE(DFMSideBarItemSeparator)

#endif   //DFMSIDEBARITEM_H
