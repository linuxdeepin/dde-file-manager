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
#include "eventchannel.h"

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
