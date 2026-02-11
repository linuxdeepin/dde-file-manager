// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTMENUIMPL_H
#define DFMEXTMENUIMPL_H

#include "dfmplugin_utils_global.h"

#include <dfm-extension/menu/dfmextmenu.h>

class QMenu;

DPUTILS_BEGIN_NAMESPACE

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

DPUTILS_END_NAMESPACE

#endif   // DFMEXTMENUIMPL_H
