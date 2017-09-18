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

#include "dbookmarkline.h"

DBookmarkLine::DBookmarkLine()
{
    setDefaultItem(true);
    setAcceptDrops(false);
    setObjectName("DBookmarkLine");
}

void DBookmarkLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    double w = boundWidth();
    double h = boundHeight();
    painter->setPen(QColor(0, 0, 0, 25));
    if(!isTightModel())
        painter->drawLine(0, h/2 , w, h/2);
    else
        painter->drawLine(0, h/2 , 60, h/2);
}
