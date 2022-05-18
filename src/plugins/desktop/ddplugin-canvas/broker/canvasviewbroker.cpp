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

Q_DECLARE_METATYPE(QRect *)
Q_DECLARE_METATYPE(QList<QUrl> *)

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
    for (auto v : mrg->views())
        if (v->screenNum() == idx)
            return v;

    return nullptr;
}

void CanvasViewBrokerPrivate::registerEvent()
{
    RegCanvasSlotsID(this, kSlotCanvasViewVisualRect);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasViewVisualRect), q, &CanvasViewBroker::visualRect);

    RegCanvasSlotsID(this, kSlotCanvasViewRefresh);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasViewRefresh), q, &CanvasViewBroker::refresh);

    RegCanvasSlotsID(this, kSlotCanvasViewUpdate);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasViewUpdate), q, &CanvasViewBroker::update);

    RegCanvasSlotsID(this, kSlotCanvasViewSelect);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasViewSelect), q, &CanvasViewBroker::select);

    RegCanvasSlotsID(this, kSlotCanvasViewSelectedUrls);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasViewSelectedUrls), q, &CanvasViewBroker::selectedUrls);
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
    if (idx < 0) {
        for (auto view : d->mrg->views())
            view->update();
        return;
    }

    if (auto view = d->view(idx))
        view->update();
}

void CanvasViewBroker::select(const QList<QUrl> &urls)
{
    QItemSelection selection;
    auto m = d->mrg->model();
    for (const QUrl &url: urls) {
        auto index = m->index(url);
        if (index.isValid())
            selection.append(QItemSelectionRange(index));
    }

    d->mrg->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

void CanvasViewBroker::selectedUrls(int idx, QList<QUrl> *urls)
{
    if (urls) {
        auto files = d->mrg->selectionModel()->selectedUrls();

        if (idx < 0) {
            *urls = files;
        } else if (auto view = d->view(idx)) {
            const int num = view->screenNum();

            QStringList items;
            items << GridIns->points(num).keys();
            items << GridIns->overloadItems(num);

            QList<QUrl> viewOn;
            for (const QUrl &url: files) {
                if (items.contains(url.toString()))
                    viewOn.append(url);
            }

            *urls = viewOn;
        }
    }

    return;
}

