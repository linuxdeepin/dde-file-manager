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
#ifndef SCREENPLUGIN_H
#define SCREENPLUGIN_H

#include "ddplugin_screen_global.h"

#include <interfaces/screen/abstractscreenproxy.h>

#include <dfm-framework/framework.h>

DDP_SCREEN_BEGIN_NAMESPACE

class ScreenPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "screen.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;
private:
    DFMBASE_NAMESPACE::AbstractScreenProxy *proxy = nullptr;
};

DDP_SCREEN_END_NAMESPACE

#endif // SCREENPLUGIN_H
