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

#include <dfm-framework/dpf.h>

namespace ddplugin_wallpapersetting {

class WallpaperSettings;
class EventHandle : public QObject
{
    Q_OBJECT
public:
    explicit EventHandle(QObject *parent = nullptr);
    ~EventHandle();
    bool init();
public slots:
    bool wallpaperSetting(QString name);
    bool screenSaverSetting(QString name);
    void onQuit();

    bool hookCanvasRequest(const QString &screen);
protected:
    void onChanged();
    void show(QString name, int mode);
private:
    WallpaperSettings *wallpaperSettings = nullptr;
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
private:
    DPF_EVENT_NAMESPACE(DDP_WALLPAERSETTING_NAMESPACE)
    DPF_EVENT_REG_SIGNAL(signal_WallpaperSettings_WallpaperChanged)

    DPF_EVENT_REG_SLOT(slot_WallpaperSettings_WallpaperSetting)
    DPF_EVENT_REG_SLOT(slot_WallpaperSettings_ScreenSaverSetting)
};

}
#endif   // WLSETPLUGIN_H
