// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTACTIONIMPL_H
#define DFMEXTACTIONIMPL_H

#include "dfmplugin_utils_global.h"

#include <dfm-extension/menu/dfmextaction.h>

class QAction;

DPUTILS_BEGIN_NAMESPACE
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
DPUTILS_END_NAMESPACE

#endif   // DFMEXTACTIONIMPL_H
