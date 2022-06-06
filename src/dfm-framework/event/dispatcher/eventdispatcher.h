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

class EventDispatcher
{
public:
    using Listener = std::function<QVariant(const QVariantList &)>;
    using HandlerList = QList<EventHandler<Listener>>;
    using Filter = std::function<bool(Listener, const QVariantList &)>;

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

        list.push_back(EventHandler<Listener> { obj, memberFunctionVoidCast(method), func });
    }

    template<class T, class Func>
    inline bool remove(T *obj, Func method)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        static_assert(!std::is_pointer<T>::value, "Receiver::bind's template type T must not be a pointer type");

        bool ret { true };
        QMutexLocker guard(&listenerMutex);
        for (auto handler : list) {
            if (handler.compare(obj, method)) {
                if (!list.removeOne(handler)) {
                    qWarning() << "Cannot remove: " << handler.objectIndex->objectName();
                    ret = false;
                }
            }
        }

        return ret;
    }

    void setFilter(Filter filter);
    void unsetFilter();
    Filter filter();

private:
    HandlerList list {};
    Filter curFilter {};
    QMutex listenerMutex;
};

class EventDispatcherManager
{
    Q_DISABLE_COPY(EventDispatcherManager)

public:
    static EventDispatcherManager &instance();

    template<class T, class Func>
    inline bool subscribe(const QString &space, const QString &topic, T *obj, Func method)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        if (Q_UNLIKELY(!subscribe(EventConverter::convert(space, topic), obj, std::move(method)))) {
            qCritical() << "Topic " << space << ":" << topic << "is invalid";
            return false;
        }
        return true;
    }

    template<class T, class Func>
    [[gnu::hot]] inline bool subscribe(EventType type, T *obj, Func method)
    {
        if (!isValidEventType(type)) {
            qCritical() << "Event " << type << "is invalid";
            return false;
        }

        QWriteLocker lk(&rwLock);
        if (dispatcherMap.contains(type)) {
            dispatcherMap[type]->append(obj, method);
        } else {
            DispatcherPtr dispatcher { new EventDispatcher };
            dispatcher->append(obj, method);
            dispatcherMap.insert(type, dispatcher);
        }
        return true;
    }

    template<class T, class Func>
    inline bool unsubscribe(const QString &space, const QString &topic, T *obj, Func method)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        return unsubscribe(EventConverter::convert(space, topic), obj, std::move(method));
    }

    template<class T, class Func>
    inline bool unsubscribe(EventType type, T *obj, Func method)
    {
        if (!obj || !method)
            return false;

        QWriteLocker lk(&rwLock);
        if (dispatcherMap.contains(type))
            return dispatcherMap[type]->remove(obj, std::move(method));

        return false;
    }

    template<class T, class... Args>
    inline bool publish(const QString &space, const QString &topic, T param, Args &&... args)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        return publish(EventConverter::convert(space, topic), param, std::forward<Args>(args)...);
    }

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

    inline bool publish(const QString &space, const QString &topic)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        return publish(EventConverter::convert(space, topic));
    }

    inline bool publish(EventType type)
    {
        QReadLocker lk(&rwLock);
        if (Q_LIKELY(dispatcherMap.contains(type))) {
            auto dispatcher = dispatcherMap.value(type);
            lk.unlock();
            if (dispatcher) {
                dispatcher->dispatch();
                return true;
            }
        }
        return false;
    }

    template<class T, class... Args>
    inline QFuture<void> asyncPublish(const QString &space, const QString &topic, T param, Args &&... args)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        return asyncPublish(EventConverter::convert(space, topic), param, std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    inline QFuture<void> asyncPublish(EventType type, T param, Args &&... args)
    {
        QReadLocker lk(&rwLock);
        if (Q_LIKELY(dispatcherMap.contains(type)))
            return dispatcherMap[type]->asyncDispatch(param, std::forward<Args>(args)...);
        return QFuture<void>();
    }

    inline QFuture<void> asyncPublish(const QString &space, const QString &topic)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        return asyncPublish(EventConverter::convert(space, topic));
    }

    inline QFuture<void> asyncPublish(EventType type)
    {
        QReadLocker lk(&rwLock);
        if (Q_LIKELY(dispatcherMap.contains(type)))
            return dispatcherMap[type]->asyncDispatch();
        return QFuture<void>();
    }

    bool installEventFilter(const QString &space, const QString &topic, EventDispatcher::Filter filter);
    bool installEventFilter(EventType type, EventDispatcher::Filter filter);
    bool removeEventFilter(const QString &space, const QString &topic);
    bool removeEventFilter(EventType type);

protected:
    bool unsubscribe(const QString &space, const QString &topic);
    bool unsubscribe(EventType type);

private:
    using DispatcherPtr = QSharedPointer<EventDispatcher>;
    using EventDispatcherMap = QMap<EventType, DispatcherPtr>;

    EventDispatcherManager() = default;
    ~EventDispatcherManager() = default;

private:
    EventDispatcherMap dispatcherMap;
    QReadWriteLock rwLock;
};

DPF_END_NAMESPACE

#endif   // EVENTDISPATCHER_H
