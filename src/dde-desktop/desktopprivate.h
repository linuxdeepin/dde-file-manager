/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
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

#ifndef DESKTOPPRIVATE_H
#define DESKTOPPRIVATE_H

#include "../dde-wallpaper-chooser/frame.h"
#include "../dde-wallpaper-chooser/utils/autoactivatewindow.h"


#ifndef DISABLE_ZONE
#include "../dde-zone/mainwindow.h"
#endif
#include <malloc.h>

using WallpaperSettings = Frame;

#ifndef DISABLE_ZONE
using ZoneSettings = ZoneMainWindow;
#endif

class BackgroundManager;
class CanvasViewManager;
class DesktopPrivate
{
public:
    ~DesktopPrivate()
    {
        if (m_background) {
            delete m_background;
            m_background = nullptr;
        }

        if (m_canvas) {
            delete m_canvas;
            m_canvas = nullptr;
        }

        if (wallpaperSettings) {
            delete wallpaperSettings;
            wallpaperSettings = nullptr;
        }
#ifndef DISABLE_ZONE
        if (zoneSettings) {
            delete zoneSettings;
            zoneSettings = nullptr;
        }
#endif
    }
    BackgroundManager *m_background = nullptr;
    CanvasViewManager *m_canvas = nullptr;
    WallpaperSettings *wallpaperSettings = nullptr;

#ifndef DISABLE_ZONE
    ZoneSettings *zoneSettings { nullptr };
#endif
};
#endif // DESKTOPPRIVATE_H
