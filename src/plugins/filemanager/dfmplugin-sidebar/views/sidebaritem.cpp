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
#include "sidebaritem.h"

#include "dfm-base/base/standardpaths.h"

#include <QObject>
#include <QVariant>

DPSIDEBAR_USE_NAMESPACE

SideBarItem::SideBarItem(const QUrl &url)
    : SideBarItem(QIcon(), QString(), QString(), url)
{
}

SideBarItem::~SideBarItem()
{
}

SideBarItem::SideBarItem(const SideBarItem &item)
{
    setUrl(item.url());
    setGroup(item.group());
    setIcon(item.icon());
    setText(item.text());
}

SideBarItem::SideBarItem(const QIcon &icon, const QString &text, const QString &group, const QUrl &url)
    : DStandardItem(icon, text)
{
    setIcon(icon);
    setText(text);
    setGroup(group);
    setUrl(url);
}

QUrl SideBarItem::url() const
{
    return this->data(ItemUrlRole).value<QUrl>();
}

void SideBarItem::setUrl(const QUrl &url)
{
    this->setData(QVariant::fromValue(url), ItemUrlRole);
}

void SideBarItem::setGroup(const QString &group)
{
    setData(group, Roles::ItemGroupRole);
}

void SideBarItem::setItemInfo(DSB_FM_NAMESPACE::SideBar::ItemInfo info)
{
    this->info = info;
}

DSB_FM_NAMESPACE::SideBar::ItemInfo SideBarItem::itemInfo() const
{
    return this->info;
}

QString SideBarItem::group() const
{
    return data(Roles::ItemGroupRole).toString();
}

SideBarItemSeparator::SideBarItemSeparator(const QString &group)
    : SideBarItem(QUrl())
{
    setGroup(group);
}

SideBarItemSeparator::~SideBarItemSeparator()
{
}
