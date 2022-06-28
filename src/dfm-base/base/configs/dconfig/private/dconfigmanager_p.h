/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#ifndef DCONFIGMANAGER_P_H
#define DCONFIGMANAGER_P_H

#include "dfm_base_global.h"

#include <dtkcore_global.h>
#include <QMap>
#include <QReadWriteLock>

DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE

DFMBASE_BEGIN_NAMESPACE

class DConfigManager;
class DConfigManagerPrivate
{
    friend DConfigManager;
    DConfigManager *q { nullptr };

    QMap<QString, DTK_NAMESPACE::DCORE_NAMESPACE::DConfig *> configs;
    QReadWriteLock lock;

public:
    explicit DConfigManagerPrivate(DConfigManager *qq)
        : q(qq) {}
};

DFMBASE_END_NAMESPACE

#endif   // DCONFIGMANAGER_P_H
