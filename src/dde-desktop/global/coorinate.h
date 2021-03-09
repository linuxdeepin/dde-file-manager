/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
 *             liqiang<liqianga@uniontech.com>
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

#pragma once

#include <QtGlobal>
#include <QPoint>

class Coordinate
{
public:
    Coordinate(int _x, int _y)
    {
        d.x = _x;
        d.y = _y;
    }

    explicit Coordinate(QPoint pos)
    {
        d.x = pos.x();
        d.y = pos.y();
    }

    QPoint position()
    {
        return QPoint(d.x, d.y);
    }

    Coordinate moveLeft(int offset = 1) const
    {
        return Coordinate(d.x - offset, d.y);
    }

    Coordinate moveRight(int offset = 1) const
    {
        return Coordinate(d.x + offset, d.y);
    }

    Coordinate moveUp(int offset = 1) const
    {
        return Coordinate(d.x, d.y - offset);
    }

    Coordinate moveDown(int offset = 1) const
    {
        return Coordinate(d.x, d.y + offset);
    }

private:
    union CoordinateData {
        struct { /*Position*/
            int32_t x;
            int32_t y;
        };
//        CoordValue value;
    };

    CoordinateData  d;
};
