/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef DFMAPPLICATION_P_H
#define DFMAPPLICATION_P_H

#include <dfmglobal.h>

DFM_BEGIN_NAMESPACE

class DFMApplication;
class DFMApplicationPrivate
{
public:
    explicit DFMApplicationPrivate(dde_file_manager::DFMApplication *qq);

    void _q_onSettingsValueChanged(const QString &group, const QString &key, const QVariant &value, bool edited = false);
    void _q_onSettingsValueEdited(const QString &group, const QString &key, const QVariant &value);

    static DFMApplication *self;
};

DFM_END_NAMESPACE

#endif // DFMAPPLICATION_P_H
