// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CANVASVIEWSHELL_H
#define CANVASVIEWSHELL_H

#include "ddplugin_organizer_global.h"

#include <QObject>
#include <QRect>
#include <QUrl>
#include <QAbstractItemView>

class QMimeData;

namespace ddplugin_organizer {

class CanvasInterface;
class CanvasViewShell : public QObject
{
    Q_OBJECT
public:
    explicit CanvasViewShell(QObject *parent = nullptr);
    ~CanvasViewShell();
    bool initialize();

public:
    QPoint gridPos(const int &viewIndex, const QPoint &viewPoint);
    QRect visualRect(int viewIndex, const QUrl &url);
    QRect gridVisualRect(int viewIndex, const QPoint &gridPos);
    QSize gridSize(int viewIndex);
    QAbstractItemView *canvasView(int viewIndex);
signals:   // unqiue and direct signals
    bool filterDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPos, void *extData);
    bool filterShortcutkeyPress(int viewIndex, int key, int modifiers);
    bool filterKeyPress(int viewIndex, int key, int modifiers);
    bool filterWheel(int viewIndex, const QPoint &angleDelta, bool ctrl);
    bool filterContextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &viewPos);
    //bool filterMousePress(int viewIndex, int button, const QPoint &viewPos);

private slots:
    bool eventDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData);
    bool eventKeyPress(int viewIndex, int key, int modifiers, void *extData = nullptr);
    bool eventShortcutkeyPress(int viewIndex, int key, int modifiers, void *extData = nullptr);
    bool eventWheel(int viewIndex, const QPoint &angleDelta, void *extData);
    bool eventContextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &viewPos, void *extData);
    //bool eventMousePress(int viewIndex, int button, const QPoint &viewPos, void *extData);
};

}

#endif   // CANVASVIEWSHELL_H
