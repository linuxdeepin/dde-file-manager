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
#include "canvasmanagerbroker_p.h"

DDP_CANVAS_USE_NAMESPACE
Q_DECLARE_METATYPE(QAbstractItemModel **)

CanvasManagerBrokerPrivate::CanvasManagerBrokerPrivate(CanvasManagerBroker *qq)
    : QObject(qq)
    , CanvasEventProvider()
    , q(qq)
{

}

void CanvasManagerBrokerPrivate::registerEvent()
{
    RegCanvasSlotsID(this, kSlotCanvasManagerFileModel);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasManagerFileModel), q, &CanvasManagerBroker::fileInfoModel);

    RegCanvasSlotsID(this, kSlotCanvasManagerUpdate);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasManagerUpdate), q, &CanvasManagerBroker::update);

    RegCanvasSlotsID(this, kSlotCanvasManagerEdit);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasManagerEdit), q, &CanvasManagerBroker::edit);
}

CanvasManagerBroker::CanvasManagerBroker(CanvasManager *canvas, QObject *parent)
    : QObject(parent)
    , d(new CanvasManagerBrokerPrivate(this))
{
    d->canvas = canvas;
}

bool CanvasManagerBroker::init()
{
    return d->initEvent();
}

void CanvasManagerBroker::update()
{
    d->canvas->update();
}

void CanvasManagerBroker::edit(const QUrl &url)
{
    d->canvas->openEditor(url);
}

void CanvasManagerBroker::fileInfoModel(QAbstractItemModel **model)
{
    if (model)
        *model = d->canvas->fileModel();
}


