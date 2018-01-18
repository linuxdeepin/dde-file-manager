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

#include "dbookmarkrootitem.h"
#include "dbookmarkscene.h"
#include "dbookmarkitem.h"

#include "app/define.h"
#include "app/filesignalmanager.h"

#include "controllers/bookmarkmanager.h"
#include "dfmevent.h"

#include <QDebug>
#include <QMimeData>
#include <QDir>

DBookmarkRootItem::DBookmarkRootItem(DBookmarkScene *scene)
{
    m_scene = scene;
    setBackgroundEnable(false);
    setHoverBackgroundEnable(false);
    setCheckable(false);
//    setPressBackgroundColor(QColor(0,0,0,0));
    setDefaultItem(true);
    setAcceptDrops(true);
    setAcceptHoverEvents(false);
    setObjectName("BMRootItem");
}

void DBookmarkRootItem::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    event->accept();
}


void DBookmarkRootItem::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
    emit m_scene->dragLeft();
}

void DBookmarkRootItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
}
