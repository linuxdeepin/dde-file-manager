// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASMANAGERHOOK_H
#define CANVASMANAGERHOOK_H

#include "private/canvasmanagerhookinterface.h"

#include <QObject>

namespace ddplugin_canvas {

class CanvasManagerHook : public QObject, public CanvasManagerHookInterface
{
    Q_OBJECT
public:
    explicit CanvasManagerHook(QObject *parent = nullptr);
public:
    void iconSizeChanged(int level) const override;
    void fontChanged() const override;
    void autoArrangeChanged(bool on) const override;
    bool requestWallpaperSetting(const QString &screen) const override;
};

}

#endif // CANVASMANAGERHOOK_H
