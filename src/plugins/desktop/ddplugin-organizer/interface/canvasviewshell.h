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
#ifndef CANVASVIEWSHELL_H
#define CANVASVIEWSHELL_H

#include "ddplugin_organizer_global.h"

#include <QObject>
#include <QRect>
#include <QUrl>

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
signals: // unqiue and direct signals
    bool filterDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPos);
    bool filterShortcutkeyPress(int viewIndex, int key, int modifiers);
    bool filterWheel(int viewIndex, const QPoint &angleDelta, bool ctrl);
    bool filterContextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &viewPos);

private slots:
    bool eventDropData(int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData);
    bool eventShortcutkeyPress(int viewIndex, int key, int modifiers, void *extData = nullptr);
    bool eventWheel(int viewIndex, const QPoint &angleDelta, void *extData);
    bool eventContextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &viewPos, void *extData);
};

}

#endif // CANVASVIEWSHELL_H
