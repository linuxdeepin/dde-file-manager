// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFMEXTMENUPROXY_H
#define DFMEXTMENUPROXY_H

#include <dfm-extension/dfm-extension-global.h>

#include <string>
#include <functional>

BEGEN_DFMEXT_NAMESPACE

class DFMExtMenu;
class DFMExtAction;
class DFMExtMenuProxyPrivate;
class DFMExtMenuProxy
{
public:
    explicit DFMExtMenuProxy(DFMExtMenuProxyPrivate *d_ptr);
    ~DFMExtMenuProxy();

    DFMExtMenu *createMenu();
    bool deleteMenu(DFMExtMenu *menu);

    DFMExtAction *createAction();
    bool deleteAction(DFMExtAction *action);

private:
    DFMExtMenuProxyPrivate *d;
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTMENUPROXY_H
