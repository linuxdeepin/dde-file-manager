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
#include "backgroundplugin.h"

#include <services/desktop/background/backgroundservice.h>

#include <dfm-framework/listener/listener.h>

DDP_BACKGROUND_USE_NAMESPACE
DPF_USE_NAMESPACE
DSB_D_USE_NAMESPACE

void BackgroundPlugin::initialize()
{

}

bool BackgroundPlugin::start()
{
    auto &ctx = dpfInstance.serviceContext();
    // start background service.
    QString error;
    bool ret = ctx.load(BackgroundService::name(), &error);
    Q_ASSERT_X(ret, "BackgroundPlugin", error.toStdString().c_str());

    backgroundManager = new BackgroundManager;
    backgroundManager->init();

    return true;
}

dpf::Plugin::ShutdownFlag BackgroundPlugin::stop()
{
    delete backgroundManager;
    backgroundManager = nullptr;

    return kSync;
}
