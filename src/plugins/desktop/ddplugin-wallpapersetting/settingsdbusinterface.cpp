// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingsdbusinterface.h"
#include "wlsetplugin.h"

namespace ddplugin_wallpapersetting {
DFM_LOG_REGISTER_CATEGORY(DDP_WALLPAERSETTING_NAMESPACE)

SettingsDBusInterface::SettingsDBusInterface(EventHandle *parent)
    : QObject(parent), QDBusContext(), handle(parent)
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
}   // namespace ddplugin_wallpapersetting
