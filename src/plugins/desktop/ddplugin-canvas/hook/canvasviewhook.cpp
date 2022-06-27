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
#include "canvasviewhook.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-framework/dpf.h>

#include <QUrl>
#include <QMimeData>
#include <QPoint>
#include <QPainter>
#include <QStyleOptionViewItem>

Q_DECLARE_METATYPE(const QMimeData *)
Q_DECLARE_METATYPE(QMimeData *)
Q_DECLARE_METATYPE(const QStyleOptionViewItem *)

using namespace ddplugin_canvas;

#define CanvasViewRunHook(topic, args...) \
            dpfHookSequence->run(QT_STRINGIFY(DDP_CANVAS_NAMESPACE), QT_STRINGIFY2(topic), ##args)

CanvasViewHook::CanvasViewHook(QObject *parent)
    : QObject(parent), ViewHookInterface()
{
}

bool CanvasViewHook::contextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &pos, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_ContextMenu, viewIndex, dir, files, pos, extData);
}

bool CanvasViewHook::dropData(int viewIndex, const QMimeData *md, const QPoint &viewPos, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_DropData, viewIndex, md, viewPos, extData);
}

bool CanvasViewHook::keyPress(int viewIndex, int key, int modifiers, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_KeyPress, viewIndex, key, modifiers, extData);
}

bool CanvasViewHook::mousePress(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_MousePress, viewIndex, button, viewPos, extData);
}

bool CanvasViewHook::mouseRelease(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_MouseRelease, viewIndex, button, viewPos, extData);
}

bool CanvasViewHook::mouseDoubleClick(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_MouseDoubleClick, viewIndex, button, viewPos, extData);
}

bool CanvasViewHook::wheel(int viewIndex, const QPoint &angleDelta, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_Wheel, viewIndex, angleDelta, extData);
}

bool CanvasViewHook::startDrag(int viewIndex, int supportedActions, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_StartDrag, viewIndex, supportedActions, extData);
}

bool CanvasViewHook::dragEnter(int viewIndex, const QMimeData *mime, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_DragEnter, viewIndex, mime, extData);
}

bool CanvasViewHook::dragMove(int viewIndex, const QMimeData *mime, const QPoint &viewPos, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_DragMove, viewIndex, mime, viewPos, extData);
}

bool CanvasViewHook::dragLeave(int viewIndex, const QMimeData *mime, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_DragLeave, viewIndex, mime, extData);
}

bool CanvasViewHook::keyboardSearch(int viewIndex, const QString &search, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_KeyboardSearch, viewIndex, search, extData);
}

bool CanvasViewHook::drawFile(int viewIndex, const QUrl &file, QPainter *painter, const QStyleOptionViewItem *option, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_DrawFile, viewIndex, file, painter, option, extData);
}
