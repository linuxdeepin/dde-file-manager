#ifndef EVENT_P_H
#define EVENT_P_H

#include "dfm-framework/event/event.h"

#include <QString>
#include <QVariant>

DPF_BEGIN_NAMESPACE

class EventPrivate
{
    friend class Event;
    Event * const q_ptr;
    QHash<QString,QVariant> sourceHash;
    friend Q_CORE_EXPORT QDebug operator <<(QDebug, const Event &);
public:

    explicit EventPrivate(Event *qq)
        : q_ptr(qq)
    {

    }
};

DPF_END_NAMESPACE

#endif // EVENT_P_H
