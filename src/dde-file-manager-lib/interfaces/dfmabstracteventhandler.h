// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    explicit DFMAbstractEventHandler(bool autoInstallHandler = true);
    virtual ~DFMAbstractEventHandler();

    virtual bool fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData = 0);
    virtual bool fmEventFilter(const QSharedPointer<DFMEvent> &event, DFMAbstractEventHandler *target = 0, QVariant *resultData = 0);

    friend class DFMEventDispatcher;
};

DFM_END_NAMESPACE

#endif // DFMABSTRACTEVENTHANDLER_H
