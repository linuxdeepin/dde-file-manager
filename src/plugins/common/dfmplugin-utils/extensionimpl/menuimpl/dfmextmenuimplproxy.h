// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMMENUIMPLPROXY_H
#define DFMMENUIMPLPROXY_H

#include "dfmplugin_utils_global.h"

#include <dfm-extension/menu/dfmextmenuproxy.h>
#include <QtCore>

DPUTILS_BEGIN_NAMESPACE

class DFMExtMenuImplProxy : public DFMEXT::DFMExtMenuProxy
{
public:
    explicit DFMExtMenuImplProxy();
};

DPUTILS_END_NAMESPACE

#endif   // DFMMENUIMPLPROXY_H
