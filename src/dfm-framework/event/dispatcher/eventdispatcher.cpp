/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "eventdispatcher.h"

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

    int size { globalFilterMap.size() };
    for (int i = 0; i != size; ++i) {
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
