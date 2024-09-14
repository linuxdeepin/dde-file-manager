// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebaritem.h"
#include "models/sidebarmodel.h"
#include "utils/sidebarinfocachemananger.h"

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

QString SideBarItem::subGourp() const
{
    return itemInfo().subGroup;
}

ItemInfo SideBarItem::itemInfo() const
{
    // if this is storage as a member variable, click item after dragged, dfm crash.
    return SideBarInfoCacheMananger::instance()->itemInfo(url());
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
