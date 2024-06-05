// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/event/eventdispatcher.h>

#include <QtConcurrent>

#include <algorithm>

DPF_USE_NAMESPACE

bool EventDispatcher::dispatch()
{
    return dispatch(QVariantList());
}

bool EventDispatcher::dispatch(const QVariantList &params)
{
    if (std::any_of(filterList.begin(), filterList.end(), [params](const EventHandler<Listener> &h) {
            return h.handler(params).toBool();
        })) {
        return false;
    }

    std::for_each(handlerList.begin(), handlerList.end(), [params](const EventHandler<Listener> &h) {
        h.handler(params);
    });

    return true;
}

QFuture<bool> EventDispatcher::asyncDispatch()
{
    return asyncDispatch(QVariantList());
}

QFuture<bool> EventDispatcher::asyncDispatch(const QVariantList &params)
{
    return QFuture<bool>(QtConcurrent::run([this, params]() -> bool {
        return this->dispatch(params);
    }));
}

bool EventDispatcherManager::installGlobalEventFilter(QObject *obj, EventDispatcherManager::GlobalFilter filter)
{
    Q_ASSERT(obj);

    QWriteLocker guard(&rwLock);
    return globalFilterMap.insert(obj, filter) != globalFilterMap.end();
}

bool EventDispatcherManager::removeGlobalEventFilter(QObject *obj)
{
    QWriteLocker guard(&rwLock);
    if (globalFilterMap.contains(obj))
        return globalFilterMap.remove(obj) > 0;

    return false;
}

bool EventDispatcherManager::globalFiltered(EventType type, const QVariantList &params)
{
    QReadLocker lk(&rwLock);

    qsizetype size { globalFilterMap.size() };
    for (qsizetype i = 0; i != size; ++i) {
        auto key { globalFilterMap.keys()[i] };
        if (key) {
            auto func { globalFilterMap.value(key) };
            lk.unlock();
            return func(type, params);
        }
    }

    return false;
}

bool EventDispatcherManager::unsubscribe(const QString &space, const QString &topic)
{
    Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
    return unsubscribe(EventConverter::convert(space, topic));
}

bool EventDispatcherManager::unsubscribe(EventType type)
{
    QWriteLocker guard(&rwLock);
    if (dispatcherMap.contains(type))
        return dispatcherMap.remove(type) > 0;

    return false;
}
