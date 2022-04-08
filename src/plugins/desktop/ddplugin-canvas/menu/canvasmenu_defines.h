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
#ifndef CANVASMENU_DEFINES_H
#define CANVASMENU_DEFINES_H

#include "ddplugin_canvas_global.h"

DDP_CANVAS_BEGIN_NAMESPACE

namespace ActionID {
static constexpr char kSortBy[] = "sort-by";
static constexpr char kIconSize[] = "icon-size";
static constexpr char kAutoArrange[] = "auto-arrange";
static constexpr char kDisplaySettings[] = "display-settings";
static constexpr char kWallpaperSettings[] = "wallpaper-settings";

// sort by
static constexpr char kSrtName[] = "sort-by-name";
static constexpr char kSrtTimeModified[] = "sort-by-time-modified";
static constexpr char kSrtSize[] = "sort-by-size";
static constexpr char kSrtType[] = "sort-by-type";
}

namespace CanvasMenuParams {
static constexpr char kDesktopGridPos[] = "DesktopGridPos";
}

DDP_CANVAS_END_NAMESPACE

#endif   // CANVASMENU_DEFINES_H
