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
    bool dropData(int viewIndex, const QMimeData *, const QPoint &viewPos, void *extData = nullptr) const override;
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
};

}

#endif   // CANVASVIEWHOOK_H
