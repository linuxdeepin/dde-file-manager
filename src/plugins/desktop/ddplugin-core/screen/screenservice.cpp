/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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

#include "screenservice.h"
#include "screen/screenproxyqt.h"
#include "screenproxydbus.h"

#include "utils/desktoputils.h"

#include <QGuiApplication>

DSB_D_BEGIN_NAMESPACE

ScreenService::ScreenService(QObject *parent)
    : PluginService(parent)
    , AutoServiceRegister<ScreenService>()
{

}

ScreenService::~ScreenService()
{
    delete proxy;
}

void ScreenService::init()
{
    if (waylandDectected())
        proxy = new ScreenProxyDBus(this);
    else
        proxy = new ScreenProxyQt(this);
    proxy->reset();

    connect(proxy, &dfmbase::AbstractScreenProxy::screenChanged,
                     this, &ScreenService::screenChanged, Qt::UniqueConnection);

    connect(proxy, &dfmbase::AbstractScreenProxy::displayModeChanged,
                     this, &ScreenService::displayModeChanged, Qt::UniqueConnection);

    connect(proxy, &dfmbase::AbstractScreenProxy::screenGeometryChanged,
                     this, &ScreenService::screenGeometryChanged, Qt::UniqueConnection);

    connect(proxy, &dfmbase::AbstractScreenProxy::screenAvailableGeometryChanged,
            this, &ScreenService::screenAvailableGeometryChanged, Qt::UniqueConnection);
}

dfmbase::ScreenPointer ScreenService::primaryScreen()
{
    return proxy->primaryScreen();
}

QVector<dfmbase::ScreenPointer> ScreenService::screens() const
{
    return proxy->screens();
}

QVector<dfmbase::ScreenPointer> ScreenService::logicScreens() const
{
    return proxy->logicScreens();
}

dfmbase::ScreenPointer ScreenService::screen(const QString &name) const
{
    return proxy->screen(name);
}

qreal ScreenService::devicePixelRatio() const
{
    return proxy->devicePixelRatio();
}

dfmbase::DisplayMode ScreenService::displayMode() const
{
    return proxy->displayMode();
}

dfmbase::DisplayMode ScreenService::lastChangedMode() const
{
    return proxy->lastChangedMode();
}

void ScreenService::reset()
{
    return proxy->reset();
}

DSB_D_END_NAMESPACE
