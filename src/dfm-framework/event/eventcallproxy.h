#ifndef EVENTCALLPROXY_H
#define EVENTCALLPROXY_H

#include <QObject>
#include <QDebug>

#include "dfm-framework/log/frameworklog.h"

#include "dfm-framework/event/event.h"

#include <functional>

DPF_BEGIN_NAMESPACE

#define TOPIC_SIGNAL(func,topic) \
    void func(const Event& event) \
    { \
        if (event.topic().isEmpty()) { \
            qCCritical(Framework) << "can't send topic empty event"; \
            return; \
        }\
        if (event.topic() != topic) return; \
    } \

#define TOPIC_SLOT Q_SLOT

template<class T>
class EventCallProxy : public QObject
{
    std::list<std::function<void(T::*)(const Event &event)>> funcs;

public:
    EventCallProxy(QObject *parent = nullptr)
        : QObject(parent)
    {

    }

    template<class CT,class...Args>
    static void call(CT* ins, Args...as)
    {

    }

    template<class CT, class...Args>
    static void connect(const QString &topic,
                        CT* ins, Args...as)
    {

    }
};

DPF_END_NAMESPACE

#endif // EVENTCALLPROXY_H
