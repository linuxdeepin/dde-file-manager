// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmextmenuutil.h"

namespace dfmplugin_utils {

DFMExtMenuUtils *DFMExtMenuUtils::instance()
{
    static DFMExtMenuUtils ins;
    return &ins;
}

QList<QPair<QAction *, QAction *>> *DFMExtMenuUtils::extensionMenuSortRules()
{
    return &extMenuSortRules;
}

DFMExtMenuUtils::DFMExtMenuUtils()
{
}

}
