// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmextmenucache.h"

#include <QThread>
#include <QApplication>

namespace dfmplugin_utils {

DFMExtMenuCache &DFMExtMenuCache::instance()
{
    Q_ASSERT(qApp->thread() == QThread::currentThread());
    static DFMExtMenuCache ins;
    return ins;
}

DFMExtMenuCache::DFMExtMenuCache()
{
}

}
