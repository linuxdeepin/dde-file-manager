// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/event/event.h>

DPF_BEGIN_NAMESPACE
class EventPrivate
{
public:
    using EventMap = QMap<QString, EventType>;

    const QMap<QString, EventStratege> prefixMap { 
        { kSignalStrategePrefix, EventStratege::kSignal },
        { kSlotStrategePrefix, EventStratege::kSlot },
        { kHookStrategePrefix, EventStratege::kHook } 
    };
    const QStringList prefixKeys { prefixMap.keys() };

    QReadWriteLock rwLock;
    QMap<EventStratege, EventMap> eventsMap {
        { EventStratege::kSignal, {} },
        { EventStratege::kSlot, {} },
        { EventStratege::kHook, {} }
    };
};

DPF_END_NAMESPACE

DPF_USE_NAMESPACE

/*!
 * \brief Get the global Event instance
 * \details Uses Leaky Singleton pattern to avoid static destruction order issues.
 * The instance is created once and never destroyed, which is safe for system-level
 * event managers that need to exist throughout the entire program lifecycle.
 * \return Pointer to the global Event instance
 */
Event *Event::instance()
{
    // C++11 guarantees thread-safe initialization of function static variables
    // The instance is created on first call and never destroyed
    static Event *instance = new Event();
    return instance;
}

/*!
 * \brief Get the global EventDispatcherManager instance
 * \details Uses Leaky Singleton pattern to avoid static destruction order issues.
 * The instance is created once and never destroyed, which is safe for system-level
 * dispatcher managers that need to exist throughout the entire program lifecycle.
 * \return Pointer to the global EventDispatcherManager instance
 */
EventDispatcherManager *Event::dispatcher()
{
    // C++11 guarantees thread-safe initialization of function static variables
    // The instance is created on first call and never destroyed
    static EventDispatcherManager *instance = new EventDispatcherManager();
    return instance;
}

/*!
 * \brief Get the global EventSequenceManager instance
 * \details Uses Leaky Singleton pattern to avoid static destruction order issues.
 * The instance is created once and never destroyed, which is safe for system-level
 * sequence managers that need to exist throughout the entire program lifecycle.
 * \return Pointer to the global EventSequenceManager instance
 */
EventSequenceManager *Event::sequence()
{
    // C++11 guarantees thread-safe initialization of function static variables
    // The instance is created on first call and never destroyed
    static EventSequenceManager *instance = new EventSequenceManager();
    return instance;
}

/*!
 * \brief Get the global EventChannelManager instance
 * \details Uses Leaky Singleton pattern to avoid static destruction order issues.
 * The instance is created once and never destroyed, which is safe for system-level
 * channel managers that need to exist throughout the entire program lifecycle.
 * \return Pointer to the global EventChannelManager instance
 */
EventChannelManager *Event::channel()
{
    // C++11 guarantees thread-safe initialization of function static variables
    // The instance is created on first call and never destroyed
    static EventChannelManager *instance = new EventChannelManager();
    return instance;
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
    QStringList splits { topic.split("_") };
    Q_ASSERT(splits.size() > 0);
    QString prefix { splits.first().toLower() };
    if (!d->prefixKeys.contains(prefix))
        return EventTypeScope::kInValid;
    EventStratege stratege { d->prefixMap.value(prefix) };
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
