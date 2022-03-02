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
#ifndef CANVASPLUGIN_H
#define CANVASPLUGIN_H

#include "ddplugin_canvas_global.h"

#include <dfm-framework/framework.h>

DDP_CANVAS_BEGIN_NAMESPACE
class CanvasManager;
class CanvasPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "canvas.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;
private:
    CanvasManager *proxy = nullptr;
};

DDP_CANVAS_END_NAMESPACE

#endif // CANVASPLUGIN_H
