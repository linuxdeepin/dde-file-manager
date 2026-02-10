// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasgridshell.h"

#include <dfm-framework/dpf.h>

#include <QPoint>

Q_DECLARE_METATYPE(QPoint *)

using namespace ddplugin_organizer;

#define CanvasGridPush(topic) \
        dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic))

#define CanvasGridPush2(topic, args...) \
        dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic), ##args)

CanvasGridShell::CanvasGridShell(QObject *parent)
    : QObject(parent)
{

}

CanvasGridShell::~CanvasGridShell()
{

}

bool CanvasGridShell::initialize()
{
    return true;
}

QString CanvasGridShell::item(int index, const QPoint &gridPos)
{
    return CanvasGridPush2(slot_CanvasGrid_Item, index, gridPos).toString();
}

void CanvasGridShell::tryAppendAfter(const QStringList &items, int index, const QPoint &begin)
{
    CanvasGridPush2(slot_CanvasGrid_TryAppendAfter, items, index, begin);
}

int CanvasGridShell::point(const QString &item, QPoint *pos)
{
    return CanvasGridPush2(slot_CanvasGrid_Point, item, pos).toInt();
}
