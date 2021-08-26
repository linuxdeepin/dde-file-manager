#include "eventhandler.h"

#include "private/eventhandler_p.h"

DPF_BEGIN_NAMESPACE

EventHandler::EventHandler()
    : QObject (nullptr)
    , d(new EventHandlerPrivate(this))
{

}

DPF_END_NAMESPACE
