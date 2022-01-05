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

DPF_USE_NAMESPACE

EventDispatcher::EventDispatcher(EventType type)
    : eventType(type)
{
}

void EventDispatcher::dispatch()
{
    return dispatch(QVariantList());
}

void EventDispatcher::dispatch(const QVariantList &params)
{
    for (auto listener : allListeners)
        listener(params);
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

/*!
 * \class EventDispatcherManager
 * \brief
 */

EventDispatcherManager &EventDispatcherManager::instance()
{
    static EventDispatcherManager instance;
    return instance;
}

void EventDispatcherManager::unsubscribe(EventDispatcherManager::EventType type)
{
    QMutexLocker guard(&mutex);
    if (dispatcherMap.contains(type))
        dispatcherMap.remove(type);
}

EventDispatcherManager::EventDispatcherManager()
{
}

EventDispatcherManager::~EventDispatcherManager()
{
    auto keys = dispatcherMap.keys();
    dispatcherMap.clear();
}
