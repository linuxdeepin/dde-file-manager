/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef BACKGROUNDPLUGIN_H
#define BACKGROUNDPLUGIN_H

#include "ddplugin_background_global.h"
#include "backgroundmanager.h"

#include <dfm-framework/dpf.h>

DDP_BACKGROUND_BEGIN_NAMESPACE

class BackgroundPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "background.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

private:
    BackgroundManager *backgroundManager = nullptr;
};

DDP_BACKGROUND_END_NAMESPACE

#endif   // BACKGROUNDPLUGIN_H
