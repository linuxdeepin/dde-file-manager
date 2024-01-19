// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "private/dfmextmenuproxyprivate.h"

#include <dfm-extension/menu/dfmextmenuproxy.h>

#include <cassert>

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
