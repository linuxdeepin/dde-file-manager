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

#include "view/viewextendinterface.h"

DDP_CANVAS_BEGIN_NAMESPACE
class CanvasViewExtendPrivate;
class CanvasViewExtend : public QObject, public ViewExtendInterface
{
    Q_OBJECT
    friend class CanvasViewExtendPrivate;
public:
    explicit CanvasViewExtend(QObject *parent = nullptr);
    ~CanvasViewExtend();
    bool initEvent();
    bool contextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &pos, void *extData = nullptr) const override;
    bool dropData(int viewIndex, const QMimeData *, const QPoint &viewPos, void *extData = nullptr) const override;
private:
    CanvasViewExtendPrivate *d;
};

DDP_CANVAS_END_NAMESPACE

#endif // CANVASVIEWEXTEND_H
