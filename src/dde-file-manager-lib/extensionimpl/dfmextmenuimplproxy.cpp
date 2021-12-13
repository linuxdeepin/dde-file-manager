/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *             huangyu<zhangyub@uniontech.com>
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
#include "dfmextmenuimplproxy.h"

#include "private/dfmextactionimpl_p.h"
#include "dfmextactionimpl.h"
#include "dfmextmenuimpl.h"
#include "dfmextmenuprivateimpl.h"
#include "private/dfmextmenuimplproxy_p.h"

#include <QAction>

USING_DFMEXT_NAMESPACE

DFMExtMenuImplProxy::DFMExtMenuImplProxy()
    : DFMExtMenuProxy(new DFMExtMenuImplProxyPrivate)
{

}

DFMExtMenu *DFMExtMenuImplProxyPrivate::createMenu(){
    auto menu = new DFMExtMenuImpl();
    return menu;
}

bool DFMExtMenuImplProxyPrivate::deleteMenu(DFMExtMenu *menu)
{
    if (menu) {
        DFMExtMenuImpl *impl = dynamic_cast<DFMExtMenuImpl *>(menu);
        if (impl == nullptr)
            return false;

        DFMExtMenuImplPrivate *impl_d = dynamic_cast<DFMExtMenuImplPrivate *>(impl->d);
        if (impl_d == nullptr)
            return false;

        // 非文管内部的才能释放
        if (!impl_d->isInterior())
            delete impl;
        else
            return false;
    }
    return true;
}

DFMExtAction *DFMExtMenuImplProxyPrivate::createAction()
{
    auto action = new DFMExtActionImpl();
    return action;
}

bool DFMExtMenuImplProxyPrivate::deleteAction(DFMExtAction *action)
{
    if (action) {

        DFMExtActionImpl *impl = dynamic_cast<DFMExtActionImpl *>(action);
        if (impl == nullptr)
            return false;

        DFMExtActionImplPrivate *impl_d = dynamic_cast<DFMExtActionImplPrivate *>(impl->d);
        if (impl_d == nullptr)
            return false;

        // 非文管内部创建的才能释放
        if (!impl_d->isInterior()) {
            delete impl;
        } else {
            return false;
        }
    }
    return true;
}
