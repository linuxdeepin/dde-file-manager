// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTSEQUENCE_H
#define EVENTSEQUENCE_H

#include <dfm-framework/dfm_framework_global.h>
#include <dfm-framework/event/eventhelper.h>
#include <dfm-framework/event/invokehelper.h>

#include <QMutex>
#include <QReadWriteLock>
#include <QSharedPointer>

#include <type_traits>

DPF_BEGIN_NAMESPACE

class EventSequence
{
public:
    using Sequence = std::function<bool(const QVariantList &)>;
    using HandlerList = QList<EventHandler<Sequence>>;

    bool traversal();
    bool traversal(const QVariantList &params);
    template<class T, class... Args>
    inline bool traversal(T param, Args &&...args)
    {
        QVariantList ret;
        makeVariantList(&ret, param, std::forward<Args>(args)...);
        return traversal(ret);
    }

    template<class T, class Func>
    inline void append(T *obj, Func method)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        static_assert(!std::is_pointer<T>::value, "Receiver::bind's template type T must not be a pointer type");
#if __cplusplus > 201703L
        static_assert(std::is_same_v<bool, ReturnType<decltype(method)>>, "The return value of template method must is bool");
#elif __cplusplus > 201103L
        static_assert(std::is_same<bool, ReturnType<decltype(method)>>::value, "Template method's ReturnType must is bool");
#endif

        QMutexLocker guard(&sequenceMutex);
        auto func = [obj, method](const QVariantList &args) -> bool {
            EventHelper<decltype(method)> helper = (EventHelper<decltype(method)>(obj, method));
            return helper.invoke(args).toBool();
        };
        list.push_back(EventHandler<Sequence> { obj, memberFunctionVoidCast(method), func });
    }

    template<class T, class Func>
    inline bool remove(T *obj, Func method)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        static_assert(!std::is_pointer<T>::value, "Receiver::bind's template type T must not be a pointer type");
#if __cplusplus > 201703L
        static_assert(std::is_same_v<bool, ReturnType<decltype(method)>>, "The return value of template method must is bool");
#elif __cplusplus > 201103L
        static_assert(std::is_same<bool, ReturnType<decltype(method)>>::value, "Template method's ReturnType must is bool");
#endif

        bool ret { true };
        QMutexLocker guard(&sequenceMutex);
        for (auto handler : list) {
            if (handler.compare(obj, method)) {

                if (!list.removeOne(handler)) {
                    qCWarning(logDPF) << "Cannot remove: " << handler.objectIndex->objectName();
                    ret = false;
                }
            }
        }

        return ret;
    }

private:
    HandlerList list {};
    QMutex sequenceMutex;
};

class EventSequenceManager
{
public:
    template<class T, class Func>
    inline bool follow(const QString &space, const QString &topic, T *obj, Func method)
    {
        Q_ASSERT(topic.startsWith(kHookStrategePrefix));
        if (Q_UNLIKELY(!follow(EventConverter::convert(space, topic), obj, std::move(method)))) {
            qCWarning(logDPF) << "Topic " << space << ":" << topic << "is invalid";
            return false;
        }
        return true;
    }

    template<class T, class Func>
    inline bool follow(EventType type, T *obj, Func method)
    {
        if (!isValidEventType(type)) {
            qCWarning(logDPF) << "Event " << type << "is invalid";
            return false;
        }

        QWriteLocker lk(&rwLock);
        if (sequenceMap.contains(type)) {
            sequenceMap[type]->append(obj, method);
        } else {
            SequencePtr sequence { new EventSequence };
            sequence->append(obj, method);
            sequenceMap.insert(type, sequence);
        }
        return true;
    }

    template<class T, class Func>
    bool unfollow(const QString &space, const QString &topic, T *obj, Func method)
    {
        Q_ASSERT(topic.startsWith(kHookStrategePrefix));
        return unfollow(EventConverter::convert(space, topic), obj, std::move(method));
    }

    template<class T, class Func>
    bool unfollow(EventType type, T *obj, Func method)
    {
        if (!obj || !method)
            return false;

        QWriteLocker lk(&rwLock);
        if (sequenceMap.contains(type))
            return sequenceMap[type]->remove(obj, std::move(method));

        return false;
    }

    template<class T, class... Args>
    inline bool run(const QString &space, const QString &topic, T param, Args &&...args)
    {
        Q_ASSERT(topic.startsWith(kHookStrategePrefix));
        threadEventAlert(space, topic);
        return run(EventConverter::convert(space, topic), param, std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    inline bool run(EventType type, T param, Args &&...args)
    {
        threadEventAlert(type);
        QReadLocker lk(&rwLock);
        if (Q_LIKELY(sequenceMap.contains(type))) {
            auto sequence = sequenceMap.value(type);
            lk.unlock();
            if (sequence)
                return sequence->traversal(param, std::forward<Args>(args)...);
        }
        return false;
    }

    inline bool run(const QString &space, const QString &topic)
    {
        Q_ASSERT(topic.startsWith(kHookStrategePrefix));
        threadEventAlert(space, topic);
        return run(EventConverter::convert(space, topic));
    }

    inline bool run(EventType type)
    {
        threadEventAlert(type);
        QReadLocker lk(&rwLock);
        if (Q_LIKELY(sequenceMap.contains(type))) {
            auto sequence = sequenceMap.value(type);
            lk.unlock();
            if (sequence)
                return sequence->traversal();
        }
        return false;
    }

protected:
    bool unfollow(const QString &space, const QString &topic);
    bool unfollow(EventType type);

private:
    using SequencePtr = QSharedPointer<EventSequence>;
    using EventSequenceMap = QMap<EventType, SequencePtr>;

private:
    EventSequenceMap sequenceMap;
    QReadWriteLock rwLock;
};

DPF_END_NAMESPACE

#endif   // EVENTSEQUENCE_H
