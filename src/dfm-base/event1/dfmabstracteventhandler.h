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

#include "base/dfmglobal.h"

class DFMEvent;
class DFMAbstractEventHandler
{
public:
    virtual QObject *object() const;

protected:
    explicit DFMAbstractEventHandler(bool autoInstallHandler = true);
    virtual ~DFMAbstractEventHandler();

    virtual bool fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData = nullptr);
    virtual bool fmEventFilter(const QSharedPointer<DFMEvent> &event,
                               DFMAbstractEventHandler *target = nullptr,
                               QVariant *resultData = nullptr);

    virtual bool fileEvent(DFMEvent *event);
    virtual bool fileLauchAppEvent(DFMEvent *);
    virtual bool ChangedEvent(DFMEvent *);
    virtual bool windowEvent(DFMEvent *);

    friend class DFMEventDispatcher;
};

#endif // DFMABSTRACTEVENTHANDLER_H
