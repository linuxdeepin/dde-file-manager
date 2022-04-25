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
#ifndef CANVASGRIDBROKER_P_H
#define CANVASGRIDBROKER_P_H

#include "canvasgridbroker.h"
#include "canvaseventprovider.h"
#include "grid/canvasgrid.h"

DDP_CANVAS_BEGIN_NAMESPACE

static constexpr char kSlotCanvasGridItems[] = "CanvasGrid_Method_items";

class CanvasGridBrokerPrivate : public QObject, public CanvasEventProvider
{
    Q_OBJECT
public:
    explicit CanvasGridBrokerPrivate(CanvasGridBroker *);
    ~CanvasGridBrokerPrivate() override;
protected:
    void registerEvent() override;
public:
    CanvasGrid *grid = nullptr;
private:
    CanvasGridBroker *q;
};

DDP_CANVAS_END_NAMESPACE

#endif // CANVASGRIDBROKER_P_H
