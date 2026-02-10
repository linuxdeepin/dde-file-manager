// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasmanagerhookinterface.h"

using namespace ddplugin_canvas;

CanvasManagerHookInterface::CanvasManagerHookInterface()
{

}

CanvasManagerHookInterface::~CanvasManagerHookInterface()
{

}

bool CanvasManagerHookInterface::requestWallpaperSetting(const QString &screen) const
{
    return false;
}

void CanvasManagerHookInterface::iconSizeChanged(int level) const
{
    return;
}

void CanvasManagerHookInterface::fontChanged() const
{
    return;
}

void CanvasManagerHookInterface::autoArrangeChanged(bool on) const
{
    return;
}
