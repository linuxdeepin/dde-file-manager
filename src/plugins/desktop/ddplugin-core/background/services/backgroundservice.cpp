/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "backgroundservice.h"
#include "background/backgroundmanager.h"

DSB_D_BEGIN_NAMESPACE

BackgroundService::BackgroundService(QObject *parent)
    : PluginService(parent)
    , AutoServiceRegister<BackgroundService>()
{
    proxy = new BackgroundManager(this);

    connect(proxy, &BackgroundManager::sigBackgroundBuilded
            , this, &BackgroundService::sigBackgroundBuilded, Qt::UniqueConnection);
}

QMap<QString, dfmbase::BackgroundWidgetPointer> BackgroundService::allBackground()
{
    return proxy->allBackgroundWidgets();
}

dfmbase::BackgroundWidgetPointer BackgroundService::background(const QString &screenName)
{
    return proxy->backgroundWidget(screenName);
}

QMap<QString, QString> BackgroundService::allBackgroundPath()
{
    return proxy->allBackgroundPath();
}

QString BackgroundService::backgroundPath(const QString &screen)
{
    return proxy->backgroundPath(screen);
}

DSB_D_END_NAMESPACE
