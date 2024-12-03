// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wlsetplugin.h"
#include "settingsdbusinterface.h"

#ifdef COMPILE_ON_V2X
#    include "wallpapersettings.h"
#    include "private/autoactivatewindow.h"
#    include "desktoputils/ddpugin_eventinterface_helper.h"

#    include <dfm-base/utils/universalutils.h>
#    include <dfm-base/utils/windowutils.h>
#    include <dfm-base/utils/sysinfoutils.h>

#    include <QProcess>
#else
#    include <QDBusMessage>
#    include <QDBusPendingCall>
#endif

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
        fmCritical() << "org.deepin.dde.desktop.wallpapersettings register object failed" << conn.lastError();
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

#ifdef COMPILE_ON_V2X

bool EventHandle::wallpaperSetting(const QString &name)
{
    if (DFMBASE_NAMESPACE::SysInfoUtils::isDeepin23()) {
        show(name, (int)WallpaperSettings::Mode::WallpaperMode);
    } else {
        fmDebug() << "call ControlCenter serivce by dbus.";
        QDBusMessage msg = QDBusMessage::createMethodCall("org.deepin.dde.ControlCenter1", "/org/deepin/dde/ControlCenter1",
                                                          "org.deepin.dde.ControlCenter1", "ShowPage");
        msg.setArguments({ QVariant::fromValue(QString("personalization/wallpaper")) });
        QDBusConnection::sessionBus().asyncCall(msg, 5);
        fmInfo() << "ControlCenter serivce called." << msg.service() << msg.arguments();
    }

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
        fmWarning() << "invalid screen" << name;
        auto primary = ddplugin_desktop_util::screenProxyPrimaryScreen();
        if (!primary.get()) {
            fmCritical() << "get primary screen failed! stop show wallpaper";
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

    QMetaObject::invokeMethod(wallpaperSettings, "refreshList", Qt::QueuedConnection);
}
#else
bool EventHandle::wallpaperSetting(const QString &name)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("com.deepin.dde.ControlCenter", "/com/deepin/dde/ControlCenter",
                                                      "com.deepin.dde.ControlCenter", "ShowPage");
    msg.setArguments({ QVariant::fromValue(QString("personalization")), QVariant::fromValue(QString("WallpaperSetting")) });
    QDBusConnection::sessionBus().asyncCall(msg, 5);
    fmInfo() << "ControlCenter serivce called." << msg.service() << msg.arguments();
    return true;
}

bool EventHandle::screenSaverSetting(const QString &name)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("com.deepin.dde.ControlCenter", "/com/deepin/dde/ControlCenter",
                                                      "com.deepin.dde.ControlCenter", "ShowPage");
    msg.setArguments({ QVariant::fromValue(QString("personalization")), QVariant::fromValue(QString("ScreensaverSetting")) });
    QDBusConnection::sessionBus().asyncCall(msg, 5);
    fmInfo() << "ControlCenter serivce called." << msg.service() << msg.arguments();
    return true;
}
#endif

bool EventHandle::hookCanvasRequest(const QString &screen)
{
    wallpaperSetting(screen);
    return true;
}
