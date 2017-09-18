/*
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

#include "dbookmarkitemgroup.h"
#include "dbookmarkitem.h"

DBookmarkItemGroup::DBookmarkItemGroup()
{

}

void DBookmarkItemGroup::addItem(DBookmarkItem *item)
{
    m_items.append(item);
    item->setItemGroup(this);
}

void DBookmarkItemGroup::insert(int index, DBookmarkItem *item)
{
    m_items.insert(index, item);
    item->setItemGroup(this);
}

void DBookmarkItemGroup::removeItem(DBookmarkItem *item)
{
    m_items.removeOne(item);
}

void DBookmarkItemGroup::deselectAll()
{
    for(int i = 0; i < m_items.size(); i++)
    {
        m_items.at(i)->setChecked(false);
        if (m_items.at(i)->isDiskItem()){
            m_items.at(i)->setHighlightDisk(false);
        }
    }
}

QList<DBookmarkItem *> DBookmarkItemGroup::items() const
{
    return m_items;
}
