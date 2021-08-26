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
#ifndef DFMWINDOWEVENTHANDLER_H
#define DFMWINDOWEVENTHANDLER_H

#include "dfm-base/event/protocol/dfmwindowevent.h"

#include "dfm-base/event/handler/dfmabstracteventhandler.h"

class DFMWindowEventHandler :public DFMAbstractEventHandler
{
public:
    DFMWindowEventHandler();

    virtual ~DFMWindowEventHandler() override;

    void event(const DFMEventPointer &event) override;

    virtual void changeCurrentUrl(const DFMWindowEventPointer &event);

    virtual void openNewWindow(const DFMWindowEventPointer &event);

    virtual void openNewTab(const DFMWindowEventPointer &event);

    virtual void openUrl(const DFMWindowEventPointer &event);

    virtual void menuAction(const DFMWindowEventPointer &event);

    virtual void navBack(const DFMWindowEventPointer &event);

    virtual void navForward(const DFMWindowEventPointer &event);

    virtual void saveOperator(const DFMWindowEventPointer &event);

    virtual void cleanSaveOperator(const DFMWindowEventPointer &event);

    virtual void revocation(const DFMWindowEventPointer &event);
};

#endif // DFMWINDOWEVENTHANDLER_H
