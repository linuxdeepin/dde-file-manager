// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EVENTCHANNEL_H
#define EVENTCHANNEL_H

#include <dfm-framework/dfm_framework_global.h>
#include <dfm-framework/event/eventhelper.h>
#include <dfm-framework/event/invokehelper.h>

#include <QFuture>
#include <QReadWriteLock>

DPF_BEGIN_NAMESPACE

class EventChannelFuture
{
public:
    explicit EventChannelFuture(const QFuture<QVariant> &future);

    void cancel();
    bool isCanceled() const;

    bool isStarted() const;
    bool isFinished() const;
    bool isRunning() const;

    void waitForFinished();
    QVariant result() const;

private:
    QFuture<QVariant> curFuture;
};

class EventChannel
{
public:
    using Connector = std::function<QVariant(const QVariantList &)>;

    QVariant send();
    QVariant send(const QVariantList &params);
    template<class T, class... Args>
    inline QVariant send(T param, Args &&... args)
    {
        QVariantList ret;
        makeVariantList(&ret, param, std::forward<Args>(args)...);
        return send(ret);
    }

    EventChannelFuture asyncSend();
    EventChannelFuture asyncSend(const QVariantList &params);
    template<class T, class... Args>
    inline EventChannelFuture asyncSend(T param, Args &&... args)
    {
        QVariantList ret;
        makeVariantList(&ret, param, std::forward<Args>(args)...);
        return asyncSend(ret);
    }

    template<class T, class Func>
    inline void setReceiver(T *obj, Func method)
    {
        static_assert(std::is_base_of<QObject, T>::value, "Template type T must be derived QObject");
        static_assert(!std::is_pointer<T>::value, "Receiver::bind's template type T must not be a pointer type");

        QMutexLocker guard(&receiverMutex);
        conn = [obj, method](const QVariantList &args) -> QVariant {
            EventHelper<decltype(method)> helper = (EventHelper<decltype(method)>(obj, method));
            return helper.invoke(args);
        };
    }

private:
    Connector conn;
    QMutex receiverMutex;
};

class EventChannelManager
{
public:
    template<class T, class Func>
    inline bool connect(const QString &space, const QString &topic, T *obj, Func method)
    {
        Q_ASSERT(topic.startsWith(kSlotStrategePrefix));
        if (Q_UNLIKELY(!connect(EventConverter::convert(space, topic), obj, std::move(method)))) {
            qCWarning(logDPF) << "Topic " << space << ":" << topic << "is invalid";
            return false;
        }
        return true;
    }

    template<class T, class Func>
    [[gnu::hot]] inline bool connect(EventType type, T *obj, Func method)
    {
        if (!isValidEventType(type)) {
            qCWarning(logDPF) << "Event " << type << "is invalid";
            return false;
        }

        QWriteLocker guard(&rwLock);
        if (channelMap.contains(type)) {
            channelMap[type]->setReceiver(obj, method);
        } else {
            ChannelPtr Channel { new EventChannel };
            Channel->setReceiver(obj, method);
            channelMap.insert(type, Channel);
        }
        return true;
    }

    bool disconnect(const QString &space, const QString &topic);
    bool disconnect(const EventType &type);

    template<class T, class... Args>
    inline QVariant push(const QString &space, const QString &topic, T param, Args &&... args)
    {
        Q_ASSERT(topic.startsWith(kSlotStrategePrefix));
        threadEventAlert(space, topic);
        return push(EventConverter::convert(space, topic), param, std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    [[gnu::hot]] inline QVariant push(EventType type, T param, Args &&... args)
    {
        threadEventAlert(type);
        QReadLocker guard(&rwLock);
        if (Q_LIKELY(channelMap.contains(type))) {
            auto channel = channelMap.value(type);
            guard.unlock();
            return channel->send(param, std::forward<Args>(args)...);
        }
        return QVariant();
    }

    inline QVariant push(const QString &space, const QString &topic)
    {
        Q_ASSERT(topic.startsWith(kSlotStrategePrefix));
        threadEventAlert(space, topic);
        return push(EventConverter::convert(space, topic));
    }

    inline QVariant push(const EventType &type)
    {
        threadEventAlert(type);
        QReadLocker guard(&rwLock);
        if (Q_LIKELY(channelMap.contains(type))) {
            auto channel = channelMap.value(type);
            guard.unlock();
            if (channel)
                return channel->send();
        }

        return QVariant();
    }

    template<class T, class... Args>
    inline EventChannelFuture post(const QString &space, const QString &topic, T param, Args &&... args)
    {
        Q_ASSERT(topic.startsWith(kSlotStrategePrefix));
        return post(EventConverter::convert(space, topic), param, std::forward<Args>(args)...);
    }

    template<class T, class... Args>
    inline EventChannelFuture post(EventType type, T param, Args &&... args)
    {
        QReadLocker guard(&rwLock);
        if (Q_LIKELY(channelMap.contains(type))) {
            auto channel { channelMap[type] };
            guard.unlock();
            return channel->asyncSend(param, std::forward<Args>(args)...);
        }
        return EventChannelFuture(QFuture<QVariant>());
    }

    inline EventChannelFuture post(const QString &space, const QString &topic)
    {
        Q_ASSERT(topic.startsWith(kSlotStrategePrefix));
        return post(EventConverter::convert(space, topic));
    }

    inline EventChannelFuture post(const EventType &type)
    {
        QReadLocker guard(&rwLock);
        if (Q_LIKELY(channelMap.contains(type))) {
            auto channel { channelMap[type] };
            guard.unlock();
            return channel->asyncSend();
        }
        return EventChannelFuture(QFuture<QVariant>());
    }

private:
    using ChannelPtr = QSharedPointer<EventChannel>;
    using EventChannelMap = QMap<EventType, ChannelPtr>;

private:
    EventChannelMap channelMap;
    QReadWriteLock rwLock;
};

DPF_END_NAMESPACE

#endif   // EVENTCHANNEL_H
