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
#include "canvasgridbroker.h"
#include "grid/canvasgrid.h"

#include <dfm-framework/dpf.h>

DDP_CANVAS_USE_NAMESPACE

#define CanvasGridSlot(topic, args...) \
            dpfSlotChannel->connect(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

#define CanvasGridDisconnect(topic) \
            dpfSlotChannel->disconnect(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic))

CanvasGridBroker::CanvasGridBroker(CanvasGrid *gridPtr, QObject *parent)
    : QObject(parent)
    , grid(gridPtr)
{
}

CanvasGridBroker::~CanvasGridBroker()
{
    CanvasGridDisconnect(slot_CanvasGrid_Items);
}

bool CanvasGridBroker::init()
{
    CanvasGridSlot(slot_CanvasGrid_Items, &CanvasGridBroker::items);
    return true;
}

QStringList CanvasGridBroker::items(int index)
{
    return grid->items(index);
}

