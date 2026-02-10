// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    virtual void fontChanged() const;
    virtual void autoArrangeChanged(bool on) const;
    virtual bool requestWallpaperSetting(const QString &screen) const;
};

}

#endif // CANVASMANAGERHOOKINTERFACE_H
