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
#ifndef GSETTINGMANAGER_P_H
#define GSETTINGMANAGER_P_H

#include "dfm_base_global.h"

#include <QMap>
#include <QReadWriteLock>

class QGSettings;

DFMBASE_BEGIN_NAMESPACE

class GSettingManager;
class GSettingManagerPrivate
{
    friend class GSettingManager;
    GSettingManager *q { nullptr };
    QMap<QString, QGSettings *> settings;
    QReadWriteLock lock;

public:
    GSettingManagerPrivate(GSettingManager *qq)
        : q(qq) {}
};

DFMBASE_END_NAMESPACE

#endif   // GSETTINGMANAGER_P_H
