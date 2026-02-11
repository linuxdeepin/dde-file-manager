// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENT_H
#define EVENT_H

#include <dfm-framework/event/eventdispatcher.h>
#include <dfm-framework/event/eventsequence.h>
#include <dfm-framework/event/eventchannel.h>

// ====== Event API Statement ======
// usually the namespace of the plugin
#define DPF_EVENT_NAMESPACE(spaceMacro)
// register event
#define DPF_EVENT_REG(stratege, topicMacro)
#define DPF_EVENT_REG_SIGNAL(topicMacro)   // use dispatcher
#define DPF_EVENT_REG_SLOT(topicMacro)   // use channel
#define DPF_EVENT_REG_HOOK(topicMacro)   // use sequence

// acquire event type
#define DPF_EVENT_TYPE(spaceStr, topicStr)

// event instance
#define dpfEvent
#define dpfSignalDispatcher   // signal event use it
#define dpfSlotChannel   // slot event use it
#define dpfHookSequence   // hook event use it

// ====== Event API Statement ======

DPF_BEGIN_NAMESPACE

class EventPrivate;
class Event
{
    Q_DISABLE_COPY(Event)

public:
    static Event *instance();

    [[gnu::hot]] EventDispatcherManager *dispatcher();
    EventSequenceManager *sequence();
    [[gnu::hot]] EventChannelManager *channel();

    [[gnu::hot]] void registerEventType(EventStratege stratege, const QString &space, const QString &topic);
    [[gnu::hot]] EventType eventType(const QString &space, const QString &topic);

    QStringList pluginTopics(const QString &space);
    QStringList pluginTopics(const QString &space, EventStratege stratege);

private:
    Event();
    ~Event() = default;

private:
    QScopedPointer<EventPrivate> d;
};

DPF_END_NAMESPACE

// event instance
#undef dpfEvent
#define dpfEvent ::DPF_NAMESPACE::Event::instance()

// define namespace for plugin's event
#undef DPF_EVENT_NAMESPACE
#define DPF_EVENT_NAMESPACE2(space) static constexpr char __KPF_EVENT_NAMESPACE__[] { #space };
#define DPF_EVENT_NAMESPACE(spaceMacro) DPF_EVENT_NAMESPACE2(spaceMacro)

// register event
#undef DPF_EVENT_REG
#define DPF_EVENT_REG(stratege, topicMacro)                                              \
    struct DPF_EVENT_##topicMacro                                                        \
    {                                                                                    \
        DPF_EVENT_##topicMacro()                                                         \
        {                                                                                \
            static_assert(*__KPF_EVENT_NAMESPACE__);                                     \
            dpfEvent->registerEventType(stratege, __KPF_EVENT_NAMESPACE__, #topicMacro); \
        }                                                                                \
    };                                                                                   \
    DPF_EVENT_##topicMacro _dpf_event_##topicMacro;

#undef DPF_EVENT_REG_SIGNAL
#define DPF_EVENT_REG_SIGNAL(topicMacro) DPF_EVENT_REG(DPF_NAMESPACE::EventStratege::kSignal, topicMacro)

#undef DPF_EVENT_REG_SLOT
#define DPF_EVENT_REG_SLOT(topicMacro) DPF_EVENT_REG(DPF_NAMESPACE::EventStratege::kSlot, topicMacro)

#undef DPF_EVENT_REG_HOOK
#define DPF_EVENT_REG_HOOK(topicMacro) DPF_EVENT_REG(DPF_NAMESPACE::EventStratege::kHook, topicMacro)

// acquire event type
#undef DPF_EVENT_TYPE
#define DPF_EVENT_TYPE(spaceStr, topicStr) dpfEvent->eventType(spaceStr, topicStr)

// dispatcher
#undef dpfSignalDispatcher
#define dpfSignalDispatcher dpfEvent->dispatcher()

// channel
#undef dpfSlotChannel
#define dpfSlotChannel dpfEvent->channel()

// sequence
#undef dpfHookSequence
#define dpfHookSequence dpfEvent->sequence()

#endif   // EVENT_H
