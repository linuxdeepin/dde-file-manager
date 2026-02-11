// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

void CanvasManagerHook::fontChanged() const
{
    CanvasManagerPublish(signal_CanvasManager_FontChanged);
}

void CanvasManagerHook::autoArrangeChanged(bool on) const
{
    CanvasManagerPublish(signal_CanvasManager_AutoArrangeChanged, on);
}
