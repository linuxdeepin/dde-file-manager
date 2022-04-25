/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef EVENTPROVIDER_H
#define EVENTPROVIDER_H

#include "services/desktop/dd_service_global.h"

#include <QVariantHash>
#include <QReadWriteLock>

DSB_D_BEGIN_NAMESPACE

namespace EventType {
static constexpr int kEventSignal = 0x1;
static constexpr int kEventSlot = 0x2;
static constexpr int kSeqSignal = 0x3;
}

class EventProvider
{
public:
    typedef void (*EventChanged)(int eventType, const QStringList &eventKeys, void *);
public:
    virtual QVariantHash query(int type) const = 0;
    virtual bool monitor(EventChanged func, void *data);
    virtual void unmonitor(EventChanged func);
protected:
    virtual void notify(int eventType, const QStringList &eventKeys) const;
protected:
    explicit EventProvider();
    virtual ~EventProvider();
protected:
    QMap<EventChanged, void *> monitors;
    mutable QReadWriteLock locker;
};

DSB_D_END_NAMESPACE

#endif // EVENTPROVIDER_H
