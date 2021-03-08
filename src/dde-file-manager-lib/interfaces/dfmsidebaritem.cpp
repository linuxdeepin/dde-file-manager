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

#include "dfmsidebaritem.h"
#include "dfmsidebarmanager.h"

#include <QObject>
#include <QVariant>
#include <QUuid>

DFM_BEGIN_NAMESPACE

DFMSideBarItem::DFMSideBarItem(const QIcon &icon, const QString &text, const DUrl &url, const QString &groupName)
    : DStandardItem (icon, text)
{
    setUrl(url);
    setGroupName(groupName);
    this->setData(SidebarItem, ItemTypeRole);
    this->setData("none", ItemUseRegisteredHandlerRole);
    // setToolTip(text);
    this->setData(QUuid::createUuid().toString(), ItemUniqueKeyRole);
    this->setData(url.path().remove("/").remove(".localdisk"), ItemVolTagRole);
}

/*!
 * \internal
 * \brief DFMSideBarItem::createSeparatorItem
 *
 * This is more likely a group header for the group \a groupName .
 * Since QListView itself doesn't support grouping, so we need draw something as an
 * identifier of the group. This separator located act as "first item of the group
 * but not clickable", and once we found the group is empty, we should hide it.
 *
 * We should not remove a separator unless we actually would like to remove a group.
 * For add/insert/append item to a group, we need find the correct group location, if
 * there are no item in the group, the separator is the "placeholder" of the group.
 */
DFMSideBarItem * DFMSideBarItem::createSeparatorItem(const QString &groupName)
{
    DFMSideBarItem * item = new DFMSideBarItem(DUrl(), groupName);

    item->setData(Separator, ItemTypeRole);
    item->setFlags(Qt::NoItemFlags);

    return item;
}

DUrl DFMSideBarItem::url() const
{
    return this->data(ItemUrlRole).value<DUrl>();
}

QString DFMSideBarItem::groupName() const
{
    return this->data(ItemGroupNameRole).toString();
}

int DFMSideBarItem::itemType() const
{
    return this->data(ItemTypeRole).toInt();
}

/*!
 * \brief Returns the type of this item.
 *
 * The type is used to distinguish custom items from the base class, don't confused
 * with the Roles::ItemTypeRole data role, they are NOT the same thing, for that purpose
 * please use itemType() instead.
 *
 * \sa itemType()
 */
int DFMSideBarItem::type() const
{
    return DFMSideBarItem::SidebarItemType;
}

bool DFMSideBarItem::useRegisteredHandler() const
{
    return this->registeredHandler() == "none";
}

QString DFMSideBarItem::registeredHandler(const QString &fallback) const
{
    QString identifier = this->data(ItemUseRegisteredHandlerRole).toString();
    if (identifier != "none") {
        return identifier;
    } else {
        return fallback.isEmpty() ? identifier : fallback;
    }
}

void DFMSideBarItem::setUrl(const DUrl &url)
{
    this->setData(QVariant::fromValue(url), ItemUrlRole);
}

void DFMSideBarItem::setGroupName(const QString &groupName)
{
    this->setData(groupName, ItemGroupNameRole);
}

void DFMSideBarItem::setRegisteredHandler(const QString &identifier)
{
    this->setData(identifier, ItemUseRegisteredHandlerRole);
}

DFM_END_NAMESPACE
