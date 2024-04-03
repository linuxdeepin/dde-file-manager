// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTMENUUTILS_H
#define DFMEXTMENUUTILS_H

#include "dfmplugin_utils_global.h"

#include <QAction>

namespace dfmplugin_utils {

class DFMExtMenuUtils : public QObject
{
    Q_OBJECT
public:
    static DFMExtMenuUtils *instance();
    QList<QPair<QAction*, QAction*>> *extensionMenuSortRules();

private:
    DFMExtMenuUtils();
    QList<QPair<QAction *, QAction *>> extMenuSortRules {};
};

}

#endif   // DFMEXTMENUUTILS_H
