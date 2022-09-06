// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
