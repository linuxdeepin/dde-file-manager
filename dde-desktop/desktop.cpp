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

using WallpaperSettings=Frame;
using ZoneSettings=ZoneMainWindow;

class DesktopPrivate
{
public:
    Presenter        presenter;
    CanvasGridView      screenFrame;
    WallpaperSettings wallpaperSettings;
    ZoneSettings zoneSettings;
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
    d->presenter.init();
}

void Desktop::loadView()
{
    auto desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    auto desktopUrl = DUrl::fromLocalFile(desktopPath);
    d->screenFrame.setRootUrl(desktopUrl);
}

void Desktop::showWallpaperSettings()
{
    d->wallpaperSettings.show();
    d->wallpaperSettings.grabKeyboard();
}

void Desktop::showZoneSettings()
{
    d->zoneSettings.show();
    d->zoneSettings.grabKeyboard();
}

void Desktop::Show()
{
    d->screenFrame.show();
}
