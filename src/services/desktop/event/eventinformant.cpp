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
#include "eventinformant.h"

DSB_D_USE_NAMESPACE

QVariantHash EventInformant::eventSignals() const
{
    QVariantHash events;
    QReadLocker lk(&locker);
    for (auto e : providers) {
        auto ed = e->eventSignals();
        for (auto itor = ed.begin(); itor != ed.end(); ++itor)
            events.insert(itor.key(), itor.value());
    }
    return events;
}

QVariantHash EventInformant::eventSlots() const
{
    QVariantHash events;
    QReadLocker lk(&locker);
    for (auto e : providers) {
        auto ed = e->eventSlots();
        for (auto itor = ed.begin(); itor != ed.end(); ++itor)
            events.insert(itor.key(), itor.value());
    }
    return events;
}

bool EventInformant::registerEvent(EventProvider *e)
{
    if (!e)
        return false;

    QWriteLocker lk(&locker);
    if (providers.contains(e))
        return true;

    providers.append(e);
    return true;
}

void EventInformant::unRegisterEvent(EventProvider *e)
{
    QWriteLocker lk(&locker);
    providers.removeAll(e);
}

