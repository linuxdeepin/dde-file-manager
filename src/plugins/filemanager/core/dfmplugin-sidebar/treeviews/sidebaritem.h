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
        kItemUrlRole = Dtk::UserRole + 1,
        kItemGroupRole,
        kItemUserCustomRole = Dtk::UserRole + 0x0100
    };

    explicit SideBarItem(const QUrl &url);
    explicit SideBarItem(const QIcon &icon,
                         const QString &text,
                         const QString &group,
                         const QUrl &url);
    explicit SideBarItem(const SideBarItem &item);
    virtual ~SideBarItem();

    QUrl url() const;
    void setUrl(const QUrl &url);

    QString group() const;
    void setGroup(const QString &group = "");
    QString subGourp() const;

    ItemInfo itemInfo() const;
};

class SideBarItemSeparator : public DPSIDEBAR_NAMESPACE::SideBarItem
{
public:
    explicit SideBarItemSeparator(const QString &group = "");
    virtual ~SideBarItemSeparator();
    void setExpanded(bool state);
    bool isExpanded();
    void setVisible(bool value);
    bool isVisible();

private:
    bool expanded = true;   // TODO(zhuangshu): Maybe it's better to storage it to ItemInfo, do it later
    bool visible = true;
};

DPSIDEBAR_END_NAMESPACE

Q_DECLARE_METATYPE(DPSIDEBAR_NAMESPACE::SideBarItemSeparator)

#endif   //DFMSIDEBARITEM_H
