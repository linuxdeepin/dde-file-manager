/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef EVENTSEQUENCE_H
#define EVENTSEQUENCE_H

#include "dfm-framework/dfm_framework_global.h"
#include "dfm-framework/event/eventhelper.h"
#include "dfm-framework/event/invokehelper.h"

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
    inline bool traversal(T param, Args &&... args)
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
                    qWarning() << "Cannot remove: " << handler.objectIndex->objectName();
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
    Q_DISABLE_COPY(EventSequenceManager)

public:
    static EventSequenceManager &instance();

    template<class T, class Func>
    inline bool follow(const QString &space, const QString &topic, T *obj, Func method)
    {
        Q_ASSERT(topic.startsWith(kHookStrategePrefix));
        if (Q_UNLIKELY(!follow(EventConverter::convert(space, topic), obj, std::move(method)))) {
            qCritical() << "Topic " << space << ":" << topic << "is invalid";
            return false;
        }
        return true;
    }

    template<class T, class Func>
    inline bool follow(EventType type, T *obj, Func method)
    {
        if (!isValidEventType(type)) {
            qCritical() << "Event " << type << "is invalid";
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
    inline bool run(const QString &space, const QString &topic, T param, Args &&... args)
    {
        Q_ASSERT(topic.startsWith(kHookStrategePrefix));
        return run(EventConverter::convert(space, topic), param, std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    inline bool run(EventType type, T param, Args &&... args)
    {
        QReadLocker lk(&rwLock);
        if (Q_LIKELY(sequenceMap.contains(type))) {
            auto dispatcher = sequenceMap.value(type);
            lk.unlock();
            if (dispatcher)
                return dispatcher->traversal(param, std::forward<Args>(args)...);
        }
        return false;
    }

    inline bool run(const QString &space, const QString &topic)
    {
        Q_ASSERT(topic.startsWith(kHookStrategePrefix));
        return run(EventConverter::convert(space, topic));
    }

    inline bool run(EventType type)
    {
        QReadLocker lk(&rwLock);
        if (Q_LIKELY(sequenceMap.contains(type))) {
            auto dispatcher = sequenceMap.value(type);
            lk.unlock();
            if (dispatcher)
                return dispatcher->traversal();
        }
        return false;
    }

protected:
    bool unfollow(const QString &space, const QString &topic);
    bool unfollow(EventType type);

private:
    using SequencePtr = QSharedPointer<EventSequence>;
    using EventSequenceMap = QMap<EventType, SequencePtr>;

    EventSequenceManager() = default;
    ~EventSequenceManager() = default;

private:
    EventSequenceMap sequenceMap;
    QReadWriteLock rwLock;
};

DPF_END_NAMESPACE

#endif   // EVENTSEQUENCE_H
