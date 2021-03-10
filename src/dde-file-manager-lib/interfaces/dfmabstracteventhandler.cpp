/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "dfmabstracteventhandler.h"
#include "dfmeventdispatcher.h"

DFM_BEGIN_NAMESPACE

QObject *DFMAbstractEventHandler::object() const
{
    return Q_NULLPTR;
}

DFMAbstractEventHandler::DFMAbstractEventHandler(bool autoInstallHandler)
{
    if (autoInstallHandler)
        DFMEventDispatcher::instance()->installEventHandler(this);
}

DFMAbstractEventHandler::~DFMAbstractEventHandler()
{
    DFMEventDispatcher::instance()->removeEventHandler(this);
    DFMEventDispatcher::instance()->removeEventFilter(this);
}

bool DFMAbstractEventHandler::fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData)
{
    Q_UNUSED(event)
    Q_UNUSED(resultData)

    return false;
}

bool DFMAbstractEventHandler::fmEventFilter(const QSharedPointer<DFMEvent> &event, DFMAbstractEventHandler *target, QVariant *resultData)
{
    Q_UNUSED(event)
    Q_UNUSED(target)
    Q_UNUSED(resultData)

    return false;
}

DFM_END_NAMESPACE
