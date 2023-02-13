// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingsdbusinterface.h"
#include "wlsetplugin.h"


using namespace ddplugin_wallpapersetting;

SettingsDBusInterface::SettingsDBusInterface(EventHandle *parent)
    : QObject(parent)
    , QDBusContext()
    , handle(parent)
{

}

void SettingsDBusInterface::ShowWallpaperChooser(const QString &screen)
{
    handle->wallpaperSetting(screen);
}

void SettingsDBusInterface::ShowScreensaverChooser(const QString &screen)
{
    handle->screenSaverSetting(screen);
}
