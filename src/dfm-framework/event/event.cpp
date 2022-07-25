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
    QMap<EventStratege, EventMap> eventsMap {
        { EventStratege::kSignal, {} },
        { EventStratege::kSlot, {} },
        { EventStratege::kHook, {} }
    };
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
    static EventDispatcherManager ins;
    return &ins;
}

EventSequenceManager *Event::sequence()
{
    static EventSequenceManager ins;
    return &ins;
}

EventChannelManager *Event::channel()
{
    static EventChannelManager ins;
    return &ins;
}

void Event::registerEventType(EventStratege stratege, const QString &space, const QString &topic)
{
    QString key { space + ":" + topic };
    if (Q_UNLIKELY(d->eventsMap[stratege].contains(key))) {
        qWarning() << "Register repeat event: " << key;
        return;
    }

    QWriteLocker guard(&d->rwLock);
    d->eventsMap[stratege].insert(key, genCustomEventId());
}

EventType Event::eventType(const QString &space, const QString &topic)
{
    static const QMap<QString, EventStratege> prefixMap { { kSignalStrategePrefix, EventStratege::kSignal },
                                                          { kSlotStrategePrefix, EventStratege::kSlot },
                                                          { kHookStrategePrefix, EventStratege::kHook } };
    static const QStringList prefixKeys { prefixMap.keys() };

    QStringList splits { topic.split("_") };
    Q_ASSERT(splits.size() > 0);
    QString prefix { splits.first().toLower() };
    if (!prefixKeys.contains(prefix))
        return EventTypeScope::kInValid;
    EventStratege stratege { prefixMap.value(prefix) };
    QString key { space + ":" + topic };

    QReadLocker guard(&d->rwLock);
    return d->eventsMap[stratege].contains(key) ? d->eventsMap[stratege].value(key) : EventTypeScope::kInValid;
}

QStringList Event::pluginTopics(const QString &space)
{
    QStringList topics;

    topics.append(pluginTopics(space, EventStratege::kSignal));
    topics.append(pluginTopics(space, EventStratege::kSlot));
    topics.append(pluginTopics(space, EventStratege::kHook));

    return topics;
}

QStringList Event::pluginTopics(const QString &space, EventStratege stratege)
{
    QStringList topics;
    auto &&spaces { d->eventsMap.value(stratege).keys() };
    for (QString name : spaces) {
        if (name.startsWith(space))
            topics.append(name.remove(space + ":"));
    }

    return topics;
}

Event::Event()
    : d(new EventPrivate)
{
    EventConverter::registerConverter([this](const QString &space, const QString &topic) {
        return eventType(space, topic);
    });
}
