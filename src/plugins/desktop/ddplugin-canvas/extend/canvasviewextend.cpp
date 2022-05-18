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

#include <QUrl>
#include <QMimeData>
#include <QPoint>
#include <QPainter>
#include <QStyleOptionViewItem>

Q_DECLARE_METATYPE(const QMimeData *)
Q_DECLARE_METATYPE(QMimeData *)
Q_DECLARE_METATYPE(const QStyleOptionViewItem *)

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
    RegCanvasSeqSigID(this, kFilterCanvasViewContextMenu);
    RegCanvasSeqSigID(this, kFilterCanvasViewDropData);
    RegCanvasSeqSigID(this, kFilterCanvasViewKeyPress);
    RegCanvasSeqSigID(this, kFilterCanvasViewMousePress);
    RegCanvasSeqSigID(this, kFilterCanvasViewMouseRelease);
    RegCanvasSeqSigID(this, kFilterCanvasViewMouseDoubleClick);
    RegCanvasSeqSigID(this, kFilterCanvasViewWheel);
    RegCanvasSeqSigID(this, kFilterCanvasViewStartDrag);
    RegCanvasSeqSigID(this, kFilterCanvasViewDragEnter);
    RegCanvasSeqSigID(this, kFilterCanvasViewDragMove);
    RegCanvasSeqSigID(this, kFilterCanvasViewDragLeave);
    RegCanvasSeqSigID(this, kFilterCanvasViewKeyboardSearch);
    RegCanvasSeqSigID(this, kFilterCanvasViewDrawFile);
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
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewContextMenu),
                                           viewIndex, dir, files, pos, extData);
}

bool CanvasViewExtend::dropData(int viewIndex, const QMimeData *md, const QPoint &viewPos, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewDropData),
                                           viewIndex, md, viewPos, extData);
}

bool CanvasViewExtend::keyPress(int viewIndex, int key, int modifiers, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewKeyPress),
                                           viewIndex, key, modifiers, extData);
}

bool CanvasViewExtend::mousePress(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewMousePress),
                                           viewIndex, button, viewPos, extData);
}

bool CanvasViewExtend::mouseRelease(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewMouseRelease),
                                           viewIndex, button, viewPos, extData);
}

bool CanvasViewExtend::mouseDoubleClick(int viewIndex, int button, const QPoint &viewPos, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewMouseDoubleClick),
                                           viewIndex, button, viewPos, extData);
}

bool CanvasViewExtend::wheel(int viewIndex, const QPoint &angleDelta, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewWheel),
                                           viewIndex, angleDelta, extData);
}

bool CanvasViewExtend::startDrag(int viewIndex, int supportedActions, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewStartDrag),
                                           viewIndex, supportedActions, extData);
}

bool CanvasViewExtend::dragEnter(int viewIndex, const QMimeData *mime, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewDragEnter),
                                           viewIndex, mime, extData);
}

bool CanvasViewExtend::dragMove(int viewIndex, const QMimeData *mime, const QPoint &viewPos, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewDragMove),
                                           viewIndex, mime, viewPos, extData);
}

bool CanvasViewExtend::dragLeave(int viewIndex, const QMimeData *mime, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewDragLeave),
                                           viewIndex, mime, extData);
}

bool CanvasViewExtend::keyboardSearch(int viewIndex, const QString &search, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewKeyboardSearch),
                                           viewIndex, search, extData);
}

bool CanvasViewExtend::drawFile(int viewIndex, const QUrl &file, QPainter *painter, const QStyleOptionViewItem *option, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasViewDrawFile),
                                           viewIndex, file, painter, option, extData);
}




