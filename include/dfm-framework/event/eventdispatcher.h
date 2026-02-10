// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H

#include <dfm-framework/dfm_framework_global.h>
#include <dfm-framework/event/eventhelper.h>
#include <dfm-framework/event/invokehelper.h>

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
    using FilterList = QList<EventHandler<Listener>>;

    bool dispatch();
    bool dispatch(const QVariantList &params);
    template<class T, class... Args>
    inline bool dispatch(T param, Args &&... args)
    {
        QVariantList ret;
        makeVariantList(&ret, param, std::forward<Args>(args)...);
        return dispatch(ret);
    }

    QFuture<bool> asyncDispatch();
    QFuture<bool> asyncDispatch(const QVariantList &params);
    template<class T, class... Args>
    inline QFuture<bool> asyncDispatch(T param, Args &&... args)
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

        auto func = [obj, method](const QVariantList &args) -> QVariant {
            EventHelper<decltype(method)> helper = (EventHelper<decltype(method)>(obj, method));
            return helper.invoke(args);
        };

        handlerList.push_back(EventHandler<Listener> { obj, memberFunctionVoidCast(method), func });
    }

    template<class T, class Func>
    inline bool remove(T *obj, Func method)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        static_assert(!std::is_pointer<T>::value, "Receiver::bind's template type T must not be a pointer type");

        bool ret { true };
        for (auto handler : handlerList) {
            if (handler.compare(obj, method)) {
                if (!handlerList.removeOne(handler)) {
                    qCWarning(logDPF) << "Cannot remove: " << handler.objectIndex->objectName();
                    ret = false;
                }
            }
        }

        return ret;
    }

    template<class T, class Func>
    inline void appendFilter(T *obj, Func method)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        static_assert(!std::is_pointer<T>::value, "Receiver::bind's template type T must not be a pointer type");
#if __cplusplus > 201703L
        static_assert(std::is_same_v<bool, ReturnType<decltype(method)>>, "The return value of template method must is bool");
#elif __cplusplus > 201103L
        static_assert(std::is_same<bool, ReturnType<decltype(method)>>::value, "Template method's ReturnType must is bool");
#endif
        auto func = [obj, method](const QVariantList &args) -> bool {
            EventHelper<decltype(method)> helper = (EventHelper<decltype(method)>(obj, method));
            return helper.invoke(args).toBool();
        };
        filterList.push_back(EventHandler<Listener> { obj, memberFunctionVoidCast(method), func });
    }

    template<class T, class Func>
    inline bool removeFilter(T *obj, Func method)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        static_assert(!std::is_pointer<T>::value, "Receiver::bind's template type T must not be a pointer type");
#if __cplusplus > 201703L
        static_assert(std::is_same_v<bool, ReturnType<decltype(method)>>, "The return value of template method must is bool");
#elif __cplusplus > 201103L
        static_assert(std::is_same<bool, ReturnType<decltype(method)>>::value, "Template method's ReturnType must is bool");
#endif
        bool ret { true };
        for (auto handler : filterList) {
            if (handler.compare(obj, method)) {
                if (!filterList.removeOne(handler)) {
                    qCWarning(logDPF) << "Cannot remove: " << handler.objectIndex->objectName();
                    ret = false;
                }
            }
        }

        return ret;
    }

private:
    HandlerList handlerList {};
    FilterList filterList {};
};

class EventDispatcherManager
{
public:
    using GlobalFilter = std::function<bool(EventType type, const QVariantList &)>;

    template<class T, class Func>
    inline bool subscribe(const QString &space, const QString &topic, T *obj, Func method)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        if (!subscribe(EventConverter::convert(space, topic), obj, std::move(method))) {
            qCWarning(logDPF) << "Topic " << space << ":" << topic << "is invalid";
            return false;
        }
        return true;
    }

    template<class T, class Func>
    [[gnu::hot]] inline bool subscribe(EventType type, T *obj, Func method)
    {
        if (!isValidEventType(type)) {
            qCWarning(logDPF) << "Event " << type << "is invalid";
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
        threadEventAlert(space, topic);
        return publish(EventConverter::convert(space, topic), param, std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    [[gnu::hot]] inline bool publish(EventType type, T param, Args &&... args)
    {
        threadEventAlert(type);
        if (!globalFilterMap.isEmpty()) {
            QVariantList ret;
            makeVariantList(&ret, param, std::forward<Args>(args)...);
            if (globalFiltered(type, ret))
                return false;
        }

        QReadLocker lk(&rwLock);
        if (Q_LIKELY(dispatcherMap.contains(type))) {
            auto dispatcher = dispatcherMap.value(type);
            lk.unlock();
            if (dispatcher)
                return dispatcher->dispatch(param, std::forward<Args>(args)...);
        }
        return false;
    }

    inline bool publish(const QString &space, const QString &topic)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        threadEventAlert(space, topic);
        return publish(EventConverter::convert(space, topic));
    }

    inline bool publish(EventType type)
    {
        threadEventAlert(type);
        if (!globalFilterMap.isEmpty() && globalFiltered(type, QVariantList()))
            return false;

        QReadLocker lk(&rwLock);
        if (Q_LIKELY(dispatcherMap.contains(type))) {
            auto dispatcher = dispatcherMap.value(type);
            lk.unlock();
            if (dispatcher)
                return dispatcher->dispatch();
        }
        return false;
    }

    template<class T, class... Args>
    inline QFuture<bool> asyncPublish(const QString &space, const QString &topic, T param, Args &&... args)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        return asyncPublish(EventConverter::convert(space, topic), param, std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    inline QFuture<bool> asyncPublish(EventType type, T param, Args &&... args)
    {
        if (!globalFilterMap.isEmpty()) {
            QVariantList ret;
            makeVariantList(&ret, param, std::forward<Args>(args)...);
            if (globalFiltered(type, ret))
                return QFuture<bool>();
        }

        QReadLocker lk(&rwLock);
        if (Q_LIKELY(dispatcherMap.contains(type))) {
            auto dispatcher { dispatcherMap[type] };
            Q_ASSERT(dispatcher);
            lk.unlock();
            return dispatcher->asyncDispatch(param, std::forward<Args>(args)...);
        }
        return QFuture<bool>();
    }

    inline QFuture<bool> asyncPublish(const QString &space, const QString &topic)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        return asyncPublish(EventConverter::convert(space, topic));
    }

    inline QFuture<bool> asyncPublish(EventType type)
    {
        if (!globalFilterMap.isEmpty() && globalFiltered(type, QVariantList()))
            return QFuture<bool>();

        QReadLocker lk(&rwLock);
        if (Q_LIKELY(dispatcherMap.contains(type))) {
            auto dispatcher { dispatcherMap[type] };
            Q_ASSERT(dispatcher);
            lk.unlock();
            return dispatcher->asyncDispatch();
        }
        return QFuture<bool>();
    }

    bool installGlobalEventFilter(QObject *obj, GlobalFilter filter);
    bool removeGlobalEventFilter(QObject *obj);
    bool globalFiltered(EventType type, const QVariantList &params);

    template<class T, class Func>
    inline bool installEventFilter(const QString &space, const QString &topic, T *obj, Func method)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        if (!installEventFilter(EventConverter::convert(space, topic), obj, std::move(method))) {
            qCWarning(logDPF) << "Topic " << space << ":" << topic << "is invalid";
            return false;
        }
        return true;
    }

    template<class T, class Func>
    inline bool installEventFilter(EventType type, T *obj, Func method)
    {
        if (!isValidEventType(type)) {
            qCWarning(logDPF) << "Event " << type << "is invalid";
            return false;
        }

        QWriteLocker lk(&rwLock);
        if (dispatcherMap.contains(type)) {
            dispatcherMap[type]->appendFilter(obj, method);
        } else {
            DispatcherPtr dispatcher { new EventDispatcher };
            dispatcher->appendFilter(obj, method);
            dispatcherMap.insert(type, dispatcher);
        }
        return true;
    }

    template<class T, class Func>
    inline bool removeEventFilter(const QString &space, const QString &topic, T *obj, Func method)
    {
        Q_ASSERT(topic.startsWith(kSignalStrategePrefix));
        return removeEventFilter(EventConverter::convert(space, topic), obj, std::move(method));
    }

    template<class T, class Func>
    inline bool removeEventFilter(EventType type, T *obj, Func method)
    {
        if (!obj || !method)
            return false;

        QWriteLocker lk(&rwLock);
        if (dispatcherMap.contains(type))
            return dispatcherMap[type]->removeFilter(obj, std::move(method));

        return false;
    }

protected:
    bool unsubscribe(const QString &space, const QString &topic);
    bool unsubscribe(EventType type);

private:
    using DispatcherPtr = QSharedPointer<EventDispatcher>;
    using EventDispatcherMap = QMap<EventType, DispatcherPtr>;
    using GlobalEventFilterMap = QMap<QObject *, GlobalFilter>;

private:
    EventDispatcherMap dispatcherMap;
    GlobalEventFilterMap globalFilterMap;
    QReadWriteLock rwLock;
};

DPF_END_NAMESPACE

#endif   // EVENTDISPATCHER_H
