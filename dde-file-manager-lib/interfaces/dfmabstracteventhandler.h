/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef DFMABSTRACTEVENTHANDLER_H
#define DFMABSTRACTEVENTHANDLER_H

#include "dfmglobal.h"

class DFMEvent;
DFM_BEGIN_NAMESPACE

class DFMAbstractEventHandler
{
public:
    virtual QObject *object() const;

protected:
    DFMAbstractEventHandler();
    virtual ~DFMAbstractEventHandler();

    virtual bool fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData = 0);
    virtual bool fmEventFilter(const QSharedPointer<DFMEvent> &event, DFMAbstractEventHandler *target = 0, QVariant *resultData = 0);

    friend class DFMEventDispatcher;
};

DFM_END_NAMESPACE

#endif // DFMABSTRACTEVENTHANDLER_H
