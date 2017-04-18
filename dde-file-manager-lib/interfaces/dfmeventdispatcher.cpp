/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#include "dfmeventdispatcher.h"
#include "dfmabstracteventhandler.h"

#include <QList>

DFM_BEGIN_NAMESPACE

namespace DFMEventDispatcherData
{
    static QList<DFMAbstractEventHandler*> eventHandler;
    static QList<DFMAbstractEventHandler*> eventFilter;
}

class DFMEventDispatcher_ : public DFMEventDispatcher {};
Q_GLOBAL_STATIC(DFMEventDispatcher_, fmedGlobal)

DFMEventDispatcher *DFMEventDispatcher::instance()
{
    return fmedGlobal;
}

QVariant DFMEventDispatcher::processEvent(const QSharedPointer<DFMEvent> &event)
{
    QVariant result;

    for (DFMAbstractEventHandler *handler : DFMEventDispatcherData::eventFilter) {
        if (handler->fmEventFilter(event, &result))
            return result;
    }

    for (DFMAbstractEventHandler *handler : DFMEventDispatcherData::eventHandler) {
        if (handler->fmEvent(event, &result))
            return result;
    }

    return result;
}

void DFMEventDispatcher::installEventFilter(DFMAbstractEventHandler *handler)
{
    if (!DFMEventDispatcherData::eventFilter.contains(handler))
        DFMEventDispatcherData::eventFilter.append(handler);
}

void DFMEventDispatcher::removeEventFilter(DFMAbstractEventHandler *handler)
{
    DFMEventDispatcherData::eventFilter.removeOne(handler);
}

DFMEventDispatcher::DFMEventDispatcher()
{

}

void DFMEventDispatcher::installEventHandler(DFMAbstractEventHandler *handler)
{
    if (!DFMEventDispatcherData::eventHandler.contains(handler))
        DFMEventDispatcherData::eventHandler.append(handler);
}

void DFMEventDispatcher::removeEventHandler(DFMAbstractEventHandler *handler)
{
    DFMEventDispatcherData::eventFilter.removeOne(handler);
}

DFM_END_NAMESPACE
