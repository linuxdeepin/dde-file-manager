/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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

#include "dfmextmenuproxy.h"
#include "private/dfmextmenuproxyprivate.h"

#include <assert.h>

USING_DFMEXT_NAMESPACE

DFMExtMenuProxy::DFMExtMenuProxy(DFMExtMenuProxyPrivate *d_ptr)
    : d(d_ptr)
{
    assert(d);
}

DFMExtMenuProxy::~DFMExtMenuProxy()
{
    delete d;
}

DFMExtMenu *DFMExtMenuProxy::createMenu()
{
    return d->createMenu();
}

bool DFMExtMenuProxy::deleteMenu(DFMExtMenu *menu)
{
    return d->deleteMenu(menu);
}

DFMExtAction *DFMExtMenuProxy::createAction()
{
    return d->createAction();
}

bool DFMExtMenuProxy::deleteAction(DFMExtAction *action)
{
    return d->deleteAction(action);
}
