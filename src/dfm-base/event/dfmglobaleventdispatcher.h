/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMGLOBALEVENTDISPATCHER_H
#define DFMGLOBALEVENTDISPATCHER_H

#include "dfm-base/event/protocol/dfmevent.h"
#include "dfm-base/event/handler/dfmabstracteventhandler.h"
#include "dfm-base/base/singleton.h"

#include <QtConcurrent>
#include <QThreadPool>
#include <QHash>
#include <QMetaEnum>
#include <QDebug>

/* @class DFMGlobalEventDispatcher
 * @brief 事件分发器，需要使用DFMEvent中type类型进行handler绑定后生效。
 */
class DFMGlobalEventDispatcher
{
    explicit DFMGlobalEventDispatcher(){}

    ~DFMGlobalEventDispatcher(){}

public:

    static void sendEvent(const DFMEventPointer &event);

    static void setMaxThreadCount(int count);

    static int maxThreadCount();

    static int installHandler(DFMAbstractEventHandler* const handler);
};

typedef DFMGlobalEventDispatcher DFMEventProxy;

#endif // DFMGLOBALEVENTDISPATCHER_H
