/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "event.h"

DPF_BEGIN_NAMESPACE
class EventPrivate
{
public:
    using EventMap = QMap<QString, EventType>;

    QReadWriteLock rwLock;
    EventMap signalEventMap;
    EventMap slotEventMap;
    EventMap hookEventMap;
};
DPF_END_NAMESPACE

DPF_USE_NAMESPACE

Event *Event::instance()
{
    static Event ins;
    return &ins;
}

EventDispatcherManager *Event::dispatcher()
{
    return &EventDispatcherManager::instance();
}

EventSequenceManager *Event::sequence()
{
    return &EventSequenceManager::instance();
}

EventChannelManager *Event::channel()
{
    return &EventChannelManager::instance();
}

void Event::registerEventType(EventStratege stratege, const QString &space, const QString &topic)
{
    QWriteLocker guard(&d->rwLock);
    QString key { space + ":" + topic };
    switch (stratege) {
    case EventStratege::kSignal:
        d->signalEventMap.insert(key, genCustomEventId());
        return;
    case EventStratege::kSlot:
        d->slotEventMap.insert(key, genCustomEventId());
        return;
    case EventStratege::kHook:
        d->hookEventMap.insert(key, genCustomEventId());
        return;
    }
}

EventType Event::eventType(EventStratege stratege, const QString &space, const QString &topic)
{
    QReadLocker guard(&d->rwLock);
    QString key { space + ":" + topic };
    switch (stratege) {
    case EventStratege::kSignal:
        return d->signalEventMap.contains(key) ? d->signalEventMap.value(key) : EventTypeScope::kInValid;
    case EventStratege::kSlot:
        return d->slotEventMap.contains(key) ? d->slotEventMap.value(key) : EventTypeScope::kInValid;
    case EventStratege::kHook:
        return d->hookEventMap.contains(key) ? d->hookEventMap.value(key) : EventTypeScope::kInValid;
    }
    return EventTypeScope::kInValid;
}

Event::Event()
    : d(new EventPrivate)
{
}
