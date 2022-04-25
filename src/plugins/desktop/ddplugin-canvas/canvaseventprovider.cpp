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
#include "canvaseventprovider.h"

DDP_CANVAS_USE_NAMESPACE
DSB_D_USE_NAMESPACE

CanvasEventProvider::CanvasEventProvider() : EventProvider()
{

}

CanvasEventProvider::~CanvasEventProvider()
{
    service->unRegisterEvent(this);
}

bool CanvasEventProvider::initEvent()
{
    auto &ctx = dpfInstance.serviceContext();
    service = ctx.service<CanvasService>(CanvasService::name());
    Q_ASSERT_X(service, "CanvasEventProvider", "CanvasService not found");

    // register event
    registerEvent();

    // register event to service
    service->registerEvent(this);
    return true;
}

QVariantHash CanvasEventProvider::query(int type) const
{
    switch (type) {
    case EventType::kEventSignal:
        return eSignals;
    case EventType::kEventSlot:
        return eSlots;
    case EventType::kSeqSignal:
        return eSeqSig;
    default:
        break;
    }

    return {};
}

QVariantHash *CanvasEventProvider::events(int type)
{
    switch (type) {
    case EventType::kEventSignal:
        return &eSignals;
    case EventType::kEventSlot:
        return &eSlots;
    case EventType::kSeqSignal:
        return &eSeqSig;
    default:
        break;
    }

    return nullptr;
}
