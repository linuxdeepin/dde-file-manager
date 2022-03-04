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
#include "wlsetplugin.h"
#include "wallpapersettings.h"
#include "private/autoactivatewindow.h"

#include <dfm-base/utils/universalutils.h>

#include "services/desktop/event/private/eventhelperfunc.h"
#include <services/desktop/wallpapersetting/wallpaperservice.h>
#include <services/desktop/screen/screenservice.h>


DDP_WALLPAERSETTING_USE_NAMESPACE
DSB_D_USE_NAMESPACE

void WlSetPlugin::initialize()
{
}

bool WlSetPlugin::start()
{
    handle = new EventHandle();

    auto &ctx = dpfInstance.serviceContext();
    // start service.
    {
        QString error;
        bool ret = ctx.load(WallpaperService::name(), &error);
        Q_ASSERT_X(ret, "WallpaperSettingPlugin", error.toStdString().c_str());
    }

    auto service = ctx.service<WallpaperService>(WallpaperService::name());
    Q_ASSERT_X(service, "WallpaperSettingPlugin", "ScreenService not found");

    // register event
    service->registerEvent(handle);

    // register slots
    {
        auto eslots = handle->query(EventType::kEventSlot);
        int showWallpaper = EventHelperFunc::getEventID(eslots, kEventWallpaperSetting);
        if (showWallpaper > 0)
            dpfInstance.eventDispatcher().subscribe(showWallpaper,
                                                    handle,
                                                    &EventHandle::wallpaperSetting);
        else
            qWarning() << "Fail to get EventShowWallpaper";

        int showScreenSaver = EventHelperFunc::getEventID(eslots, kEventScreenSaverSetting);
        if (showScreenSaver > 0)
            dpfInstance.eventDispatcher().subscribe(showScreenSaver,
                                                    handle,
                                                    &EventHandle::screenSaverSetting);
        else
            qWarning() << "Fail to get EventShowScreenSaver";
    }

    return true;
}

dpf::Plugin::ShutdownFlag WlSetPlugin::stop()
{
    return kSync;
}

EventHandle::EventHandle(QObject *parent)
    : QObject(parent)
    , EventProvider()
{
    eSignals.insert(kEventWallpaperChanged,
                    DFMBASE_NAMESPACE::UniversalUtils::registerEventType());
    eSlots.insert(kEventWallpaperSetting,
                    DFMBASE_NAMESPACE::UniversalUtils::registerEventType());
    eSlots.insert(kEventScreenSaverSetting,
                  DFMBASE_NAMESPACE::UniversalUtils::registerEventType());
}

QVariantHash EventHandle::query(int type) const
{
    if (type == EventType::kEventSignal)
        return eSignals;
    else if (type == EventType::kEventSlot)
        return eSlots;

    return {};
}

void EventHandle::wallpaperSetting(QString name)
{
    show(name, (int)WallpaperSettings::Mode::WallpaperMode);
}

void EventHandle::screenSaverSetting(QString name)
{
    show(name, (int)WallpaperSettings::Mode::ScreenSaverMode);
}

void EventHandle::onQuit()
{
    if (wallpaperSettings) {
        wallpaperSettings->deleteLater();
        wallpaperSettings = nullptr;
    }
}

void EventHandle::onChanged()
{
    if (!wallpaperSettings)
        return;

    auto wallpaper = wallpaperSettings->currentWallpaper();
    int wallpaperChanged = EventHelperFunc::getEventID(eSignals, kEventWallpaperChanged);
    if (wallpaperChanged > 0)
        dpfInstance.eventDispatcher().publish(wallpaperChanged, wallpaper.first, wallpaper.second);
    else {
        qWarning() << "invalid Event_WallpaperChanged id" << wallpaperChanged;
    }
}

void EventHandle::show(QString name, int mode)
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
    connect(wallpaperSettings, &WallpaperSettings::quit, this, &EventHandle::onQuit);
    connect(wallpaperSettings, &WallpaperSettings::backgroundChanged, this, &EventHandle::onChanged);

    wallpaperSettings->show();
    wallpaperSettings->activateWindow();
    wallpaperSettings->refreshList();

    // auto focus tool
    auto autoAct = new AutoActivateWindow(wallpaperSettings);
    autoAct->setWatched(wallpaperSettings);
    autoAct->start();
}
