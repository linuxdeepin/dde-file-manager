/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "masteredmediadiriterator.h"

#include "utils/opticalhelper.h"
#include "utils/opticaldevicehelper.h"

DFMBASE_USE_NAMESPACE
DPOPTICAL_USE_NAMESPACE

MasteredMediaDirIterator::MasteredMediaDirIterator(const QUrl &url,
                                                   const QStringList &nameFilters,
                                                   QDir::Filters filters,
                                                   QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(url, nameFilters, filters, flags)
{
    QString device { OpticalHelper::burnDestDevice(url) };
    mntPoint = OpticalDeviceHelper::firstMountPoint(device.remove("/dev/"));
    while (*mntPoint.rbegin() == '/')
        mntPoint.chop(1);
}

QUrl MasteredMediaDirIterator::next()
{
    // TODO(zhangs): impl me!
    return {};
}

bool MasteredMediaDirIterator::hasNext() const
{
    // TODO(zhangs): impl me!
    return {};
}

QString MasteredMediaDirIterator::fileName() const
{
    // TODO(zhangs): impl me!
    return {};
}

QUrl MasteredMediaDirIterator::fileUrl() const
{
    // TODO(zhangs): impl me!
    return {};
}

const AbstractFileInfoPointer MasteredMediaDirIterator::fileInfo() const
{
    // TODO(zhangs): impl me!
    return {};
}

QUrl MasteredMediaDirIterator::url() const
{
    // TODO(zhangs): impl me!
    return {};
}

QUrl MasteredMediaDirIterator::changeScheme(const QUrl &in) const
{
    // TODO(zhangs): impl me!
    return {};
}

QUrl MasteredMediaDirIterator::changeSchemeUpdate(const QUrl &in)
{
    // TODO(zhangs): impl me!
    return {};
}
