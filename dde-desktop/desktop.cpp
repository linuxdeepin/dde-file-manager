/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "desktop.h"

#include <QDebug>
#include <QApplication>
#include <QStandardPaths>
#include <QStyleOptionViewItem>
#include <QDir>
#include <QDBusConnection>

#include <durl.h>

#include "view/canvasgridview.h"
#include "presenter/apppresenter.h"

#include "../dde-wallpaper-chooser/frame.h"

#ifndef DISABLE_ZONE
#include "../dde-zone/mainwindow.h"
#endif

using WallpaperSettings = Frame;

#ifndef DISABLE_ZONE
using ZoneSettings = ZoneMainWindow;
#endif

class DesktopPrivate
{
public:
    CanvasGridView      screenFrame;
    WallpaperSettings *wallpaperSettings{ nullptr };

#ifndef DISABLE_ZONE
    ZoneSettings *zoneSettings { nullptr };
#endif
};

Desktop::Desktop()
    : d(new DesktopPrivate)
{

}

Desktop::~Desktop()
{

}

void Desktop::loadData()
{
    Presenter::instance()->init();
}

void Desktop::loadView()
{
    auto desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    auto desktopUrl = DUrl::fromLocalFile(desktopPath);

    if (!QDir(desktopPath).exists()) {
        QDir::home().mkpath(desktopPath);
    }

    d->screenFrame.setRootUrl(desktopUrl);
}

void Desktop::showWallpaperSettings()
{
    if (d->wallpaperSettings) {
        d->wallpaperSettings->deleteLater();
        d->wallpaperSettings = nullptr;
    }

    d->wallpaperSettings = new WallpaperSettings;
    connect(d->wallpaperSettings, &Frame::done, this, [ = ] {
        d->wallpaperSettings->deleteLater();
        d->wallpaperSettings = nullptr;
    });

    d->wallpaperSettings->show();
    d->wallpaperSettings->grabKeyboard();
}

#ifndef DISABLE_ZONE
void Desktop::showZoneSettings()
{
    if (d->zoneSettings) {
        d->zoneSettings->deleteLater();
        d->zoneSettings = nullptr;
    }

    d->zoneSettings = new ZoneSettings;
    connect(d->zoneSettings, &ZoneMainWindow::finished, this, [ = ] {
        d->zoneSettings->deleteLater();
        d->zoneSettings = nullptr;
    });

    d->zoneSettings->show();
    d->zoneSettings->grabKeyboard();
}
#endif

void Desktop::initDebugDBus(QDBusConnection &conn)
{
    if (!conn.registerObject(DesktopCanvasPath, &d->screenFrame,
                             QDBusConnection::ExportScriptableSlots)) {
        qDebug() << "registerObject Failed" << conn.lastError();
        exit(0x0004);
    }
}

CanvasGridView *Desktop::getView()
{
    return (&(d->screenFrame));
}

void Desktop::Show()
{
    d->screenFrame.show();
}
