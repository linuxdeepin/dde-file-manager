// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
