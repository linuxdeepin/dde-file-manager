// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmextmenuimplproxy.h"

#include "private/dfmextactionimpl_p.h"
#include "private/dfmextmenuimpl_p.h"
#include "private/dfmextmenuimplproxy_p.h"
#include "dfmextactionimpl.h"
#include "dfmextmenuimpl.h"

#include <QAction>

USING_DFMEXT_NAMESPACE
DPUTILS_USE_NAMESPACE

DFMExtMenuImplProxy::DFMExtMenuImplProxy()
    : DFMExtMenuProxy(new DFMExtMenuImplProxyPrivate)
{
}

DFMExtMenu *DFMExtMenuImplProxyPrivate::createMenu()
{
    auto menu = new DFMExtMenuImpl();
    return menu;
}

bool DFMExtMenuImplProxyPrivate::deleteMenu(DFMExtMenu *menu)
{
    if (menu) {
        DFMExtMenuImpl *impl = static_cast<DFMExtMenuImpl *>(menu);
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

        DFMExtActionImpl *impl = static_cast<DFMExtActionImpl *>(action);
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
