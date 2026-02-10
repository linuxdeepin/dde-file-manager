// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gridcoordinate.h"

using namespace ddplugin_canvas;

GridCoordinate::GridCoordinate(int x, int y)
    : xp(x)
    , yp(y)
{

}

GridCoordinate::GridCoordinate(QPoint pos)
    : xp(pos.x())
    , yp(pos.y())
{

}

