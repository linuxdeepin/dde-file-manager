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

#include "dfmsidebaritem.h"
#include "dfmsidebaritemgroup.h"

#include "singleton.h"

#include <QDebug>

DFM_BEGIN_NAMESPACE

DFMSideBarItemGroup::DFMSideBarItemGroup()
{
    setSpacing(0);
    itemHolder = new QVBoxLayout();
    bottomSeparator = new DFMSideBarItemSeparator();

    this->addLayout(itemHolder);
    this->addWidget(bottomSeparator);
    bottomSeparator->setVisible(itemList.count() != 0);
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

    bottomSeparator->setVisible(itemList.count() != 0);

    return index; // array start at 0, so this is the right index.
}

/*!
 * \brief Insert \a item before \a index.
 *
 * Insert a `DFMSideBarItem` \a item to group before \a index.
 */
void DFMSideBarItemGroup::insertItem(int index, DFMSideBarItem *item)
{
    itemList.insert(index, item);
    itemHolder->insertWidget(index, item); // FIXME: bottom line issue
}

/*!
 * \brief Remove sidebar item at \a index .
 *
 * Index from 0 to `count() - 1`
 */
void DFMSideBarItemGroup::removeItem(int index)
{
    if (index >= 0 && index < itemList.count()) {
        DFMSideBarItem *item = itemList.takeAt(index);
        itemHolder->removeWidget(item);
        item->deleteLater();
    }
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
    DFMSideBarItem *value = nullptr;

    if (index >= 0 && index < itemList.count()) {
        value = itemList.takeAt(index);
        itemHolder->removeWidget(value);
    }

    return value;
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

DFMSideBarItem *DFMSideBarItemGroup::operator [](int index)
{
    return itemList[index];
}

DFM_END_NAMESPACE
