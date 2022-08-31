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
    void hiddenFlagChanged(bool hidden) const override;
    void autoArrangeChanged(bool on) const override;
    bool requestWallpaperSetting(const QString &screen) const override;
};

}

#endif // CANVASMANAGERHOOK_H
