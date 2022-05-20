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
#ifndef EVENT_H
#define EVENT_H

#include "dfm-framework/event/dispatcher/eventdispatcher.h"
#include "dfm-framework/event/sequence/eventsequence.h"
#include "dfm-framework/event/channel/eventchannel.h"
#include "dfm-framework/event/unicast/eventunicast.h"   // TODO(zhangs): remove it after refactor services

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
