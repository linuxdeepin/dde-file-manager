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

#ifndef DBOOKMARKROOTITEM_H
#define DBOOKMARKROOTITEM_H

#include "dbookmarkitem.h"

#define DEFAULT_ITEM_COUNT 11

class DBookmarkScene;

class DBookmarkRootItem : public DBookmarkItem
{
public:
    DBookmarkRootItem(DBookmarkScene * scene);
protected:
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
private:
    DBookmarkScene * m_scene;
};

#endif // DBOOKMARKROOTITEM_H
