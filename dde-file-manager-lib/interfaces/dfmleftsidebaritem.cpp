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
#include "dfmleftsidebaritem.h"

#include "singleton.h"
#include "dfileservices.h"
#include "app/define.h"
#include "controllers/pathmanager.h"

#include <QObject>
#include <QVariant>

DFMLeftSideBarItem::DFMLeftSideBarItem(const QIcon &icon, const QString &text, const DUrl &url, const QString &groupName)
    : QStandardItem (icon, text)
{
    setUrl(url);
    this->setData(SidebarItem, ItemTypeRole);
    this->setData(groupName, ItemGroupNameRole);
    this->setData(ChangeDirectory, ItemCdActionRole);
}

DFMLeftSideBarItem *DFMLeftSideBarItem::createSystemPathItem(const QString &pathKey, const QString &groupName)
{
    QString iconName = systemPathManager->getSystemPathIconName(pathKey);
    if (!iconName.contains("-symbolic")) {
        iconName.append("-symbolic");
    }
    DFMLeftSideBarItem * item = new DFMLeftSideBarItem(
                    QIcon::fromTheme(iconName),
                    systemPathManager->getSystemPathDisplayName(pathKey),
                    DUrl::fromUserInput(systemPathManager->getSystemPath(pathKey)),
                    groupName
                );

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    qDebug() << item->flags();

    return item;
}

DFMLeftSideBarItem *DFMLeftSideBarItem::createDeviceItem(const DUrl &url, const QString &groupName)
{
    const DAbstractFileInfoPointer infoPointer = DFileService::instance()->createFileInfo(nullptr, url);
    QVariantHash info = infoPointer->extraProperties();
    QString displayName = infoPointer->fileDisplayName();
    QString iconName("drive-harddisk");

    DFMLeftSideBarItem * item = new DFMLeftSideBarItem(QIcon::fromTheme(iconName), displayName, url, groupName);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    item->setData(MountPartitionThenCd, ItemCdActionRole);

    return item;
}

DFMLeftSideBarItem *DFMLeftSideBarItem::createBookmarkItem(const DUrl &url, const QString &groupName)
{
    // leave url a default display name.
    DAbstractFileInfoPointer fileInfo = DFileService::instance()->createFileInfo(nullptr, url);
    QString displayName;

    if (fileInfo) {
        displayName = fileInfo->fileDisplayName();
    }

    DFMLeftSideBarItem * item = new DFMLeftSideBarItem(
                    QIcon::fromTheme("folder-bookmark-symbolic"), displayName, url, groupName
                );

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);

    return item;
}

DFMLeftSideBarItem *DFMLeftSideBarItem::createTagItem(const DUrl &url, const QString &groupName)
{
    DFMLeftSideBarItem * item = new DFMLeftSideBarItem(QIcon(), url.fileName(), url, groupName);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren);

    return item;
}

/*!
 * \internal
 * \brief DFMLeftSideBarItem::createSeparatorItem
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
DFMLeftSideBarItem * DFMLeftSideBarItem::createSeparatorItem(const QString &groupName)
{
    DFMLeftSideBarItem * item = new DFMLeftSideBarItem(DUrl(), groupName);

    item->setData(Separator, ItemTypeRole);
    item->setFlags(Qt::NoItemFlags);

    return item;
}

DUrl DFMLeftSideBarItem::url() const
{
    return this->data(ItemUrlRole).value<DUrl>();
}

QString DFMLeftSideBarItem::groupName() const
{
    return this->data(ItemGroupNameRole).toString();
}

int DFMLeftSideBarItem::cdActionType() const
{
    return this->data(ItemCdActionRole).toInt();
}

int DFMLeftSideBarItem::itemType() const
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
int DFMLeftSideBarItem::type() const
{
    return DFMLeftSideBarItem::SidebarItemType;
}

void DFMLeftSideBarItem::setUrl(const DUrl &url)
{
    this->setData(QVariant::fromValue(url), ItemUrlRole);
}

