// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASVIEWEXTEND_H
#define CANVASVIEWEXTEND_H

#include "view/viewhookinterface.h"

namespace ddplugin_canvas {

class CanvasViewHook : public QObject, public ViewHookInterface
{
    Q_OBJECT
public:
    explicit CanvasViewHook(QObject *parent = nullptr);
    bool contextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &pos, void *extData = nullptr) const override;
    bool dropData(int viewIndex, const QMimeData *, const QPoint &viewPoint, void *extData = nullptr) const override;
    bool keyPress(int viewIndex, int key, int modifiers, void *extData = nullptr) const override;
    bool mousePress(int viewIndex, int button, const QPoint &viewPos, void *extData = nullptr) const override;
    bool mouseRelease(int viewIndex, int button, const QPoint &viewPos, void *extData = nullptr) const override;
    bool mouseDoubleClick(int viewIndex, int button, const QPoint &viewPos, void *extData = nullptr) const override;
    bool wheel(int viewIndex, const QPoint &angleDelta, void *extData = nullptr) const override;
    bool startDrag(int viewIndex, int supportedActions, void *extData = nullptr) const override;
    bool dragEnter(int viewIndex, const QMimeData *, void *extData = nullptr) const override;
    bool dragMove(int viewIndex, const QMimeData *, const QPoint &viewPos, void *extData = nullptr) const override;
    bool dragLeave(int viewIndex, const QMimeData *, void *extData = nullptr) const override;
    bool keyboardSearch(int viewIndex, const QString &search, void *extData = nullptr) const override;
    bool drawFile(int viewIndex, const QUrl &file, QPainter *painter, const QStyleOptionViewItem *option, void *extData = nullptr) const override;
    bool shortcutkeyPress(int viewIndex, int key, int modifiers, void *extData = nullptr) const override;
};

}

#endif   // CANVASVIEWHOOK_H
