/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef CANVASMANAGERHOOKINTERFACE_H
#define CANVASMANAGERHOOKINTERFACE_H

#include "ddplugin_canvas_global.h"

#include <QString>

namespace ddplugin_canvas {

class CanvasManagerHookInterface
{
public:
    explicit CanvasManagerHookInterface();
    virtual ~CanvasManagerHookInterface();

public:
    // signals
    virtual void iconSizeChanged(int level) const;
    virtual void hiddenFlagChanged(bool hidden) const;
    virtual void autoArrangeChanged(bool on) const;
    virtual bool requestWallpaperSetting(const QString &screen) const;
};

}

#endif // CANVASMANAGERHOOKINTERFACE_H
