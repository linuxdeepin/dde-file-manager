// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    bool wallpaperSetting(const QString &name);
    bool screenSaverSetting(const QString &name);
    bool hookCanvasRequest(const QString &screen);
#ifndef COMPILE_ON_V20
    void onQuit();

protected:
    void onChanged();
    void show(QString name, int mode);

private:
    WallpaperSettings *wallpaperSettings = nullptr;
#endif
};

class WlSetPlugin : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "wallpapersetting.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

private:
    void registerDBus();

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
