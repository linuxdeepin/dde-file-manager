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
#include "private/frameservice_p.h"

DSB_D_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

FrameServicePrivate::FrameServicePrivate(FrameService *parent)
    : QObject(parent)
    , q(parent)
{
    setObjectName("dfm_service_desktop::FrameServicePrivate");
}

bool FrameServicePrivate::setProxy(QObject *ptr)
{
    if (proxy)
        return false;

    AbstractDesktopFrame *obj = qobject_cast<AbstractDesktopFrame *>(ptr);
    if (!obj)
        return false;

    proxy = obj;

    // all the signals are non interruptible, so using Qt::DirectConnection.
    connect(proxy, &AbstractDesktopFrame::windowAboutToBeBuilded, q, &FrameService::windowAboutToBeBuilded, Qt::DirectConnection);
    connect(proxy, &AbstractDesktopFrame::windowBuilded, q, &FrameService::windowBuilded, Qt::DirectConnection);
    connect(proxy, &AbstractDesktopFrame::geometryChanged, q, &FrameService::geometryChanged, Qt::DirectConnection);
    connect(proxy, &AbstractDesktopFrame::availableGeometryChanged, q, &FrameService::availableGeometryChanged, Qt::DirectConnection);

    return true;
}

QList<QWidget *> FrameService::rootWindows() const
{
    if (d->proxy)
        return d->proxy->rootWindows();

    return {};
}

FrameService::FrameService(QObject *parent)
    : PluginService(parent)
    , AutoServiceRegister<FrameService>()
    , d(new FrameServicePrivate(this))
{

}

