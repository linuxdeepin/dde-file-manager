/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef DFMEVENTDISPATCHER_H
#define DFMEVENTDISPATCHER_H

#include "dfmglobal.h"

class DFMEvent;
DFM_BEGIN_NAMESPACE

class DFMAbstractEventHandler;
class DFMEventDispatcher
{
public:
    static DFMEventDispatcher *instance();

    QVariant processEvent(const QSharedPointer<DFMEvent> &event);

    void installEventFilter(DFMAbstractEventHandler *handler);
    void removeEventFilter(DFMAbstractEventHandler *handler);

protected:
    DFMEventDispatcher();

private:
    void installEventHandler(DFMAbstractEventHandler *handler);
    void removeEventHandler(DFMAbstractEventHandler *handler);

    friend class DFMAbstractEventHandler;
};

DFM_END_NAMESPACE

#endif // DFMEVENTDISPATCHER_H
