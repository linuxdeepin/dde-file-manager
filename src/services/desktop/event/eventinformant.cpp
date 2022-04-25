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

#define notiyAll(type, provider) {\
    auto es = provider->query(type).keys(); \
    if (!es.isEmpty()) \
        notify(type, es);}

// receive the notification sended by provider.
static void eventChangedCallback(int eventType, const QStringList &event, void *ptr)
{
    class HookClass : public EventInformant
    {
    public:
        using EventInformant::onEventChanged;
    };

    if (HookClass *self = static_cast<HookClass *>(ptr))
        self->onEventChanged(eventType, event);
}

QVariantHash EventInformant::query(int type) const
{
    QVariantHash events;
    QReadLocker lk(&locker);
    for (auto e : providers) {
        auto ed = e->query(type);
        for (auto itor = ed.begin(); itor != ed.end(); ++itor)
            events.insert(itor.key(), itor.value());
    }
    return events;
}

bool EventInformant::registerEvent(EventProvider *e)
{
    if (!e)
        return false;

    {
        QWriteLocker lk(&locker);
        if (providers.contains(e))
            return true;

        providers.append(e);
    }

    e->monitor(&eventChangedCallback, this);

    notiyAll(EventType::kEventSignal, e);
    notiyAll(EventType::kEventSlot, e);
    notiyAll(EventType::kSeqSignal, e);
    return true;
}

void EventInformant::unRegisterEvent(EventProvider *e)
{
    {
        QWriteLocker lk(&locker);
        providers.removeAll(e);
    }

    e->unmonitor(&eventChangedCallback);

    notiyAll(EventType::kEventSignal, e);
    notiyAll(EventType::kEventSlot, e);
    notiyAll(EventType::kSeqSignal, e);
}

EventInformant::~EventInformant()
{
    QReadLocker lk(&locker);
    QList<EventProvider *> list = std::move(providers);
    lk.unlock();

    for (auto e : list)
        e->unmonitor(&eventChangedCallback);
}

void EventInformant::onEventChanged(int eventType, const QStringList &event)
{
    if (!event.isEmpty())
        notify(eventType, event);
}

