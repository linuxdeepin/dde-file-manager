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

#include "private/screenservice_p.h"
#include <QGuiApplication>

DFMBASE_USE_NAMESPACE
DSB_D_USE_NAMESPACE

ScreenService::ScreenService(QObject *parent)
    : PluginService(parent)
    , AutoServiceRegister<ScreenService>()
    , d(new ScreenServicePrivate(this))
{

}

ScreenService::~ScreenService()
{

}

dfmbase::ScreenPointer ScreenService::primaryScreen()
{
    if (d->proxy)
        return d->proxy->primaryScreen();

    return nullptr;
}

QVector<dfmbase::ScreenPointer> ScreenService::screens() const
{
    if (d->proxy)
        return d->proxy->screens();

    return {};
}

QVector<dfmbase::ScreenPointer> ScreenService::logicScreens() const
{
    if (d->proxy)
        return d->proxy->logicScreens();

    return {};
}

dfmbase::ScreenPointer ScreenService::screen(const QString &name) const
{
    if (d->proxy)
        return d->proxy->screen(name);

    return nullptr;
}

qreal ScreenService::devicePixelRatio() const
{
    if (d->proxy)
        return d->proxy->devicePixelRatio();

    return -1;
}

dfmbase::DisplayMode ScreenService::displayMode() const
{
    if (d->proxy)
        return d->proxy->displayMode();
    return DisplayMode::Custom;
}

dfmbase::DisplayMode ScreenService::lastChangedMode() const
{
    if (d->proxy)
        return d->proxy->lastChangedMode();

    return DisplayMode::Custom;
}

void ScreenService::reset()
{
    if (d->proxy)
        return d->proxy->reset();
}

ScreenServicePrivate::ScreenServicePrivate(ScreenService *parent)
    : QObject(parent)
    , q(parent)
{
    setObjectName("dfm_service_desktop::ScreenServicePrivate");
}

bool ScreenServicePrivate::setProxy(QObject *ptr)
{
    if (proxy)
        return false;

    AbstractScreenProxy *obj = qobject_cast<AbstractScreenProxy *>(ptr);
    if (!obj)
        return false;

    connect(obj, &AbstractScreenProxy::screenChanged, q, &ScreenService::screenChanged, Qt::DirectConnection);
    connect(obj, &AbstractScreenProxy::displayModeChanged, q, &ScreenService::displayModeChanged, Qt::DirectConnection);
    connect(obj, &AbstractScreenProxy::screenGeometryChanged, q, &ScreenService::screenGeometryChanged, Qt::DirectConnection);
    connect(obj, &AbstractScreenProxy::screenAvailableGeometryChanged, q, &ScreenService::screenAvailableGeometryChanged, Qt::DirectConnection);

    proxy = obj;
    return true;
}
