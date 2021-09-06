/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#include "schemefactory.h"

DFMBASE_BEGIN_NAMESPACE
Q_GLOBAL_STATIC(DirIteratorFactory,_dirIteratorFactory)
Q_GLOBAL_STATIC(WacherFactory,_watcherFactory)
Q_GLOBAL_STATIC(InfoFactory,_infoFactory)
Q_GLOBAL_STATIC(FileDeviceFactory,_fileDeviceFactory)

InfoFactory &InfoFactory::instance()
{
    return *_infoFactory;
}

WacherFactory &WacherFactory::instance()
{
    return *_watcherFactory;
}

FileDeviceFactory &FileDeviceFactory::instance()
{
    return *_fileDeviceFactory;
}

DirIteratorFactory &DirIteratorFactory::instance()
{
    return *_dirIteratorFactory;
}
DFMBASE_END_NAMESPACE
