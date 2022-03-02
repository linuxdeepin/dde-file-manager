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
#ifndef EVENTINFORMANT_H
#define EVENTINFORMANT_H

#include "services/desktop/dd_service_global.h"
#include "services/desktop/event/eventprovider.h"

#include <QObject>
#include <QReadWriteLock>

DSB_D_BEGIN_NAMESPACE

class EventInformant : public EventProvider
{
public:
    QVariantHash eventSignals() const override;
    QVariantHash eventSlots() const override;
    virtual bool registerEvent(EventProvider *);
    virtual void unRegisterEvent(EventProvider *);
protected:
    QList<EventProvider *> providers;
    mutable QReadWriteLock locker;
};

DSB_D_END_NAMESPACE

#endif // EVENTINFORMANT_H
