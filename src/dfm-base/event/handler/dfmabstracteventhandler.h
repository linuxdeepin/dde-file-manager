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

#include "dfm-base/event/protocol/dfmevent.h"
#include "dfm-base/base/dfmglobal.h"

#include <QSharedPointer>

class DFMEvent;

class DFMAbstractEventHandler
{

public:
    explicit DFMAbstractEventHandler();

    virtual ~DFMAbstractEventHandler();

    virtual bool canAsynProcess();

    virtual QString scheme();

    virtual void event(const DFMEventPointer &event);
};


#endif // DFMABSTRACTEVENTHANDLER_H
