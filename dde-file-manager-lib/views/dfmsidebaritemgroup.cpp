/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
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

#include "dfmsidebardeviceitem.h"
#include "dfmsidebaritem.h"
#include "dfmsidebaritemgroup.h"
#include "dfmapplication.h"
#include "dfmsettings.h"

#include "singleton.h"

#include "deviceinfo/udisklistener.h"

#include <QDebug>

#define SIDEBAR_ITEMORDER_KEY "SideBar/ItemOrder"

DFM_BEGIN_NAMESPACE

/*!
 * \class DFMSideBarItemGroup
 * \inmodule dde-file-manager-lib
 *
 * \brief DFMSideBarItemGroup is the item group inside DFMSideBar
 *
 * DFMSideBarItemGroup is the item group inside DFMSideBar which holds the sidebar items.
 * all items is managed by DFMSideBarItemGroup. DFMSideBarItem managed all avaliable
 * DFMSideBarItemGroup s' state.
 *
 * \sa DFMSideBar, DFMSideBarItem
 */

DFMSideBarItemGroup::DFMSideBarItemGroup(QString groupName)
{
    setSpacing(0);
    itemHolder = new QVBoxLayout();
    bottomSeparator = new DFMSideBarItemSeparator();

    this->groupName = groupName;
    this->addLayout(itemHolder);
    this->addWidget(bottomSeparator);
    bottomSeparator->setVisible(visibleItemCount() != 0);
}

/*!
 * \brief Get the item's index by reference (pointer)
 * \return index (start at 0), return -1 if the item is not in current group.
 */
int DFMSideBarItemGroup::itemIndex(const DFMSideBarItem *item) const
{
    return itemList.indexOf(const_cast<DFMSideBarItem *>(item));
}

/*!
 * \brief Append a DFMSideBarItem to this sidebar item group.
 */
int DFMSideBarItemGroup::appendItem(DFMSideBarItem *item)
{
    // for better indexing, counting and etc.
    int index = itemList.count();
    itemList.append(item);
    itemHolder->addWidget(item);
    item->setGroupName(groupName);
    itemConnectionRegister(item);

    bottomSeparator->setVisible(visibleItemCount() != 0);

    if (!m_autosort) {
        return index; // array start at 0, so this is the right index.
    }

    sort();
    return itemIndex(item);
}

/*!
 * \brief Insert \a item before \a index.
 *
 * Insert a `DFMSideBarItem` \a item to group before \a index.
 */
void DFMSideBarItemGroup::insertItem(int index, DFMSideBarItem *item)
{
    itemList.insert(index, item);
    itemHolder->insertWidget(index, item);
    item->setGroupName(groupName);
    itemConnectionRegister(item);

    bottomSeparator->setVisible(visibleItemCount() != 0);

    if (m_autosort) {
        sort();
    }
}

/*!
 * \brief Remove sidebar item at \a index .
 *
 * Index from 0 to `count() - 1`
 *
 * Notice: always call `DFMSideBar::removeItem()` instead of this function
 * since that one managed some `DFMSideBar` internal state like `lastCheckedItem`
 */
void DFMSideBarItemGroup::removeItem(int index)
{
    if (index >= 0 && index < itemList.count()) {
        DFMSideBarItem *item = itemList.takeAt(index);
        item->hide();
        itemHolder->removeWidget(item);
        itemConnectionUnregister(item);
        item->deleteLater();

        bottomSeparator->setVisible(visibleItemCount() != 0);
    }
}

/*!
 * \brief Remove \a item from sidebar.
 *
 * Notice: always call `DFMSideBar::removeItem()` instead of this function
 * since that one managed some `DFMSideBar` internal state like `lastCheckedItem`
 */
void DFMSideBarItemGroup::removeItem(DFMSideBarItem *item)
{
    item->hide();
    itemList.removeOne(item);
    itemHolder->removeWidget(item);
    itemConnectionUnregister(item);
    item->deleteLater();

    bottomSeparator->setVisible(visibleItemCount() != 0);
}

DUrlList DFMSideBarItemGroup::itemOrder()
{
    DUrlList list;

    if (m_saveItemOrder) {
        QStringList savedList = DFMApplication::genericSetting()->value(SIDEBAR_ITEMORDER_KEY, groupName).toStringList();
        for (const QString & item : savedList) {
            list << DUrl(item);
        }
    }

    return list;
}

/*!
 * \brief Save item order to config file.
 */
void DFMSideBarItemGroup::saveItemOrder()
{
    if (m_saveItemOrder) {
        QVariantList list;

        for (const DFMSideBarItem* item : itemList) {
            list << QVariant(item->url());
        }

        DFMApplication::genericSetting()->setValue(SIDEBAR_ITEMORDER_KEY, groupName, list);
    }
}

/*!
 * \brief Find item by the given \a url
 * \return the first match item, will return nullptr if not found.
 *
 * This function will find the first item which match the given `DUrl`
 *
 * Notice: Any unmounted volume will got an empty `DUrl`. When processing
 * items in *Device* group, do not use this function to get item.
 */
DFMSideBarItem *DFMSideBarItemGroup::findItem(const DUrl &url)
{
    for (int idx = 0, cnt = itemCount(); idx < cnt; idx++) {
        DFMSideBarItem *item = (*this)[idx];
        if (item->url() == url) {
            return item;
        }
    }

    return nullptr;
}

/*!
 * \fn DFMSideBarItem *DFMSideBarItemGroup::takeItem(int index)
 *
 * \brief Take an item's reference (pointer) from this group and remove it.
 *
 * This function will return a `DFMSideBarItem *` of the given \n index,
 * and remove that item from the sidebar group.
 *
 * Please notice that once the caller take that item, it is the caller's
 * responsibility to delete/free the item.
 *
 * Please notice that it's different with `takeAt()` which will take the
 * real QWidget from the widget list of the group.
 *
 * Index from 0 to `count() - 1`
*/
DFMSideBarItem *DFMSideBarItemGroup::takeItem(int index)
{
    DFMSideBarItem *item = nullptr;

    if (index >= 0 && index < itemList.count()) {
        item = itemList.takeAt(index);
        itemHolder->removeWidget(item);
        item->setGroupName(QString());
        bottomSeparator->setVisible(visibleItemCount() != 0);
    }

    return item;
}

DFMSideBarItem *DFMSideBarItemGroup::takeItem(DFMSideBarItem *item)
{
    itemList.removeOne(item);
    itemHolder->removeWidget(item);
    item->setGroupName(QString());

    bottomSeparator->setVisible(visibleItemCount() != 0);

    return item;
}

/*!
 * \brief Get the `DFMSideBarItem` item count of this group.
 *
 * Please notice that this is different from `count()` which will
 * return the QWidget item count which not only include the sidebar
 * items (`DFMSideBarItem`).
 */
int DFMSideBarItemGroup::itemCount() const
{
    return itemList.count();
}

/*!
 * \brief Get the visible `DFMSideBarItem` item count of this group.
 *
 * Item can be hidden when calling DFMSideBar::setDisableUrlSchemes .
 *
 * \sa setDisableUrlSchemes
 */
int DFMSideBarItemGroup::visibleItemCount() const
{
    int result = 0;

    for (const DFMSideBarItem *item: itemList) {
        if (item->itemVisible()) {
            result++;
        }
    }

    return result;
}

/*!
 * \brief When item order changed, save the item order or not?
 *
 * \param saveItemOrder Save item order or not.
 */
void DFMSideBarItemGroup::setSaveItemOrder(bool saveItemOrder)
{
    m_saveItemOrder = saveItemOrder;
}

/*!
 * \brief Enable or disable automatic sorting.
 *
 * If enabled, items are sorted based on the priority they return.
 *
 * \param autoSort Whether automatic sorting is enabled.
 */
void DFMSideBarItemGroup::setAutoSort(bool autoSort)
{
    m_autosort = autoSort;
}

/*!
 * \brief Sort the items in the container according to return value of
 *        sortingPriority().
 *
 * Items with equal sorting priority retain their relative order.
 */
void DFMSideBarItemGroup::sort()
{
    std::vector<DFMSideBarItem*> items;
    for(DFMSideBarItem* i; (i = takeItem(0)) != nullptr; ) {
        items.push_back(i);
    }
    std::stable_sort(items.begin(), items.end(),
                     [](const DFMSideBarItem* a, const DFMSideBarItem* b) {
                         return a->sortingPriority() < b->sortingPriority();
                     }
    );

    for (DFMSideBarItem* i : items) {
        itemList.append(i);
        itemHolder->addWidget(i);
        i->setGroupName(groupName);
        itemConnectionRegister(i);
    }

    bottomSeparator->setVisible(!items.empty());
}

/*!
 * \brief Hide sidebar items by given url \a schemes .
 *
 * Notice that this is for *HIDE* the items, NOT for display a *DISABLED* state.
 *
 * \sa visibleItemCount, DFMSideBar::setDisableUrlSchemes
 */
void DFMSideBarItemGroup::setDisableUrlSchemes(const QSet<QString> &schemes)
{
    for (DFMSideBarItem *item : itemList) {
        item->setVisible(!schemes.contains(item->url().scheme()));
    }

    bottomSeparator->setVisible(visibleItemCount() != 0);
}

DFMSideBarItem *DFMSideBarItemGroup::operator [](int index)
{
    return itemList[index];
}

void DFMSideBarItemGroup::reorderItem(DFMSideBarItem *ori, DFMSideBarItem *dst, bool insertBefore)
{
    int oldIndex = itemIndex(ori);
    // since insertItem only provided an `index` argument to insert, we should take it first.
    takeItem(ori);
    int dstIndex = itemIndex(dst);
    if (insertBefore) {
        insertItem(dstIndex, ori);
    } else {
        if (dstIndex >= itemCount()) {
            appendItem(ori);
        } else {
            insertItem(dstIndex + 1, ori);
        }
    }
    int newIndex = itemIndex(ori);

    saveItemOrder();

    emit itemReordered(oldIndex, newIndex, ori);
}

void DFMSideBarItemGroup::itemConnectionRegister(DFMSideBarItem *item)
{
    connect(item, SIGNAL(reorder(DFMSideBarItem *, DFMSideBarItem *, bool)),
            this, SLOT(reorderItem(DFMSideBarItem *, DFMSideBarItem *, bool)));
    connect(item, &DFMSideBarItem::urlChanged, this, [this]() {
        saveItemOrder();
    });
    connect(item, &DFMSideBarItem::itemDragReleased, this, [this](QPoint dropPos, Qt::DropAction action) {
        DFMSideBarItem * item = qobject_cast<DFMSideBarItem *>(QObject::sender());
        emit itemDragRelease(dropPos, action, item);
    });
}

void DFMSideBarItemGroup::itemConnectionUnregister(DFMSideBarItem *item)
{
    item->disconnect();
}

DFM_END_NAMESPACE
