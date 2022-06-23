/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "canvasgridshell.h"

#include <dfm-framework/dpf.h>

#include <QPoint>

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
