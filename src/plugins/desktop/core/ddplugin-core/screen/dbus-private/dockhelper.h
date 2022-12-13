/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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

#ifndef DOCKHELPER_H
#define DOCKHELPER_H

#include "ddplugin_core_global.h"

#ifdef COMPILE_ON_V23
#include "dbusdock1.h"
#else
#include "dbusdock.h"
#endif

#include <QObject>

DDPCORE_BEGIN_NAMESPACE

class DockHelper : public QObject
{
    Q_OBJECT
public:
    static DockHelper *ins();
    DBusDock *dock() const;
private:
    explicit DockHelper(QObject *parent = nullptr);
    DBusDock *m_dock = nullptr;
};

DDPCORE_END_NAMESPACE

#define DockInfoIns ddplugin_core::DockHelper::ins()->dock()

#endif // DOCKHELPER_H
