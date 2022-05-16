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
#include "fileinfomodelbroker_p.h"

#include "model/fileinfomodel.h"

Q_DECLARE_METATYPE(QUrl *)
Q_DECLARE_METATYPE(QModelIndex *)
Q_DECLARE_METATYPE(QSharedPointer<DFMBASE_NAMESPACE::LocalFileInfo> *)
Q_DECLARE_METATYPE(QList<QUrl> *)

DDP_CANVAS_USE_NAMESPACE

FileInfoModelBrokerPrivate::FileInfoModelBrokerPrivate(FileInfoModelBroker *qq)
    : QObject(qq)
    , CanvasEventProvider()
    , q(qq)
{

}

void FileInfoModelBrokerPrivate::registerEvent()
{
    RegCanvasSlotsID(this, kSlotFileInfoModelRootUrl);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotFileInfoModelRootUrl), q, &FileInfoModelBroker::rootUrl);

    RegCanvasSlotsID(this, kSlotFileInfoModelRootIndex);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotFileInfoModelRootIndex), q, &FileInfoModelBroker::rootIndex);

    RegCanvasSlotsID(this, kSlotFileInfoModelUrlIndex);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotFileInfoModelUrlIndex), q, &FileInfoModelBroker::urlIndex);

    RegCanvasSlotsID(this, kSlotFileInfoModelIndexUrl);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotFileInfoModelIndexUrl), q, &FileInfoModelBroker::indexUrl);

    RegCanvasSlotsID(this, kSlotFileInfoModelFiles);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotFileInfoModelFiles), q, &FileInfoModelBroker::files);

    RegCanvasSlotsID(this, kSlotFileInfoModelFileInfo);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotFileInfoModelFileInfo), q, &FileInfoModelBroker::fileInfo);

    RegCanvasSlotsID(this, kSlotFileInfoModelRefresh);
    dpfInstance.eventDispatcher().subscribe(GetCanvasSlotsID(this, kSlotFileInfoModelRefresh), q, &FileInfoModelBroker::refresh);

    // signal
    RegCanvasSignalsID(this, kSignalFileInfoModelDataReplaced);
}


FileInfoModelBroker::FileInfoModelBroker(FileInfoModel *model, QObject *parent)
    : QObject(parent)
    , d(new FileInfoModelBrokerPrivate(this))
{

}

bool FileInfoModelBroker::init()
{
    return d->initEvent();
}

void FileInfoModelBroker::rootUrl(QUrl *url)
{
    if (url)
        *url = d->model->rootUrl();
}

void FileInfoModelBroker::rootIndex(QModelIndex *root)
{
    if (root)
        *root = d->model->rootIndex();
}

void FileInfoModelBroker::urlIndex(const QUrl &url, QModelIndex *index)
{
    if (index)
        *index = d->model->index(url);
}

void FileInfoModelBroker::fileInfo(const QModelIndex &index, DFMLocalFileInfoPointer *file)
{
    if (file)
        *file = d->model->fileInfo(index);
}

void FileInfoModelBroker::indexUrl(const QModelIndex &index, QUrl *url)
{
    if (url)
        *url = d->model->fileUrl(index);
}

void FileInfoModelBroker::files(QList<QUrl> *urls)
{
    if (urls)
        *urls = d->model->files();
}

void FileInfoModelBroker::refresh(const QModelIndex &parent)
{
    d->model->refresh(parent);
}

