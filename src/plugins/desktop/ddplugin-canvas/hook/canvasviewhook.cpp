// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

bool CanvasViewHook::dropData(int viewIndex, const QMimeData *md, const QPoint &viewPoint, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_DropData, viewIndex, md, viewPoint, extData);
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

bool CanvasViewHook::shortcutkeyPress(int viewIndex, int key, int modifiers, void *extData) const
{
    return CanvasViewRunHook(hook_CanvasView_ShortcutKeyPress, viewIndex, key, modifiers, extData);
}
