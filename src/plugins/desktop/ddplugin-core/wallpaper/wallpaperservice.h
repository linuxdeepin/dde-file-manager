/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huangyu<huangyub@uniontech.com>
 *
 * Maintainer: huangyu<huangyub@uniontech.com>
 *             zhangyu<zhangyub@uniontech.com>
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
#ifndef WALLPAPERSERVICE_H
#define WALLPAPERSERVICE_H

#include "dfm_desktop_service_global.h"

#include <dfm-framework/framework.h>

class Core;
DSB_D_BEGIN_NAMESPACE
class WallpaperSettings;
class WallpaperService final : public dpf::PluginService, dpf::AutoServiceRegister<WallpaperService>
{
    Q_OBJECT
    Q_DISABLE_COPY(WallpaperService)
    friend class dpf::QtClassFactory<dpf::PluginService>;
    friend class ::Core;
public:
    static QString name()
    {
        return "org.deepin.service.WallpaperService";
    }
    void init();
public slots:
    void wallpaperSetting(QString name);
    void screenSaverSetting(QString name);
private slots:
    void onQuit();
    void onChanged();
protected:
    explicit WallpaperService(QObject *parent = nullptr);
    ~WallpaperService();
    void show(QString name, int mode);
private:
    WallpaperSettings *wallpaperSettings = nullptr;
};
DSB_D_END_NAMESPACE

#endif // WALLPAPERSERVICE_H
