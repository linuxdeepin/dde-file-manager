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
#include "canvasviewshell.h"

#include "dfm-framework/dpf.h"

#include <QPoint>
#include <QMimeData>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(const QMimeData *)

DDP_ORGANIZER_USE_NAMESPACE

#define CanvasViewPush(topic) \
        dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic))

#define CanvasViewPush2(topic, args...) \
        dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic), ##args)

#define CanvasViewFollow(topic, args...) \
        dpfHookSequence->follow("ddplugin_canvas", QT_STRINGIFY2(topic), this, ##args)

#define CanvasViewUnfollow(topic, args...) \
        dpfHookSequence->unfollow("ddplugin_canvas", QT_STRINGIFY2(topic), this, ##args)

CanvasViewShell::CanvasViewShell(QObject *parent)
    : QObject(parent)
{

}

CanvasViewShell::~CanvasViewShell()
{
    CanvasViewUnfollow(hook_CanvasView_DropData, &CanvasViewShell::eventDropData);
}

bool CanvasViewShell::initialize()
{
    CanvasViewFollow(hook_CanvasView_DropData, &CanvasViewShell::eventDropData);

    return true;
}

QPoint CanvasViewShell::gridPos(const int &viewIndex, const QPoint &viewPoint)
{
    return CanvasViewPush2(slot_CanvasView_GridPos, viewIndex, viewPoint).toPoint();
}

QRect CanvasViewShell::visualRect(int viewIndex, const QUrl &url)
{
    return CanvasViewPush2(slot_CanvasView_VisualRect, viewIndex, url).toRect();
}

QRect CanvasViewShell::gridVisualRect(int viewIndex, const QPoint &gridPos)
{
    return CanvasViewPush2(slot_CanvasView_GridVisualRect, viewIndex, gridPos).toRect();
}

QSize CanvasViewShell::gridSize(int viewIndex)
{
    return CanvasViewPush2(slot_CanvasView_GridSize, viewIndex).toSize();
}

bool CanvasViewShell::eventDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData)
{
    Q_UNUSED(extData)

    return filterDropData(viewIndex, mimeData, viewPoint);
}


