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
#ifndef CANVASMANAGEREXTENDINTERFACE_H
#define CANVASMANAGEREXTENDINTERFACE_H

#include "ddplugin_canvas_global.h"

#include <QString>

DDP_CANVAS_BEGIN_NAMESPACE

class CanvasManagerExtendInterface
{
public:
    explicit CanvasManagerExtendInterface();
    virtual ~CanvasManagerExtendInterface();
public:
public:
    // signals
    virtual void requestWallpaperSetting(const QString &screen) const;
};

DDP_CANVAS_END_NAMESPACE

#endif // CANVASMANAGEREXTENDINTERFACE_H
