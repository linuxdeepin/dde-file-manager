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
#include "canvasmanagerextend_p.h"

DDP_CANVAS_USE_NAMESPACE

CanvasManagerExtendPrivate::CanvasManagerExtendPrivate(CanvasManagerExtend *qq)
    : QObject(qq)
    , CanvasEventProvider()
    , q(qq)
{

}

CanvasManagerExtendPrivate::~CanvasManagerExtendPrivate()
{

}

void CanvasManagerExtendPrivate::registerEvent()
{
    RegCanvasSeqSigID(this, kFilterCanvasManagerWallpaperSetting);
}

CanvasManagerExtend::CanvasManagerExtend(QObject *parent)
    : QObject(parent)
    , CanvasManagerExtendInterface()
    , d(new CanvasManagerExtendPrivate(this))
{

}

bool CanvasManagerExtend::init()
{
    return d->initEvent();
}

void CanvasManagerExtend::requestWallpaperSetting(const QString &screen) const
{
    dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasManagerWallpaperSetting),
                                    screen);
}
