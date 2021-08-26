#include "eventcallproxy.h"

DPF_BEGIN_NAMESPACE

EventCallProxy::~EventCallProxy()
{
    auto itera = eventHandlers.begin();
    while (itera != eventHandlers.end()) {
        delete itera->ins;
        itera->className = nullptr;
        itera->process = nullptr;
        itera->topics.clear();
    }
}

DPF_END_NAMESPACE






