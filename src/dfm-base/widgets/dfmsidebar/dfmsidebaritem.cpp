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
#include "dfmsidebaritem.h"
#include "dfmsidebarmanager.h"
#include "base/dfmstandardpaths.h"

#include <QObject>
#include <QVariant>

DFMSideBarItem::DFMSideBarItem(const QUrl &url)
    : DFMSideBarItem (QIcon(), QString(), QString(), url)
{

}

DFMSideBarItem::~DFMSideBarItem()
{

}

DFMSideBarItem::DFMSideBarItem(const DFMSideBarItem &item) {
    setUrl(item.url());
    setGroup(item.group());
    setIcon(item.icon());
    setText(item.text());
}

DFMSideBarItem::DFMSideBarItem(const QIcon &icon, const QString &text, const QString &group, const QUrl &url)
{
    setIcon(icon);
    setText(text);
    setGroup(group);
    setUrl(url);
}

QUrl DFMSideBarItem::url() const
{
    return this->data(ItemUrlRole).value<QUrl>();
}

void DFMSideBarItem::setUrl(const QUrl &url)
{
    this->setData(QVariant::fromValue(url), ItemUrlRole);
}

void DFMSideBarItem::setGroup(const QString &group)
{
    setData(group, Roles::ItemGroupRole);
}

QString DFMSideBarItem::group() const
{
    return data(Roles::ItemGroupRole).toString();
}

DFMSideBarItemSeparator::DFMSideBarItemSeparator():
    DFMSideBarItem(QUrl())
{

}

DFMSideBarItemSeparator::~DFMSideBarItemSeparator()
{

}
