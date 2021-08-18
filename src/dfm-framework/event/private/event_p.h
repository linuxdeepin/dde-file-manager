#ifndef EVENT_P_H
#define EVENT_P_H

#include "dfm-framework/event/event.h"

#include <QString>
#include <QVariant>

DPF_BEGIN_NAMESPACE

static QString EVENT_TOPIC_KEY{"EVENT_TOPIC_KEY"};
static QString EVENT_DATA_KEY{"EVENT_DATA_KEY"};

class EventPrivate
{
    QHash<QString,QVariant> sourceHash;
    Event * const q_ptr;
    friend class Event;

public:
    explicit EventPrivate(Event *qq)
        : q_ptr(qq)
    {

    }
};

DPF_END_NAMESPACE

#endif // EVENT_P_H
