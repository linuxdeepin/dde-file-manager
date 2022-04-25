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
#include "canvasgridbroker_p.h"

Q_DECLARE_METATYPE(QStringList *)

DDP_CANVAS_USE_NAMESPACE

CanvasGridBrokerPrivate::CanvasGridBrokerPrivate(CanvasGridBroker *qq)
    : QObject(qq)
    , CanvasEventProvider()
    , q(qq)
{

}

CanvasGridBrokerPrivate::~CanvasGridBrokerPrivate()
{

}

void CanvasGridBrokerPrivate::registerEvent()
{
    RegCanvasSlotsID(this, kSlotCanvasGridItems);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasGridItems), q, &CanvasGridBroker::items);
}

CanvasGridBroker::CanvasGridBroker(CanvasGrid *grid, QObject *parent)
    : QObject(parent)
    , d(new CanvasGridBrokerPrivate(this))
{
    d->grid = grid;
}

bool CanvasGridBroker::init()
{
    return d->initEvent();
}

void CanvasGridBroker::items(int index, QStringList *ret)
{
    if (ret)
        *ret = d->grid->items(index);
}

