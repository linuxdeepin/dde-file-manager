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
#include "canvasviewextend_p.h"

#include <dfm-base/utils/universalutils.h>

DDP_CANVAS_USE_NAMESPACE
DSB_D_USE_NAMESPACE

CanvasViewExtendPrivate::CanvasViewExtendPrivate(CanvasViewExtend *qq)
    : QObject(qq)
    , CanvasEventProvider()
    , q(qq)
{

}

CanvasViewExtendPrivate::~CanvasViewExtendPrivate()
{

}

void CanvasViewExtendPrivate::registerEvent()
{

}

CanvasViewExtend::CanvasViewExtend(QObject *parent)
    : QObject(parent)
    , ViewExtendInterface()
    , d(new CanvasViewExtendPrivate(this))
{

}

CanvasViewExtend::~CanvasViewExtend()
{

}

bool CanvasViewExtend::init()
{
    return d->initEvent();
}

bool CanvasViewExtend::contextMenu(int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &pos, void *extData) const
{
    return false;
}

bool CanvasViewExtend::dropData(int viewIndex, const QMimeData *md, const QPoint &viewPos, void *extData) const
{
    return false;
}




