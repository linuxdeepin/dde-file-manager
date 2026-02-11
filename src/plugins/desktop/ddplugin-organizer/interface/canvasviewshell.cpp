// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasviewshell.h"

#include <dfm-framework/dpf.h>

#include <QPoint>
#include <QMimeData>
#include <QMetaMethod>

Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(const QMimeData *)

using namespace ddplugin_organizer;

#define CanvasViewPush(topic) \
    dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic))

#define CanvasViewPush2(topic, args...) \
    dpfSlotChannel->push("ddplugin_canvas", QT_STRINGIFY2(topic), ##args)

#define CanvasViewFollow(topic, args...) \
    dpfHookSequence->follow("ddplugin_canvas", QT_STRINGIFY2(topic), this, ##args)

#define CanvasViewUnfollow(topic, args...) \
    dpfHookSequence->unfollow("ddplugin_canvas", QT_STRINGIFY2(topic), this, ##args)

#define CheckFilterConnected(sig)                                                   \
    {                                                                               \
        if (!isSignalConnected(QMetaMethod::fromSignal(&sig))) {                    \
            fmWarning() << "filter signal was not connected to any object" << #sig; \
            return false;                                                           \
        }                                                                           \
    }

CanvasViewShell::CanvasViewShell(QObject *parent)
    : QObject(parent)
{
}

CanvasViewShell::~CanvasViewShell()
{
    CanvasViewUnfollow(hook_CanvasView_DropData, &CanvasViewShell::eventDropData);
    CanvasViewUnfollow(hook_CanvasView_KeyPress, &CanvasViewShell::eventKeyPress);
    CanvasViewUnfollow(hook_CanvasView_ShortcutKeyPress, &CanvasViewShell::eventShortcutkeyPress);
    CanvasViewUnfollow(hook_CanvasView_Wheel, &CanvasViewShell::eventWheel);
    // CanvasViewUnfollow(hook_CanvasView_MousePress, &CanvasViewShell::eventMousePress);
    CanvasViewUnfollow(hook_CanvasView_ContextMenu, &CanvasViewShell::eventContextMenu);
}

bool CanvasViewShell::initialize()
{
    CanvasViewFollow(hook_CanvasView_DropData, &CanvasViewShell::eventDropData);
    CanvasViewFollow(hook_CanvasView_ShortcutKeyPress, &CanvasViewShell::eventShortcutkeyPress);
    CanvasViewFollow(hook_CanvasView_KeyPress, &CanvasViewShell::eventKeyPress);
    CanvasViewFollow(hook_CanvasView_Wheel, &CanvasViewShell::eventWheel);
    // CanvasViewFollow(hook_CanvasView_MousePress, &CanvasViewShell::eventMousePress);
    CanvasViewFollow(hook_CanvasView_ContextMenu, &CanvasViewShell::eventContextMenu);

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

QAbstractItemView *CanvasViewShell::canvasView(int viewIndex)
{
    return CanvasViewPush2(slot_CanvasManager_View, viewIndex).value<QAbstractItemView *>();
}

bool CanvasViewShell::eventDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData)
{
    Q_UNUSED(extData)
    CheckFilterConnected(CanvasViewShell::filterDropData) return filterDropData(viewIndex, mimeData, viewPoint, extData);
}

bool CanvasViewShell::eventKeyPress(int viewIndex, int key, int modifiers, void *extData)
{
    CheckFilterConnected(CanvasViewShell::filterKeyPress) return filterKeyPress(viewIndex, key, modifiers);
}

bool CanvasViewShell::eventShortcutkeyPress(int viewIndex, int key, int modifiers, void *extData)
{
    Q_UNUSED(extData)
    CheckFilterConnected(CanvasViewShell::filterShortcutkeyPress) return filterShortcutkeyPress(viewIndex, key, modifiers);
}

bool CanvasViewShell::eventWheel(int viewIndex, const QPoint &angleDelta, void *extData)
{
    CheckFilterConnected(CanvasViewShell::filterWheel)
#ifdef DFM_DISABLE_CHANGE_ICON_BY_WHEEL
            if (extData)
    {
        QVariantHash *ext = reinterpret_cast<QVariantHash *>(extData);
        bool ctrl = ext->value("CtrlPressed").toBool();
        return filterWheel(viewIndex, angleDelta, ctrl);
    }
#endif
    return false;
}

bool CanvasViewShell::eventContextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &viewPos, void *extData)
{
    CheckFilterConnected(CanvasViewShell::filterContextMenu) return filterContextMenu(viewIndex, dir, files, viewPos);
}

// bool CanvasViewShell::eventMousePress(int viewIndex, int button, const QPoint &viewPos, void *extData)
//{
//     CheckFilterConnected(CanvasViewShell::filterMousePress)
//     return filterMousePress(viewIndex, button, viewPos);
// }
