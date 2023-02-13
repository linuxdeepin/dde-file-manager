// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTMENUPROXYPRIVATE_H
#define DFMEXTMENUPROXYPRIVATE_H

#include <dfm-extension/dfm-extension-global.h>
#include <dfm-extension/menu/dfmextmenuproxy.h>

BEGEN_DFMEXT_NAMESPACE

class DFMExtMenuProxyPrivate
{
public:
    explicit DFMExtMenuProxyPrivate();
    virtual ~DFMExtMenuProxyPrivate();

    virtual DFMExtMenu *createMenu() = 0;
    virtual bool deleteMenu(DFMExtMenu *) = 0;

    virtual DFMExtAction *createAction() = 0;
    virtual bool deleteAction(DFMExtAction *) = 0;
};

END_DFMEXT_NAMESPACE

#endif   // DFMEXTMENUPROXYPRIVATE_H
