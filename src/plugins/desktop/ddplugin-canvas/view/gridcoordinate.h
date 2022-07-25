/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef GRIDCOORDINATE_H
#define GRIDCOORDINATE_H

#include "ddplugin_canvas_global.h"

#include <QPoint>

namespace ddplugin_canvas {

class GridCoordinate
{
public:
    explicit GridCoordinate(int x, int y);
    explicit GridCoordinate(QPoint pos);
public:
    inline int x() const {
        return xp;
    }

    inline int y() const{
        return yp;
    }

    inline QPoint point() const {
        return QPoint(xp, yp);
    }

    inline GridCoordinate moveLeft(int offset = 1) const
    {
        return GridCoordinate(xp - offset, yp);
    }

    inline GridCoordinate moveRight(int offset = 1) const
    {
        return GridCoordinate(xp + offset, yp);
    }

    inline GridCoordinate moveUp(int offset = 1) const
    {
        return GridCoordinate(xp, yp - offset);
    }

    inline GridCoordinate moveDown(int offset = 1) const
    {
        return GridCoordinate(xp, yp + offset);
    }

protected:
    int xp;
    int yp;
};

}

#endif // GRIDCOORDINATE_H
