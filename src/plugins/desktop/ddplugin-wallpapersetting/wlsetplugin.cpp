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
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include "dfm-base/utils/universalutils.h"

using namespace ddplugin_wallpapersetting;

#define WlSetSlot(topic, args...) \
            dpfSlotChannel->connect(QT_STRINGIFY(DDP_WALLPAERSETTING_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

#define CanvasMangerFollow(topic, args...) \
            dpfHookSequence->follow("ddplugin_canvas", QT_STRINGIFY2(topic), this, ##args)

#define WlSetDisconnect(topic) \
            dpfSlotChannel->disconnect(QT_STRINGIFY(DDP_WALLPAERSETTING_NAMESPACE), QT_STRINGIFY2(topic))

#define CanvasMangerUnfollow(topic, args...) \
            dpfHookSequence->unfollow("ddplugin_canvas", QT_STRINGIFY2(topic), this, ##args)

void WlSetPlugin::initialize()
{
}

bool WlSetPlugin::start()
{
    handle = new EventHandle();
    handle->init();
    return true;
}

dpf::Plugin::ShutdownFlag WlSetPlugin::stop()
{
    delete handle;
    handle = nullptr;
    return kSync;
}

EventHandle::EventHandle(QObject *parent)
    : QObject(parent)
{

}

EventHandle::~EventHandle()
{
    WlSetDisconnect(slot_WallpaperSettings_WallpaperSetting);
    WlSetDisconnect(slot_WallpaperSettings_ScreenSaverSetting);

    CanvasMangerUnfollow(hook_CanvasManager_RequestWallpaperSetting, &EventHandle::hookCanvasRequest);
}

bool EventHandle::init()
{
    WlSetSlot(slot_WallpaperSettings_WallpaperSetting, &EventHandle::wallpaperSetting);
    WlSetSlot(slot_WallpaperSettings_ScreenSaverSetting, &EventHandle::screenSaverSetting);

    // follow the request from canvas
    CanvasMangerFollow(hook_CanvasManager_RequestWallpaperSetting, &EventHandle::hookCanvasRequest);
    return true;
}

bool EventHandle::wallpaperSetting(QString name)
{
    show(name, (int)WallpaperSettings::Mode::WallpaperMode);
    return true;
}

bool EventHandle::screenSaverSetting(QString name)
{
    show(name, (int)WallpaperSettings::Mode::ScreenSaverMode);
    return true;
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
    // screen name and picture path.
    dpfSignalDispatcher->publish(QT_STRINGIFY(DDP_WALLPAERSETTING_NAMESPACE),
                                 "signal_WallpaperSettings_WallpaperChanged", wallpaper.first, wallpaper.second);
}

void EventHandle::show(QString name, int mode)
{
    if (name.isNull() || name.isEmpty()) {
        auto primary = ddplugin_desktop_util::screenProxyPrimaryScreen();
        if (!primary.get()) {
            qCritical() << "get primary screen failed! stop show wallpaper";
            return;
        }

        name = primary->name();
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

bool EventHandle::hookCanvasRequest(const QString &screen)
{
    wallpaperSetting(screen);
    return true;
}

