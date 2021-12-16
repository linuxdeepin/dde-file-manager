/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "recent.h"
#include "recentutil.h"

#include "services/filemanager/windows/windowsservice.h"
#include "services/common/menu/menuservice.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/interfaces/abstractfilewatcher.h"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfileinfo.h"
#include "dfm-base/file/local/localdiriterator.h"
#include "services/common/menu/menuservice.h"

#include <dfm-framework/framework.h>

void Recent::initialize()
{
    QString recentScheme { "recent" };
}

bool Recent::start()
{
    return true;
}

dpf::Plugin::ShutdownFlag Recent::stop()
{
    return kSync;
}
