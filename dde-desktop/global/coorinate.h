/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

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

    Coordinate(QPoint pos)
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
