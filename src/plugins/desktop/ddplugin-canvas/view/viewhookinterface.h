// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWHOOKINTERFACE_H
#define VIEWHOOKINTERFACE_H

#include "ddplugin_canvas_global.h"

#include <QObject>

class QMimeData;
class QPainter;
class QStyleOptionViewItem;
namespace ddplugin_canvas {

class ViewHookInterface
{
public:
    explicit ViewHookInterface();
    virtual ~ViewHookInterface();
public:
    virtual bool contextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &viewPos, void *extData = nullptr) const;
    virtual bool dropData(int viewIndex, const QMimeData *, const QPoint &viewPos, void *extData = nullptr) const;
    virtual bool keyPress(int viewIndex, int key, int modifiers, void *extData = nullptr) const;
    virtual bool mousePress(int viewIndex, int button, const QPoint &viewPos, void *extData = nullptr) const;
    virtual bool mouseRelease(int viewIndex, int button, const QPoint &viewPos, void *extData = nullptr) const;
    virtual bool mouseDoubleClick(int viewIndex, int button, const QPoint &viewPos, void *extData = nullptr) const;
    virtual bool wheel(int viewIndex, const QPoint &angleDelta, void *extData = nullptr) const;
    virtual bool startDrag(int viewIndex, int supportedActions, void *extData = nullptr) const;
    virtual bool dragEnter(int viewIndex, const QMimeData *, void *extData = nullptr) const;
    virtual bool dragMove(int viewIndex, const QMimeData *, const QPoint &viewPos, void *extData = nullptr) const;
    virtual bool dragLeave(int viewIndex, const QMimeData *, void *extData = nullptr) const;
    virtual bool keyboardSearch(int viewIndex, const QString &search, void *extData = nullptr) const;
    virtual bool drawFile(int viewIndex, const QUrl &file, QPainter *painter, const QStyleOptionViewItem *option, void *extData = nullptr) const;
    virtual bool shortcutkeyPress(int viewIndex, int key, int modifiers, void *extData = nullptr) const;
    virtual bool shortcutAction(int viewIndex, int keySequence, void *extData = nullptr) const;
};

}

#endif // VIEWHOOKINTERFACE_H
