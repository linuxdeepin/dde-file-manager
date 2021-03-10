/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
