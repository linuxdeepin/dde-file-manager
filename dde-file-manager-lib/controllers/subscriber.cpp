#include "subscriber.h"


int Subscriber::eventKey() const
{
    return m_eventKey;
}

void Subscriber::setEventKey(int eventKey)
{
    m_eventKey = eventKey;
}
