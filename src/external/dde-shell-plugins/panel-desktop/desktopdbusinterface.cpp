// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopdbusinterface.h"

#include <QDBusInterface>
#include <QDBusPendingCall>

using namespace dde_desktop;

DesktopDBusInterface::DesktopDBusInterface(QObject *parent) : QObject(parent)
{

}

void DesktopDBusInterface::Refresh(bool silent)
{
    QDBusInterface ifs(kDesktopServiceName,
                       "/org/deepin/dde/desktop/canvas",
                       "org.deepin.dde.desktop.canvas");
    ifs.asyncCall("Refresh", silent);
}

void DesktopDBusInterface::ShowWallpaperChooser(const QString &screen)
{
    QDBusInterface ifs(kDesktopServiceName,
                       "/org/deepin/dde/desktop/wallpapersettings",
                       "org.deepin.dde.desktop.wallpapersettings");
    ifs.asyncCall("ShowWallpaperChooser", screen);
}

void DesktopDBusInterface::ShowScreensaverChooser(const QString &screen)
{
    QDBusInterface ifs(kDesktopServiceName,
                       "/org/deepin/dde/desktop/wallpapersettings",
                       "org.deepin.dde.desktop.wallpapersettings");
    ifs.asyncCall("ShowScreensaverChooser", screen);
}
