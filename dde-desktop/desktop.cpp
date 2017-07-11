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

#include <durl.h>

#include "view/canvasgridview.h"
#include "presenter/apppresenter.h"

#include "../dde-wallpaper-chooser/frame.h"
#include "../dde-zone/mainwindow.h"

using WallpaperSettings = Frame;
using ZoneSettings = ZoneMainWindow;

class DesktopPrivate
{
public:
    CanvasGridView      screenFrame;
    WallpaperSettings *wallpaperSettings = nullptr;
    ZoneSettings *zoneSettings = nullptr;
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

void Desktop::Show()
{
    d->screenFrame.show();
}
