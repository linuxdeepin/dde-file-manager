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
#ifndef VIEWEXTENDINTERFACE_H
#define VIEWEXTENDINTERFACE_H

#include "ddplugin_canvas_global.h"

#include <QObject>

class QMimeData;
class QPainter;
class QStyleOptionViewItem;
DDP_CANVAS_BEGIN_NAMESPACE

class ViewExtendInterface
{
public:
    explicit ViewExtendInterface();
    virtual ~ViewExtendInterface();
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
};

DDP_CANVAS_END_NAMESPACE

#endif // VIEWEXTENDINTERFACE_H
