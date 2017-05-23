/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "dfmabstracteventhandler.h"
#include "dfmeventdispatcher.h"

DFM_BEGIN_NAMESPACE

QObject *DFMAbstractEventHandler::object() const
{
    return Q_NULLPTR;
}

DFMAbstractEventHandler::DFMAbstractEventHandler()
{
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
