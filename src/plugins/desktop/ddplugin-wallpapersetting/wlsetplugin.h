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
#ifndef WLSETPLUGIN_H
#define WLSETPLUGIN_H

#include "ddplugin_wallpapersetting_global.h"

#include <services/desktop/event/eventprovider.h>

#include <dfm-framework/dpf.h>

DDP_WALLPAERSETTING_BEGIN_NAMESPACE

static constexpr char kEventWallpaperSetting[] = "WallpaperSetting";
static constexpr char kEventScreenSaverSetting[] = "ScreenSaverSetting";
static constexpr char kEventWallpaperChanged[] = "WallpaperChanged";

class WallpaperSettings;
class EventHandle : public QObject, public DSB_D_NAMESPACE::EventProvider
{
    Q_OBJECT
public:
    explicit EventHandle(QObject *parent = nullptr);
    QVariantHash query(int type) const override;
public slots:
    bool wallpaperSetting(QString name);
    bool screenSaverSetting(QString name);
    void onQuit();
    void pluginStarted();

protected:
    void onChanged();
    void show(QString name, int mode);

private:
    bool followEvent(DSB_D_NAMESPACE::EventProvider *);

private:
    WallpaperSettings *wallpaperSettings = nullptr;
    QVariantHash eSignals;
    QVariantHash eSlots;
};

class WlSetPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "wallpapersetting.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual ShutdownFlag stop() override;

private:
    EventHandle *handle = nullptr;
};

DDP_WALLPAERSETTING_END_NAMESPACE
#endif   // WLSETPLUGIN_H
