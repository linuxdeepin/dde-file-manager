/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#include "wallpaperservice.h"
#include "wallpapersettings.h"
#include "private/autoactivatewindow.h"
#include "screen/screenservice.h"
#include "background/services/backgroundservice.h"
#include "dfm-base/widgets/abstractscreen.h"

DSB_D_USE_NAMESPACE

WallpaperService::WallpaperService(QObject *parent)
    : PluginService(parent)
    , AutoServiceRegister<WallpaperService>()
{

}

WallpaperService::~WallpaperService()
{
    delete wallpaperSettings;
    wallpaperSettings = nullptr;
}

void WallpaperService::init()
{

}

void WallpaperService::wallpaperSetting(QString name)
{
    show(name, (int)WallpaperSettings::Mode::WallpaperMode);
}

void WallpaperService::screenSaverSetting(QString name)
{
    show(name, (int)WallpaperSettings::Mode::ScreenSaverMode);
}

void WallpaperService::onQuit()
{
    if (wallpaperSettings) {
        wallpaperSettings->deleteLater();
        wallpaperSettings = nullptr;
    }
}

void WallpaperService::onChanged()
{
    if (!wallpaperSettings)
        return;

    auto &ctx = dpfInstance.serviceContext();
    if (auto service = ctx.service<BackgroundService>(BackgroundService::name())) {
        auto wallpaper = wallpaperSettings->currentWallpaper();
        service->setBackgroundPath(wallpaper.first, wallpaper.second);
    }
}

void WallpaperService::show(QString name, int mode)
{
    auto &ctx = dpfInstance.serviceContext();
    auto screenScevice = ctx.service<ScreenService>(ScreenService::name());
    if (!screenScevice) {
        qCritical() << "can not get ScreenService.";
        return;
    }

    if (name.isNull() || name.isEmpty()) {
        if (screenScevice->primaryScreen() == nullptr) {
            qCritical() << "get primary screen failed! stop show wallpaper";
            return;
        }

        name = screenScevice->primaryScreen()->name();
    }

    if (wallpaperSettings) {
        //防止暴力操作，高频调用接口
        if (wallpaperSettings->isVisible())
            return;
        wallpaperSettings->deleteLater();
        wallpaperSettings = nullptr;
    }

    wallpaperSettings = new WallpaperSettings(name, WallpaperSettings::Mode(mode));
    connect(wallpaperSettings, &WallpaperSettings::quit, this, &WallpaperService::onQuit);
    connect(wallpaperSettings, &WallpaperSettings::backgroundChanged, this, &WallpaperService::onChanged);

    wallpaperSettings->show();
    wallpaperSettings->activateWindow();
    wallpaperSettings->refreshList();

    // auto focus tool
    auto autoAct = new AutoActivateWindow(wallpaperSettings);
    autoAct->setWatched(wallpaperSettings);
    autoAct->start();
}
