// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMEXTMENUCACHE_H
#define DFMEXTMENUCACHE_H

#include "dfmplugin_utils_global.h"

#include <QAction>

namespace dfmplugin_utils {

class DFMExtMenuCache : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DFMExtMenuCache)

public:
    static DFMExtMenuCache &instance();
    QList<QPair<QAction *, QAction *>> extMenuSortRules {};

private:
    DFMExtMenuCache();
};

}

#endif   // DFMEXTMENUCACHE_H
