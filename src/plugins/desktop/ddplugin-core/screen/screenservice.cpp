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
#include "screenservice.h"

#include <QGuiApplication>
DSB_D_BEGIN_NAMESPACE

namespace PlatformTypes{
const QString WAYLAND {"wayland"};
const QString XCB {"xcb"};
} //namespace ScreenType

ScreenService::ScreenService()
{

}

ScreenService *ScreenService::instance()
{
    static ScreenService service;
    return &service;
}

QList<dfmbase::AbstractScreen *> ScreenService::allScreen(const QString &platform)
{
    auto proxy = ScreenFactory::create(platform);
    if (!proxy)
        return {};

    QObject::connect(proxy, &dfmbase::AbstractScreenProxy::screenAdded,
                     this, &ScreenService::screenAdded, Qt::UniqueConnection);

    QObject::connect(proxy, &dfmbase::AbstractScreenProxy::screenRemoved,
                     this, &ScreenService::screenRemoved, Qt::UniqueConnection);

    QObject::connect(proxy, &dfmbase::AbstractScreenProxy::screenChanged,
                     this, &ScreenService::screenChanged, Qt::UniqueConnection);

    return proxy->allScreen();
}

DSB_D_END_NAMESPACE
