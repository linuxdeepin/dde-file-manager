// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmabstracteventhandler.h"
#include "dfmeventdispatcher.h"

DFM_BEGIN_NAMESPACE

QObject *DFMAbstractEventHandler::object() const
{
    return Q_NULLPTR;
}

DFMAbstractEventHandler::DFMAbstractEventHandler(bool autoInstallHandler)
{
    if (autoInstallHandler)
        DFMEventDispatcher::instance()->installEventHandler(this);
}

DFMAbstractEventHandler::~DFMAbstractEventHandler()
{
    DFMEventDispatcher::instance()->removeEventHandler(this);
    DFMEventDispatcher::instance()->removeEventFilter(this);
}

bool DFMAbstractEventHandler::fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData)
{
    Q_UNUSED(event)
    Q_UNUSED(resultData)

    return false;
}

bool DFMAbstractEventHandler::fmEventFilter(const QSharedPointer<DFMEvent> &event, DFMAbstractEventHandler *target, QVariant *resultData)
{
    Q_UNUSED(event)
    Q_UNUSED(target)
    Q_UNUSED(resultData)

    return false;
}

DFM_END_NAMESPACE
