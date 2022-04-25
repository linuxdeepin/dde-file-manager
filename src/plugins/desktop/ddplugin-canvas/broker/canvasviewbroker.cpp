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
#include "canvasviewbroker_p.h"
#include "view/canvasview_p.h"

DDP_CANVAS_USE_NAMESPACE

CanvasViewBrokerPrivate::CanvasViewBrokerPrivate(CanvasViewBroker *qq)
    : QObject(qq)
    , CanvasEventProvider()
    , q(qq)
{

}

CanvasViewBrokerPrivate::~CanvasViewBrokerPrivate()
{

}

QSharedPointer<CanvasView> CanvasViewBrokerPrivate::view(int idx)
{
    // screen num is start with 1
    idx -= 1;
    auto views = mrg->views();
    if (idx > -1 && idx < views.size()) {
        return views.at(idx);
    }

    return nullptr;
}

void CanvasViewBrokerPrivate::registerEvent()
{
    RegCanvasSlotsID(this, kSlotCanvasViewRefresh);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasViewRefresh), q, &CanvasViewBroker::refresh);
}

CanvasViewBroker::CanvasViewBroker(CanvasManager *mrg, QObject *parent)
    : QObject(parent)
    , d(new CanvasViewBrokerPrivate(this))
{
    d->mrg = mrg;
}

bool CanvasViewBroker::init()
{
    return d->initEvent();
}

void CanvasViewBroker::visualRect(int idx, const QUrl &url, QRect *rect)
{
    auto view = d->view(idx);
    if (rect && view) {
        *rect = view->d->visualRect(url.toString());
    }
}

void CanvasViewBroker::refresh(int idx)
{
    if (auto view = d->view(idx))
        view->refresh();
}

void CanvasViewBroker::update(int idx)
{
    if (auto view = d->view(idx))
        view->update();
}

