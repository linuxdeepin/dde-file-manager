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
#include "deviceutils.h"

#include "dfm-base/base/application/application.h"

DFMBASE_USE_NAMESPACE

QString DeviceUtils::getBlockDeviceId(const QString &deviceDesc)
{
    QString dev(deviceDesc);
    if (dev.startsWith("/dev/"))
        dev.remove("/dev/");
    return kBlockDeviceIdPrefix + dev;
}

QString DeviceUtils::errMessage(dfmmount::DeviceError err)
{
    return DFMMOUNT::Utils::errorMessage(err);
}

QString DeviceUtils::convertSizeToLabel(quint64 size)
{
    return "";   // TODO(xust)
}

bool DeviceUtils::isAutoMountEnable()
{
    return Application::genericAttribute(Application::GenericAttribute::kAutoMount).toBool();
}

bool DeviceUtils::isAutoMountAndOpenEnable()
{
    return Application::genericAttribute(Application::GenericAttribute::kAutoMountAndOpen).toBool();
}
