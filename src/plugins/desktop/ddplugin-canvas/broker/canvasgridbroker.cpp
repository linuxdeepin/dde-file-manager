// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasgridbroker.h"
#include "grid/canvasgrid.h"

#include <dfm-framework/event/event.h>

Q_DECLARE_METATYPE(QPoint *)

using namespace ddplugin_canvas;

#define CanvasGridSlot(topic, args...) \
    dpfSlotChannel->connect(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic), this, ##args)

#define CanvasGridDisconnect(topic) \
    dpfSlotChannel->disconnect(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic))

CanvasGridBroker::CanvasGridBroker(CanvasGrid *gridPtr, QObject *parent)
    : QObject(parent), grid(gridPtr)
{
}

CanvasGridBroker::~CanvasGridBroker()
{
    CanvasGridDisconnect(slot_CanvasGrid_Items);
    CanvasGridDisconnect(slot_CanvasGrid_Item);
    CanvasGridDisconnect(slot_CanvasGrid_Point);
    CanvasGridDisconnect(slot_CanvasGrid_TryAppendAfter);
}

bool CanvasGridBroker::init()
{
    CanvasGridSlot(slot_CanvasGrid_Items, &CanvasGridBroker::items);
    CanvasGridSlot(slot_CanvasGrid_Item, &CanvasGridBroker::item);
    CanvasGridSlot(slot_CanvasGrid_Point, &CanvasGridBroker::point);
    CanvasGridSlot(slot_CanvasGrid_TryAppendAfter, &CanvasGridBroker::tryAppendAfter);
    return true;
}

QStringList CanvasGridBroker::items(int index)
{
    return grid->items(index);
}

QString CanvasGridBroker::item(int index, const QPoint &gridPos)
{
    return grid->item(index, gridPos);
}

int CanvasGridBroker::point(const QString &item, QPoint *pos)
{
    if (pos) {
        QPair<int, QPoint> ret;
        if (grid->point(item, ret)) {
            *pos = ret.second;
            return ret.first;
        }
    }
    return -1;
}

void CanvasGridBroker::tryAppendAfter(const QStringList &items, int index, const QPoint &begin)
{
    grid->tryAppendAfter(items, index, begin);
}
