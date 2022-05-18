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
#ifndef CANVASVIEWBROKER_P_H
#define CANVASVIEWBROKER_P_H

#include "canvasviewbroker.h"
#include "canvaseventprovider.h"
#include "canvasmanager.h"
#include "view/canvasview.h"

DDP_CANVAS_BEGIN_NAMESPACE

static constexpr char kSlotCanvasViewVisualRect[] = "CanvasView_Method_visualRect";
static constexpr char kSlotCanvasViewRefresh[] = "CanvasView_Method_refresh";
static constexpr char kSlotCanvasViewUpdate[] = "CanvasView_Method_update";
static constexpr char kSlotCanvasViewSelect[] = "CanvasView_Method_select";
static constexpr char kSlotCanvasViewSelectedUrls[] = "CanvasView_Method_selectedUrls";

class CanvasViewBrokerPrivate : public QObject, public CanvasEventProvider
{
    Q_OBJECT
public:
    explicit CanvasViewBrokerPrivate(CanvasViewBroker *);
    ~CanvasViewBrokerPrivate();
    QSharedPointer<CanvasView> view(int idx);
protected:
    void registerEvent() override;
public:
    CanvasManager *mrg = nullptr;
private:
    CanvasViewBroker *q;
};

DDP_CANVAS_END_NAMESPACE
#endif // CANVASVIEWBROKER_P_H
