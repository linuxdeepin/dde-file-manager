// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/event/eventchannel.h>

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
    if (!conn)
        return QVariant();

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
