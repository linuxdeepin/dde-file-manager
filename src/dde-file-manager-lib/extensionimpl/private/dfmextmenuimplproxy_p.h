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
