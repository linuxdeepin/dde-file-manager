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
#include "eventunicast.h"

#include <QtConcurrent>

DPF_USE_NAMESPACE

EventUnicastFuture::EventUnicastFuture(const QFuture<QVariant> &future)
    : curFuture(future)
{
}

void EventUnicastFuture::cancel()
{
    curFuture.cancel();
}

bool EventUnicastFuture::isCanceled() const
{
    return curFuture.isCanceled();
}

bool EventUnicastFuture::isStarted() const
{
    return curFuture.isStarted();
}

bool EventUnicastFuture::isFinished() const
{
    return curFuture.isFinished();
}

bool EventUnicastFuture::isRunning() const
{
    return curFuture.isRunning();
}

void EventUnicastFuture::waitForFinished()
{
    curFuture.waitForFinished();
}

QVariant EventUnicastFuture::result() const
{
    return curFuture.result();
}

/*!
 * \class EventUnicastResult
 * \brief
 */

QVariant EventUnicast::send()
{
    return send(QVariantList());
}

QVariant EventUnicast::send(const QVariantList &params)
{
    if (!conn)
        return QVariant();

    return conn(params);
}

EventUnicastFuture EventUnicast::asyncSend()
{
    return asyncSend(QVariantList());
}

EventUnicastFuture EventUnicast::asyncSend(const QVariantList &params)
{
    return EventUnicastFuture(QtConcurrent::run([this, params]() {
        return this->send(params);
    }));
}

/*!
 * \class EventUnicastManager
 * \brief
 */

EventUnicastManager &EventUnicastManager::instance()
{
    static EventUnicastManager instance;
    return instance;
}

void EventUnicastManager::disconnect(const QString &topic)
{
    QWriteLocker guard(&rwLock);
    if (unicastMap.contains(topic))
        unicastMap.remove(topic);
}
