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
#ifndef DFMEXTACTIONIMPL_H
#define DFMEXTACTIONIMPL_H

#include <menu/dfmextaction.h>

class QAction;
class DFMExtActionImplPrivate;
class DFMExtActionImpl : public DFMEXT::DFMExtAction
{
    friend class DFMExtMenuImplProxy;
    friend class DFMExtMenuImpl;
    friend class DFMExtMenuImplProxyPrivate;
    friend class DFMExtMenuImplPrivate;
public:
    explicit DFMExtActionImpl(QAction *action = nullptr);
};

#endif // DFMEXTACTIONIMPL_H
