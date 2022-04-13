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
#include "eventprovider.h"

DSB_D_USE_NAMESPACE

bool EventProvider::monitor(EventChanged func, void *data)
{
    if (!func)
        return false;

    QWriteLocker lk(&locker);
    if (monitors.contains(func))
        return true;

    monitors.insert(func, data);
    return true;
}

void EventProvider::unmonitor(EventChanged func)
{
    QWriteLocker lk(&locker);
    monitors.remove(func);
}

void EventProvider::notify(int eventType, const QStringList &eventKeys) const
{
    // to prevent deadlock caused by calling EventProvider's function in callback function
    QReadLocker lk(&locker);
    auto list = monitors;
    lk.unlock();

    for (auto itor = list.begin(); itor != list.end(); ++itor)
        itor.key()(eventType, eventKeys, itor.value());
}

EventProvider::EventProvider()
{

}

EventProvider::~EventProvider()
{

}
