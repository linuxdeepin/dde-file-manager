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
#ifndef CANVASEVENTPROVIDER_H
#define CANVASEVENTPROVIDER_H

#include "ddplugin_canvas_global.h"

#include <services/desktop/event/eventprovider.h>
#include <services/desktop/canvas/canvasservice.h>

#include <dfm-base/utils/universalutils.h>

DDP_CANVAS_BEGIN_NAMESPACE

#define RegCanvasEventID(provider, type, name) provider->events(type)->insert(name, DFMBASE_NAMESPACE::UniversalUtils::registerEventType())
#define RegCanvasSignalsID(provider, name) RegCanvasEventID(provider, DSB_D_NAMESPACE::EventType::kEventSignal, name)
#define RegCanvasSlotsID(provider, name) RegCanvasEventID(provider, DSB_D_NAMESPACE::EventType::kEventSlot, name)
#define RegCanvasSeqSigID(provider, name) RegCanvasEventID(provider, DSB_D_NAMESPACE::EventType::kSeqSignal, name)

#define GetCanvasEventID(provider, type, name) provider->events(type)->value(name).toInt()
#define GetCanvasSignalsID(provider, name) GetCanvasEventID(provider, DSB_D_NAMESPACE::EventType::kEventSignal, name)
#define GetCanvasSlotsID(provider, name) GetCanvasEventID(provider, DSB_D_NAMESPACE::EventType::kEventSlot, name)
#define GetCanvasSeqSigID(provider, name) GetCanvasEventID(provider, DSB_D_NAMESPACE::EventType::kSeqSignal, name)

class CanvasEventProvider : public DSB_D_NAMESPACE::EventProvider
{
public:
    explicit CanvasEventProvider();
    ~CanvasEventProvider() override;

public:
    virtual bool initEvent();
    QVariantHash query(int type) const override;
    QVariantHash *events(int type);

protected:
    virtual void registerEvent() = 0;

protected:
    DSB_D_NAMESPACE::CanvasService *service = nullptr;
    QVariantHash eSignals;
    QVariantHash eSlots;
    QVariantHash eSeqSig;
};

DDP_CANVAS_END_NAMESPACE

#endif   // CANVASEVENTPROVIDER_H
