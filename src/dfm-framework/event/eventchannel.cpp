// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/event/eventchannel.h>
#include <dfm-framework/lifecycle/lifecycle.h>

#include <QtConcurrent>

DPF_USE_NAMESPACE

EventChannelFuture::EventChannelFuture(const QFuture<QVariant> &future)
    : curFuture(future)
{
}

void EventChannelFuture::cancel()
{
    curFuture.cancel();
}

bool EventChannelFuture::isCanceled() const
{
    return curFuture.isCanceled();
}

bool EventChannelFuture::isStarted() const
{
    return curFuture.isStarted();
}

bool EventChannelFuture::isFinished() const
{
    return curFuture.isFinished();
}

bool EventChannelFuture::isRunning() const
{
    return curFuture.isRunning();
}

void EventChannelFuture::waitForFinished()
{
    curFuture.waitForFinished();
}

QVariant EventChannelFuture::result() const
{
    return curFuture.result();
}

/*!
 * \class EventChannelResult
 * \brief
 */

QVariant EventChannel::send()
{
    return send(QVariantList());
}

QVariant EventChannel::send(const QVariantList &params)
{
    // Short-circuit during process shutdown: when plugins are being stopped,
    // Q_GLOBAL_STATIC singletons (e.g. Settings) may already be destroyed in
    // an unspecified order, and invoking cross-plugin handlers risks
    // use-after-free. Push calls originated from plugin dtors / stop() paths
    // are semantically meaningless at this point (no one consumes results).
    if (Q_UNLIKELY(LifeCycle::isShuttingDown())) {
        qCDebug(logDPF) << "EventChannel: dropping send during shutdown";
        return QVariant();
    }

    if (!conn) {
        qCWarning(logDPF) << "EventChannel: no connection available for send operation";
        return QVariant();
    }

    return conn(params);
}

EventChannelFuture EventChannel::asyncSend()
{
    return asyncSend(QVariantList());
}

EventChannelFuture EventChannel::asyncSend(const QVariantList &params)
{
    return EventChannelFuture(QtConcurrent::run([this, params]() {
        return this->send(params);
    }));
}

bool EventChannelManager::disconnect(const QString &space, const QString &topic)
{
    Q_ASSERT(topic.startsWith(kSlotStrategePrefix));
    return disconnect(EventConverter::convert(space, topic));
}

bool EventChannelManager::disconnect(const EventType &type)
{
    QWriteLocker guard(&rwLock);
    if (channelMap.contains(type))
        return channelMap.remove(type) > 0;

    return false;
}
