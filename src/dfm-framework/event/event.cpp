// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/event/event.h>

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
        qCWarning(logDPF) << "Register repeat event: " << key;
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
