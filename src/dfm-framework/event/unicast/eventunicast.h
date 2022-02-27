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
#ifndef EVENTUNICAST_H
#define EVENTUNICAST_H

#include "dfm-framework/dfm_framework_global.h"
#include "dfm-framework/event/eventhelper.h"
#include "dfm-framework/event/invokehelper.h"

#include <QFuture>

DPF_BEGIN_NAMESPACE

class EventUnicastFuture
{
public:
    explicit EventUnicastFuture(const QFuture<QVariant> &future);

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

class EventUnicast
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

    EventUnicastFuture asyncSend();
    EventUnicastFuture asyncSend(const QVariantList &params);
    template<class T, class... Args>
    inline EventUnicastFuture asyncSend(T param, Args &&... args)
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
    template<class T, class... Args>
    inline static void makeVariantList(QVariantList *list, T t, Args &&... args)
    {
        *list << QVariant::fromValue(t);
        if (sizeof...(args) > 0)
            packParamsHelper(*list, std::forward<Args>(args)...);
    }

private:
    Connector conn;
    QMutex receiverMutex;
};

class EventUnicastManager
{
    Q_DISABLE_COPY(EventUnicastManager)

public:
    static EventUnicastManager &instance();

    template<class T, class Func>
    [[gnu::hot]] inline void connect(const QString &topic, T *obj, Func method)
    {
        QMutexLocker guard(&mutex);
        if (unicastMap.contains(topic)) {
            unicastMap[topic]->setReceiver(obj, method);
        } else {
            UnicastPtr unicast { new EventUnicast };
            unicast->setReceiver(obj, method);
            unicastMap.insert(topic, unicast);
        }
    }

    void disconnect(const QString &topic);

    template<class T, class... Args>
    [[gnu::hot]] inline QVariant push(const QString &topic, T param, Args &&... args)
    {
        if (Q_LIKELY(unicastMap.contains(topic)))
            return unicastMap[topic]->send(param, std::forward<Args>(args)...);

        return QVariant();
    }

    inline QVariant push(const QString &topic)
    {
        if (Q_LIKELY(unicastMap.contains(topic)))
            return unicastMap[topic]->send();

        return QVariant();
    }

    template<class T, class... Args>
    inline EventUnicastFuture post(const QString &topic, T param, Args &&... args)
    {
        if (Q_LIKELY(unicastMap.contains(topic)))
            return unicastMap[topic]->asyncSend(param, std::forward<Args>(args)...);
        return EventUnicastFuture(QFuture<QVariant>());
    }

private:
    using UnicastPtr = QSharedPointer<EventUnicast>;
    using EventUnicastMap = QMap<QString, UnicastPtr>;

    EventUnicastManager() = default;
    ~EventUnicastManager() = default;

private:
    EventUnicastMap unicastMap;
    QMutex mutex;
};

DPF_END_NAMESPACE

#endif   // EVENTUNICAST_H
