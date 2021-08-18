#include "event.h"

#include "private/event_p.h"

DPF_USE_NAMESPACE

Event::Event()
    : d_ptr(new EventPrivate(this))
{

}

Event::~Event(){
    delete d_ptr;
}

void Event::setTopic(const QString &topic)
{
    d_ptr->sourceHash[EVENT_TOPIC_KEY] = topic;
}

QString Event::topic()
{
    return d_ptr->sourceHash[EVENT_TOPIC_KEY].toString();
}

void Event::setData(const QVariant &data)
{
    d_ptr->sourceHash[EVENT_DATA_KEY] = data;
}

QVariant Event::data()
{
    return d_ptr->sourceHash[EVENT_DATA_KEY];
}
