/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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
#ifndef SYSINFOUTILS_H
#define SYSINFOUTILS_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>
#include <QMimeData>

DFMBASE_BEGIN_NAMESPACE

class SysInfoUtils
{

public:
    static QString getUser();
    static int getUserId();

    static bool isRootUser();
    static bool isServerSys();
    static bool isDesktopSys();
    static bool isOpenAsAdmin();
    static bool isDeveloperModeEnabled();
    static bool isProfessional();
    static bool isSameUser(const QMimeData *data);
};
DFMBASE_END_NAMESPACE

#endif   // SYSINFOUTILS_H
