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

void EventDispatcher::dispatch()
{
    return dispatch(QVariantList());
}

void EventDispatcher::dispatch(const QVariantList &params)
{
    if (Q_UNLIKELY(curFilter)) {
        if (std::any_of(allListeners.begin(), allListeners.end(), [this, params](const Listener &listener) {
                if (curFilter(listener, params))
                    return true;
                return false;
            })) {
            return;
        }
    }

    std::for_each(allListeners.begin(), allListeners.end(), [params](const Listener &listener) {
        listener(params);
    });
}

QFuture<void> EventDispatcher::asyncDispatch()
{
    return asyncDispatch(QVariantList());
}

QFuture<void> EventDispatcher::asyncDispatch(const QVariantList &params)
{
    return QFuture<void>(QtConcurrent::run([this, params]() {
        return this->dispatch(params);
    }));
}

void EventDispatcher::setFilter(EventDispatcher::Filter filter)
{
    curFilter = filter;
}

void EventDispatcher::unsetFilter()
{
    curFilter = {};
}

EventDispatcher::Filter EventDispatcher::filter()
{
    return curFilter;
}

/*!
 * \class EventDispatcherManager
 * \brief
 */

EventDispatcherManager &EventDispatcherManager::instance()
{
    static EventDispatcherManager instance;
    return instance;
}

void EventDispatcherManager::unsubscribe(const QString &space, const QString &topic)
{
    Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
    unsubscribe(EventConverter::convert(space, topic));
}

void EventDispatcherManager::unsubscribe(EventType type)
{
    QWriteLocker guard(&rwLock);
    if (dispatcherMap.contains(type))
        dispatcherMap.remove(type);
}

bool EventDispatcherManager::installEventFilter(const QString &space, const QString &topic, EventDispatcher::Filter filter)
{
    Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
    return installEventFilter(EventConverter::convert(space, topic), filter);
}

bool EventDispatcherManager::installEventFilter(EventType type, EventDispatcher::Filter filter)
{
    if (dispatcherMap.contains(type)) {
        if (dispatcherMap.value(type)->filter())
            return false;
        QWriteLocker guard(&rwLock);
        dispatcherMap.value(type)->setFilter(filter);
        return true;
    }
    return false;
}

bool EventDispatcherManager::removeEventFilter(const QString &space, const QString &topic)
{
    Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
    return removeEventFilter(EventConverter::convert(space, topic));
}

bool EventDispatcherManager::removeEventFilter(EventType type)
{
    if (dispatcherMap.contains(type)) {
        QWriteLocker guard(&rwLock);
        dispatcherMap.value(type)->unsetFilter();
        return true;
    }
    return false;
}
