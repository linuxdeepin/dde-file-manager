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
#ifndef DFMEXTMENUIMPL_H
#define DFMEXTMENUIMPL_H

#include <menu/dfmextmenu.h>

class QMenu;
class DFMExtMenuImplPrivate;
class DFMExtMenuImpl : public DFMEXT::DFMExtMenu
{
    friend class DFMExtMenuImplProxy;
    friend class DFMExtActionImpl;
    friend class DFMExtMenuImplProxyPrivate;
    friend class DFMExtActionImplPrivate;
public:
    explicit DFMExtMenuImpl(QMenu *menu = nullptr);
};

#endif // DFMEXTMENUIMPL_H
