#ifndef EVENTHANDLER_P_H
#define EVENTHANDLER_P_H

#include "dfm-framework/event/eventhandler.h"

DPF_BEGIN_NAMESPACE

class EventHandlerPrivate
{
    friend class EventHandler;
    EventHandler * const q_ptr;
    EventHandler::Type type;

public:
    //与public 接口保持一致
    EventHandlerPrivate() = delete;
    EventHandlerPrivate(EventHandler* qq)
        : q_ptr(qq)
    {

    }
};

DPF_END_NAMESPACE

#endif // EVENTHANDLER_P_H
