// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wlsetplugin.h"
#include "wallpapersettings.h"
#include "settingsdbusinterface.h"
#include "private/autoactivatewindow.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include <dfm-base/utils/universalutils.h>
#include <QDBusConnection>

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
    registerDBus();
    return true;
}

void WlSetPlugin::stop()
{
    delete handle;
    handle = nullptr;
}

void WlSetPlugin::registerDBus()
{
    Q_ASSERT(handle);
    auto ifs = new SettingsDBusInterface(handle);

    QDBusConnection conn = QDBusConnection::sessionBus();
    auto registerOptions = QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;
    if (!conn.registerObject("/org/deepin/dde/desktop/wallpapersettings", "org.deepin.dde.desktop.wallpapersettings", ifs, registerOptions)) {
        qCritical() << "org.deepin.dde.desktop.wallpapersettings register object failed" << conn.lastError();
        delete ifs;
    }
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

bool EventHandle::wallpaperSetting(const QString &name)
{
    show(name, (int)WallpaperSettings::Mode::WallpaperMode);
    return true;
}

bool EventHandle::screenSaverSetting(const QString &name)
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
    if (name.isNull() || name.isEmpty()
            || ddplugin_desktop_util::screenProxyScreen(name).isNull()) {
        qWarning() << "invalid screen" << name;
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

    // auto focus tool
    auto autoAct = new AutoActivateWindow(wallpaperSettings);
    autoAct->setWatched(wallpaperSettings);
    autoAct->start();

    QMetaObject::invokeMethod(wallpaperSettings,"refreshList",Qt::QueuedConnection);
}

bool EventHandle::hookCanvasRequest(const QString &screen)
{
    wallpaperSetting(screen);
    return true;
}
