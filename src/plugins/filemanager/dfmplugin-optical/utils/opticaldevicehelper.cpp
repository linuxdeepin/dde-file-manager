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
#include "opticaldevicehelper.h"
#include "utils/opticalhelper.h"

#include "dfm-base/dbusservice/global_server_defines.h"

DPOPTICAL_USE_NAMESPACE
using namespace GlobalServerDefines;

static constexpr char kBlockDeviceIdPrefix[] { "/org/freedesktop/UDisks2/block_devices/" };

QString OpticalDeviceHelper::deviceId(const QString &dev)
{
    return QString(kBlockDeviceIdPrefix) + dev;
}

QString OpticalDeviceHelper::firstMountPoint(const QString &dev)
{
    // TODO(zhangs): impl me!
    return {};
}
