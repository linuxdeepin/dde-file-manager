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

#include "canvasmodelextend_p.h"
#include "model/canvasproxymodel.h"

#include <QVariant>
#include <QMimeData>

// register type for EventSequenceManager
Q_DECLARE_METATYPE(QVariant *)
Q_DECLARE_METATYPE(QList<QUrl> *)
Q_DECLARE_METATYPE(const QMimeData *)
Q_DECLARE_METATYPE(QStringList *)
Q_DECLARE_METATYPE(QMimeData *)

DDP_CANVAS_USE_NAMESPACE
DSB_D_USE_NAMESPACE

CanvasModelExtendPrivate::CanvasModelExtendPrivate(CanvasModelExtend *qq)
    : QObject(qq)
    , CanvasEventProvider()
    , q(qq)
{
    qRegisterMetaType<QVariant *>();
}

CanvasModelExtendPrivate::~CanvasModelExtendPrivate()
{

}

void CanvasModelExtendPrivate::registerEvent()
{
    RegCanvasSeqSigID(this, kFilterCanvasModelData);
    RegCanvasSeqSigID(this, kFilterCanvasModelDataInserted);
    RegCanvasSeqSigID(this, kFilterCanvasModelDataRemoved);
    RegCanvasSeqSigID(this, kFilterCanvasModelDataRenamed);
    RegCanvasSeqSigID(this, kFilterCanvasModelDataRested);
    RegCanvasSeqSigID(this, kFilterCanvasModelDataChanged);
    RegCanvasSeqSigID(this, kFilterCanvasModelDropMimeData);
    RegCanvasSeqSigID(this, kFilterCanvasModelMimeData);
    RegCanvasSeqSigID(this, kFilterCanvasModelMimeTypes);
    RegCanvasSeqSigID(this, kFilterCanvasModelSortData);
}

CanvasModelExtend::CanvasModelExtend(QObject *parent)
    : QObject(parent)
    , ModelExtendInterface()
    , d(new CanvasModelExtendPrivate(this))
{

}

bool CanvasModelExtend::init()
{
    return d->initEvent();
}

bool CanvasModelExtend::modelData(const QUrl &url, int role, QVariant *out, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasModelData),
                                           url, role, out, extData);
}

bool CanvasModelExtend::dataInserted(const QUrl &url, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasModelDataInserted),
                                           url, extData);
}

bool CanvasModelExtend::dataRemoved(const QUrl &url, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasModelDataRemoved),
                                           url, extData);
}

bool CanvasModelExtend::dataRenamed(const QUrl &oldUrl, const QUrl &newUrl, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasModelDataRenamed),
                                           oldUrl, newUrl, extData);
}

bool CanvasModelExtend::dataRested(QList<QUrl> *urls, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasModelDataRested),
                                           urls, extData);
}

bool CanvasModelExtend::dataChanged(const QUrl &url, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasModelDataChanged),
                                           url, extData);
}

bool CanvasModelExtend::dropMimeData(const QMimeData *data, const QUrl &dir, Qt::DropAction action, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasModelDropMimeData),
                                           data, dir, action, extData);
}

bool CanvasModelExtend::mimeData(const QList<QUrl> &urls, QMimeData *out, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasModelMimeData),
                                           urls, out, extData);
}

bool CanvasModelExtend::mimeTypes(QStringList *types, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasModelMimeTypes),
                                           types, extData);
}

bool CanvasModelExtend::sortData(int role, int order, QList<QUrl> *files, void *extData) const
{
    return dpfInstance.eventSequence().run(GetCanvasSeqSigID(d, kFilterCanvasModelSortData),
                                           role, order, files, extData);
}
