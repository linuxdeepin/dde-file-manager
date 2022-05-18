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
#include "canvasmodelbroker_p.h"

#include <QModelIndex>
#include <QVariant>

Q_DECLARE_METATYPE(bool *)
Q_DECLARE_METATYPE(QVariant *)
Q_DECLARE_METATYPE(QUrl *)
Q_DECLARE_METATYPE(QModelIndex *)
Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(int *)

DDP_CANVAS_USE_NAMESPACE

CanvasModelBrokerPrivate::CanvasModelBrokerPrivate(CanvasModelBroker *qq)
    : QObject(qq)
    , CanvasEventProvider()
    , q(qq)
{
    qRegisterMetaType<bool *>();
    qRegisterMetaType<QVariant *>();
}

CanvasModelBrokerPrivate::~CanvasModelBrokerPrivate()
{

}

void CanvasModelBrokerPrivate::registerEvent()
{
    RegCanvasSlotsID(this, kSlotCanvasModelRootUrl);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelRootUrl), q, &CanvasModelBroker::rootUrl);

    RegCanvasSlotsID(this, kSlotCanvasModelUrlIndex);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelUrlIndex), q, &CanvasModelBroker::urlIndex);

    RegCanvasSlotsID(this, kSlotCanvasModelIndex);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelIndex), q, &CanvasModelBroker::index);

    RegCanvasSlotsID(this, kSlotCanvasModelFileUrl);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelFileUrl), q, &CanvasModelBroker::fileUrl);

    RegCanvasSlotsID(this, kSlotCanvasModelFiles);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelFiles), q, &CanvasModelBroker::files);

    RegCanvasSlotsID(this, kSlotCanvasModelShowHiddenFiles);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelShowHiddenFiles), q, &CanvasModelBroker::showHiddenFiles);

    RegCanvasSlotsID(this, kSlotCanvasModelsetShowHiddenFiles);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelsetShowHiddenFiles), q, &CanvasModelBroker::setShowHiddenFiles);

    RegCanvasSlotsID(this, kSlotCanvasModelSortOrder);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelSortOrder), q, &CanvasModelBroker::sortOrder);

    RegCanvasSlotsID(this, kSlotCanvasModelSetSortOrder);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelSetSortOrder), q, &CanvasModelBroker::setSortOrder);

    RegCanvasSlotsID(this, kSlotCanvasModelSortRole);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelSortRole), q, &CanvasModelBroker::sortRole);

    RegCanvasSlotsID(this, kSlotCanvasModelSetSortRole);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelSetSortRole), q, &CanvasModelBroker::setSortRole);

    RegCanvasSlotsID(this, kSlotCanvasModelRowCount);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelRowCount), q, &CanvasModelBroker::rowCount);

    RegCanvasSlotsID(this, kSlotCanvasModelData);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelData), q, &CanvasModelBroker::data);

    RegCanvasSlotsID(this, kSlotCanvasModelFetch);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelFetch), q, &CanvasModelBroker::fetch);

    RegCanvasSlotsID(this, kSlotCanvasModelTake);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotCanvasModelTake), q, &CanvasModelBroker::take);
}

CanvasModelBroker::CanvasModelBroker(CanvasProxyModel *model, QObject *parent)
    : QObject(parent)
    , d(new CanvasModelBrokerPrivate(this))
{
    d->model = model;
}

bool CanvasModelBroker::init()
{
    return d->initEvent();
}

void CanvasModelBroker::rootUrl(QUrl *url)
{
    if (url) {
        *url = d->model->rootUrl();
    }
}

void CanvasModelBroker::urlIndex(const QUrl &url, QModelIndex *idx)
{
    if (idx) {
        *idx = d->model->index(url);
    }
}

void CanvasModelBroker::fileUrl(const QModelIndex &index, QUrl *url)
{
    if (url) {
        *url = d->model->fileUrl(index);
    }
}

void CanvasModelBroker::files(QList<QUrl> *urls)
{
    if (urls) {
        *urls = d->model->files();
    }
}

void CanvasModelBroker::showHiddenFiles(bool *show)
{
    if (show) {
        *show = d->model->showHiddenFiles();
    }
}

void CanvasModelBroker::setShowHiddenFiles(bool show)
{
    d->model->setShowHiddenFiles(show);
}

void CanvasModelBroker::sortOrder(int *order)
{
    if (order) {
        *order = d->model->sortOrder();
    }
}

void CanvasModelBroker::setSortOrder(int order)
{
    d->model->setSortOrder(static_cast<Qt::SortOrder>(order));
}

void CanvasModelBroker::sortRole(int *role)
{
    if (role) {
        *role = d->model->sortRole();
    }
}

void CanvasModelBroker::setSortRole(int role, int order)
{
    d->model->setSortRole(role, static_cast<Qt::SortOrder>(order));
}

void CanvasModelBroker::index(int row, QModelIndex *idx)
{
    if (idx) {
        *idx = d->model->index(row);
    }
}

void CanvasModelBroker::rowCount(int *count)
{
    if (count) {
        *count = d->model->rowCount(d->model->rootIndex());
    }
}

void CanvasModelBroker::data(const QUrl &url, int itemRole, QVariant *out)
{
    if (out) {
        *out = d->model->data(d->model->index(url), itemRole);
    }
}

void CanvasModelBroker::sort()
{
    d->model->sort();
}

void CanvasModelBroker::refresh(bool global, int ms)
{
    d->model->refresh(d->model->rootIndex(), global, ms);
}

void CanvasModelBroker::fetch(const QUrl &url, bool *ret)
{
    bool ok = d->model->fetch(url);
    if (ret)
        *ret = ok;
}

void CanvasModelBroker::take(const QUrl &url, bool *ret)
{
    bool ok = d->model->take(url);
    if (ret)
        *ret = ok;
}
