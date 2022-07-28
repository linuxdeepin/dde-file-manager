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
#include "canvasmanagerhook.h"

#include <dfm-framework/dpf.h>

using namespace ddplugin_canvas;

#define CanvasManagerPublish(topic, args...) \
            dpfSignalDispatcher->publish(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic), ##args)

#define CanvasManagerRunHook(topic, args...) \
            dpfHookSequence->run(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic), ##args)

CanvasManagerHook::CanvasManagerHook(QObject *parent)
    : QObject(parent)
    , CanvasManagerHookInterface()
{

}

bool CanvasManagerHook::requestWallpaperSetting(const QString &screen) const
{
    return CanvasManagerRunHook(hook_CanvasManager_RequestWallpaperSetting, screen);
}

void CanvasManagerHook::iconSizeChanged(int level) const
{
    CanvasManagerPublish(signal_CanvasManager_IconSizeChanged, level);
}

void CanvasManagerHook::hiddenFlagChanged(bool hidden) const
{
    CanvasManagerPublish(signal_CanvasManager_HiddenFlagChanged, hidden);
}
