// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMMENUIMPLPROXY_P_H
#define DFMMENUIMPLPROXY_P_H

#include <menu/private/dfmextmenuproxyprivate.h>

#include <QDebug>

class DFMExtMenuImplProxyPrivate : public DFMEXT::DFMExtMenuProxyPrivate
{
public:
    explicit DFMExtMenuImplProxyPrivate(){}
    DFMEXT::DFMExtMenu *createMenu() Q_DECL_OVERRIDE;
    bool deleteMenu(DFMEXT::DFMExtMenu *menu) Q_DECL_OVERRIDE;
    DFMEXT::DFMExtAction *createAction() Q_DECL_OVERRIDE;
    bool deleteAction(DFMEXT::DFMExtAction *action) Q_DECL_OVERRIDE;
};

#endif // DFMMENUIMPLPROXY_P_H
