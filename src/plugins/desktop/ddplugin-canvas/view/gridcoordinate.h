// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
