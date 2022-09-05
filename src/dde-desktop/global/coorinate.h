// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
