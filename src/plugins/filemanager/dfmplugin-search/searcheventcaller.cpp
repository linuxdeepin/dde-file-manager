/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#include "searcheventcaller.h"

#include "window/contexts.h"

#include <framework.h>

DSB_FM_USE_NAMESPACE

void SearchEventCaller::sendSetRootUrlEvent(const QUrl &rootUrl, quint64 winIdx)
{
    dpf::Event event;
    event.setTopic(EventTypes::kTopicWindowEvent);
    event.setData(EventTypes::kDataSetRootUrlEvent);
    event.setProperty(EventTypes::kPropertyRootUrl, QVariant(rootUrl));
    event.setProperty(EventTypes::kPropertyKeyWindowIndex, winIdx);
    dpfInstance.eventProxy().pubEvent(event);
}
