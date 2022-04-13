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

#include <dfm-base/utils/universalutils.h>

#include <QVariant>

// register type for EventSequenceManager
Q_DECLARE_METATYPE(QVariant *)

DDP_CANVAS_USE_NAMESPACE
DSB_D_USE_NAMESPACE

CanvasModelExtendPrivate::CanvasModelExtendPrivate(CanvasModelExtend *qq)
    : QObject(qq)
    , EventProvider()
    , q(qq)
{
    qRegisterMetaType<QVariant *>();
}

CanvasModelExtendPrivate::~CanvasModelExtendPrivate()
{
    service->unRegisterEvent(this);
}

QVariantHash CanvasModelExtendPrivate::query(int type) const
{
    switch (type) {
    case EventType::kEventSignal:
        return eSignals;
    case EventType::kEventSlot:
        return eSlots;
    case EventType::kSeqSignal:
        return eSeqSig;
    default:
        break;
    }

    return {};
}

CanvasModelExtend::CanvasModelExtend(QObject *parent)
    : QObject(parent)
    , ModelExtendInterface()
    , d(new CanvasModelExtendPrivate(this))
{

}

bool CanvasModelExtend::initEvent()
{
    auto &ctx = dpfInstance.serviceContext();
    d->service = ctx.service<CanvasService>(CanvasService::name());
    Q_ASSERT_X(d->service, "CanvasModelExtend", "CanvasService not found");

    // register filter
    d->eSeqSig.insert(kFilterCanvaModelData,
                  DFMBASE_NAMESPACE::UniversalUtils::registerEventType());

    // register filter event
    d->service->registerEvent(d);
    return true;
}

bool CanvasModelExtend::modelData(const QUrl &url, int role, QVariant *out, void *userData) const
{
    return dpfInstance.eventSequence().run(d->eSeqSig.value(kFilterCanvaModelData).toInt(),
                                    url, role, out, userData);
}
