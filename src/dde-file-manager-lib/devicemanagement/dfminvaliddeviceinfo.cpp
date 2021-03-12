/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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


#include "dfminvaliddeviceinfo.h"

#include <QIcon>

DFM_BEGIN_NAMESPACE

DFMInvalidDeviceInfo::DFMInvalidDeviceInfo()
{

}

void DFMInvalidDeviceInfo::mount()
{

}

bool DFMInvalidDeviceInfo::unmountable()
{
    return false;
}

void DFMInvalidDeviceInfo::unmount()
{

}

bool DFMInvalidDeviceInfo::ejectable()
{
    return false;
}

void DFMInvalidDeviceInfo::eject()
{

}

bool DFMInvalidDeviceInfo::isReadOnly() const
{
    return true;
}

QString DFMInvalidDeviceInfo::name() const
{
    return QStringLiteral("Invalid Device");
}

bool DFMInvalidDeviceInfo::canRename() const
{
    return false;
}

QString DFMInvalidDeviceInfo::displayName() const
{
    return QStringLiteral("Invalid Device");
}

QString DFMInvalidDeviceInfo::iconName() const
{
    return "dialog-error";
}

bool DFMInvalidDeviceInfo::deviceUsageValid() const
{
    return false;
}

quint64 DFMInvalidDeviceInfo::availableBytes() const
{
    return -1;
}

quint64 DFMInvalidDeviceInfo::freeBytes() const
{
    return -1;
}

quint64 DFMInvalidDeviceInfo::totalBytes() const
{
    return 0;
}

QString DFMInvalidDeviceInfo::mountpointPath() const
{
    return QString();
}

DFM_END_NAMESPACE
