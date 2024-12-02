// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidebaritem.h"
#include "sidebarmodel.h"
#include "utils/sidebarinfocachemananger.h"

#include <dfm-base/base/standardpaths.h>

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

    setData(kSidebarItem, kItemTypeRole);
}

SideBarItem::SideBarItem(const QIcon &icon, const QString &text, const QString &group, const QUrl &url)
    : DStandardItem(icon, text)
{
    setIcon(icon);
    setText(text);
    setGroup(group);
    setUrl(url);

    setData(kSidebarItem, kItemTypeRole);
}

QUrl SideBarItem::url() const
{
    return this->data(kItemUrlRole).value<QUrl>();
}

QUrl SideBarItem::targetUrl() const
{
    QUrl targetItemUrl;
    if (!itemInfo().finalUrl.isEmpty())
        targetItemUrl = itemInfo().finalUrl;
    else
        targetItemUrl = url();
    return targetItemUrl;
}

void SideBarItem::setUrl(const QUrl &url)
{
    setData(QVariant::fromValue(url), kItemUrlRole);
}

void SideBarItem::setGroup(const QString &group)
{
    setData(group, Roles::kItemGroupRole);
}

bool SideBarItem::isHidden() const
{
    return data(Roles::kItemGroupRole).toBool();
}

void SideBarItem::setHiiden(bool hidden)
{
    setData(hidden, Roles::kItemHiddenRole);
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
    return data(Roles::kItemGroupRole).toString();
}

SideBarItemSeparator::SideBarItemSeparator(const QString &group)
    : SideBarItem(QUrl())
{
    setGroup(group);
    setText(group);

    setData(kSeparator, kItemTypeRole);
}

SideBarItemSeparator::~SideBarItemSeparator()
{
}

void SideBarItemSeparator::setExpanded(bool state)
{
    expanded = state;
}

bool SideBarItemSeparator::isExpanded()
{
    return expanded;
}

void SideBarItemSeparator::setVisible(bool value)
{
    visible = value;
}

bool SideBarItemSeparator::isVisible()
{
    return visible;
}
