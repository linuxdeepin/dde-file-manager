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
#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H

#include "dfm-framework/dfm_framework_global.h"
#include "dfm-framework/event/eventhelper.h"
#include "dfm-framework/event/invokehelper.h"

#include <QVariant>
#include <QFuture>
#include <QSharedPointer>
#include <QReadWriteLock>

DPF_BEGIN_NAMESPACE

using EventType = int;

class EventDispatcher
{
public:
    using Listener = std::function<QVariant(const QVariantList &)>;
    using ListenerList = QList<Listener>;

    void dispatch();
    void dispatch(const QVariantList &params);
    template<class T, class... Args>
    inline void dispatch(T param, Args &&... args)
    {
        QVariantList ret;
        makeVariantList(&ret, param, std::forward<Args>(args)...);
        dispatch(ret);
    }

    QFuture<void> asyncDispatch();
    QFuture<void> asyncDispatch(const QVariantList &params);
    template<class T, class... Args>
    inline QFuture<void> asyncDispatch(T param, Args &&... args)
    {
        QVariantList ret;
        makeVariantList(&ret, param, std::forward<Args>(args)...);
        return asyncDispatch(ret);
    }

    template<class T, class Func>
    inline void append(T *obj, Func method)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        static_assert(!std::is_pointer<T>::value, "Receiver::bind's template type T must not be a pointer type");

        QMutexLocker guard(&listenerMutex);
        auto func = [obj, method](const QVariantList &args) -> QVariant {
            EventHelper<decltype(method)> helper = (EventHelper<decltype(method)>(obj, method));
            return helper.invoke(args);
        };
        allListeners.push_back(func);
    }

private:
    ListenerList allListeners;
    QMutex listenerMutex;
};

class EventDispatcherManager
{
    Q_DISABLE_COPY(EventDispatcherManager)

public:
    static EventDispatcherManager &instance();

    template<class T, class Func>
    [[gnu::hot]] inline void subscribe(EventType type, T *obj, Func method)
    {
        QWriteLocker lk(&rwLock);
        if (dispatcherMap.contains(type)) {
            dispatcherMap[type]->append(obj, method);
        } else {
            DispatcherPtr dispatcher { new EventDispatcher };
            dispatcher->append(obj, method);
            dispatcherMap.insert(type, dispatcher);
        }
    }

    void unsubscribe(EventType type);

    template<class T, class... Args>
    [[gnu::hot]] inline bool publish(EventType type, T param, Args &&... args)
    {
        QReadLocker lk(&rwLock);
        if (Q_LIKELY(dispatcherMap.contains(type))) {
            auto dispatcher = dispatcherMap.value(type);
            lk.unlock();
            if (dispatcher) {
                dispatcher->dispatch(param, std::forward<Args>(args)...);
                return true;
            }
        }
        return false;
    }

    template<class T, class... Args>
    inline QFuture<void> asyncPublish(EventType type, T param, Args &&... args)
    {
        QReadLocker lk(&rwLock);
        if (Q_LIKELY(dispatcherMap.contains(type))) {
            return dispatcherMap[type]->asyncDispatch(param, std::forward<Args>(args)...);
        }
        return QFuture<void>();
    }

private:
    using DispatcherPtr = QSharedPointer<EventDispatcher>;
    using EventDispatcherMap = QMap<int, DispatcherPtr>;

    EventDispatcherManager() = default;
    ~EventDispatcherManager() = default;

private:
    EventDispatcherMap dispatcherMap;
    QReadWriteLock rwLock;
};

DPF_END_NAMESPACE

#endif   // EVENTDISPATCHER_H
